/*-------------------------------------------------------------------------
 *
 * nodeNestloop.c
 *	  routines to support nest-loop joins
 *
 * Portions Copyright (c) 1996-2011, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/backend/executor/nodeNestloop.c
 *
 *-------------------------------------------------------------------------
 */
 /*
  *	 INTERFACE ROUTINES
  *		ExecNestLoop	 - process a nestloop join of two plans
  *		ExecInitNestLoop - initialize the join
  *		ExecEndNestLoop  - shut down the join
  */

#include "postgres.h"

#include "executor/execdebug.h"
#include "executor/nodeNestloop.h"
#include "utils/memutils.h"

#define BLOCKSIZE 1024


  /* ----------------------------------------------------------------
   *		ExecNestLoop(node)
   *
   * old comments
   *		Returns the tuple joined from inner and outer tuples which
   *		satisfies the qualification clause.
   *
   *		It scans the inner relation to join with current outer tuple.
   *
   *		If none is found, next tuple from the outer relation is retrieved
   *		and the inner relation is scanned from the beginning again to join
   *		with the outer tuple.
   *
   *		NULL is returned if all the remaining outer tuples are tried and
   *		all fail to join with the inner tuples.
   *
   *		NULL is also returned if there is no tuple from inner relation.
   *
   *		Conditions:
   *		  -- outerTuple contains current tuple from outer relation and
   *			 the right son(inner relation) maintains "cursor" at the tuple
   *			 returned previously.
   *				This is achieved by maintaining a scan position on the outer
   *				relation.
   *
   *		Initial States:
   *		  -- the outer child and the inner child
   *			   are prepared to return the first tuple.
   * ----------------------------------------------------------------
   */
TupleTableSlot*
ExecNestLoop(NestLoopState* node)
{
	NestLoop* nl;
	PlanState* innerPlan;
	PlanState* outerPlan;
	TupleTableSlot* outerTupleSlot;
	TupleTableSlot* innerTupleSlot;
	List* joinqual;
	List* otherqual;
	ExprContext* econtext;
	ListCell* lc;

	/*
	 * get information from the node
	 */
	ENL1_printf("getting info from node");

	nl = (NestLoop*)node->js.ps.plan;
	joinqual = node->js.joinqual;
	otherqual = node->js.ps.qual;
	outerPlan = outerPlanState(node);
	innerPlan = innerPlanState(node);
	econtext = node->js.ps.ps_ExprContext;

	/*
	 * Check to see if we're still projecting out tuples from a previous join
	 * tuple (because there is a function-returning-set in the projection
	 * expressions).  If so, try to project another one.
	 * 这一步是在检查是否完成该外层元组的连接
	 */
	if (node->js.ps.ps_TupFromTlist)
	{
		TupleTableSlot* result;
		ExprDoneCond isDone;

		result = ExecProject(node->js.ps.ps_ProjInfo, &isDone);
		if (isDone == ExprMultipleResult)
			return result;
		/* Done with that source tuple... */
		node->js.ps.ps_TupFromTlist = false;
	}

	/*
	 * Reset per-tuple memory context to free any expression evaluation
	 * storage allocated in the previous tuple cycle.  Note this can't happen
	 * until we're done projecting out tuples from a join tuple.
	 */
	ResetExprContext(econtext);

	/*
	 * Ok, everything is setup for the join so now loop until we return a
	 * qualifying join tuple.
	 */
	ENL1_printf("entering main loop");

	for (;;)
	{
		/*
		 * If we don't have an outer tuple, get the next one and reset the
		 * inner scan.
		 * 如果当前没有外层元组，取下一外层元组并重新开始对内层扫描
		 */
		if (node->nl_NeedNewBlock)				//需要新块
		{
			int i;			//循环变量

			/*ENL1_printf("getting new outer tuple");
			outerTupleSlot = ExecProcNode(outerPlan);*/

			if (node->nl_OuterSize < BLOCKSIZE)
			{
				return NULL;
			}

			for (i = 0; i < BLOCKSIZE; ++i)			//每次取一个
			{
				outerTupleSlot = ExecProcNode(outerPlan);			//取外层元组

				if (TupIsNull(outerTupleSlot))		//当前没有可取的了
				{
					ENL1_printf("no outer tuple, ending join");
					if (i == 0)			//说明一个都没取到，真正结束连接
					{
						return NULL;
					}
					//否则，该块还是取到了一些外层元组的，跳出正常处理
					break;
				}

				//将取到的外层元组放到NestLoopState的块成员中
				ExecCopySlot(node->nl_Block[i], outerTupleSlot);
			}
			//该块有多少个元组
			node->nl_OuterSize = i;
			node->nl_OuterPos = 0;

			ENL1_printf("saving new outer tuple information");
			/*econtext->ecxt_outertuple = outerTupleSlot;
			node->nl_NeedNewOuter = false;
			node->nl_MatchedOuter = false;*/
			node->nl_NeedNewBlock = false;
			node->nl_NeedNewInner = true;
			memset(node->nl_MatchedBlock, false, sizeof(node->nl_MatchedBlock));

			/*
			 * fetch the values of any outer Vars that must be passed to the
			 * inner scan, and store them in the appropriate PARAM_EXEC slots.
			 * 下面的进程用于为重新扫描内层元组做准备
			 * 对于每个Block只需进行一次。
			 */
			foreach(lc, nl->nestParams)
			{
				NestLoopParam* nlp = (NestLoopParam*)lfirst(lc);
				int			paramno = nlp->paramno;
				ParamExecData* prm;

				prm = &(econtext->ecxt_param_exec_vals[paramno]);
				/* Param value should be an OUTER var */
				Assert(IsA(nlp->paramval, Var));
				Assert(nlp->paramval->varno == OUTER);
				Assert(nlp->paramval->varattno > 0);
				prm->value = slot_getattr(outerTupleSlot,
					nlp->paramval->varattno,
					&(prm->isnull));
				/* Flag parameter value as changed */
				innerPlan->chgParam = bms_add_member(innerPlan->chgParam,
					paramno);
			}

			/*
			 * now rescan the inner plan
			 */
			ENL1_printf("rescanning inner plan");
			ExecReScan(innerPlan);
		}


		if (node->nl_NeedNewInner)		//取新的内层元组，则需要考察这个块的所有外层元组
		{
			/*
			 * we have an outerTuple, try to get the next inner tuple.
			 * 接下来的嵌套循环是inner在外层，outer在内层，所以我们要按原版处理outer的方式处理现在的inner
			 */
			ENL1_printf("getting new inner tuple");

			innerTupleSlot = ExecProcNode(innerPlan);			//取内层元组
			econtext->ecxt_innertuple = innerTupleSlot;
			node->nl_NeedNewInner = false;

			if (TupIsNull(innerTupleSlot))				//若没有内层元组
			{
				ENL1_printf("no inner tuple, need new outer tuple");

				if (node->js.jointype == JOIN_LEFT || node->js.jointype == JOIN_ANTI)		//左外连接或anti join在这里就进行连接
				{
					int i;
					//现在考察块中的每个外层元组
					for (i = 0; i < node->nl_OuterSize; ++i)
					{
						if (!node->nl_MatchedBlock[i])				//该位置的外层元组与所有内层元组都不匹配
						{
							econtext->ecxt_outertuple = node->nl_Block[i];		//配置Context

						/*
						 * We are doing an outer join and there were no join matches
						 * for this outer tuple.  Generate a fake join tuple with
						 * nulls for the inner tuple, and return it if it passes the
						 * non-join quals.
						 * 当前外层元组没有与之匹配的内层元组。现在我们生成一个空的
						 * "假"元组充当内层元组与之连接（如果其满足"不连接"条件）。
						 */
							econtext->ecxt_innertuple = node->nl_NullInnerTupleSlot;

							ENL1_printf("testing qualification for outer-join tuple");

							if (otherqual == NIL || ExecQual(otherqual, econtext, false))		//如果没有其他条件，或者满足其他条件
							{
								/*
								 * qualification was satisfied so we project and return
								 * the slot containing the result tuple using
								 * ExecProject().
								 * 因为此时需要这些不满足连接条件的元组，将其返回（但还是要满足其他条件的)
								 */

								TupleTableSlot* result;
								ExprDoneCond isDone;

								ENL1_printf("qualification succeeded, projecting tuple");

								result = ExecProject(node->js.ps.ps_ProjInfo, &isDone);

								if (isDone != ExprEndResult)
								{
									node->js.ps.ps_TupFromTlist =
										(isDone == ExprMultipleResult);
									return result;
								}
							}
						}
					}
				}
				//否则，若没有从上面返回，则说明所有外层块都不满足条件，从而需要取新块(内层已结束)
				node->nl_NeedNewBlock = true;
				continue;
			}
			node->nl_OuterPos = 0;			//取新内层元组，外层从0开始循环
		}

		//不需要新内层，说明正在考察对应的外层元组
		if (node->nl_OuterPos >= node->nl_OuterSize)		//超过了块大小，说明考察完了这个块，所以取下一个内层元组(继续考察这个块)
		{
			node->nl_NeedNewInner = true;
			continue;
		}
		outerTupleSlot = node->nl_Block[node->nl_OuterPos++];			//取下一外层元组
		Assert(!TupIsNull(outerTupleSlot));							//要求外层元组非空
		econtext->ecxt_outertuple = outerTupleSlot;					//配置Context

		/*
		 * at this point we have a new pair of inner and outer tuples so we
		 * test the inner and outer tuples to see if they satisfy the node's
		 * qualification.
		 * 若有内层元组，则检查是否满足连接条件
		 *
		 * Only the joinquals determine MatchedOuter status, but all quals
		 * must pass to actually return the tuple.
		 * 仅有连接条件决定MatchOuter状态（标识是否匹配）
		 * 但返回的元组必须满足所有条件
		 */
		ENL1_printf("testing qualification");

		if (ExecQual(joinqual, econtext, false))		//若满足连接条件
		{
			/* In an antijoin, we never return a matched tuple
			 * 类型为anti join，那么我们就不需要这个能匹配的元组
			 * 由于anti join具体是像not in或不等于这类的实现，那么
			 * 这个外层元组就不用考察了，直接去找下一个外层元组，而不是按原来的想法找新的内层元组
			 * 这样得到的Anti Join的结果和顺序和原来都是一致的，因为是在前面统一返回结果的
			 */
			if (node->js.jointype == JOIN_ANTI)
			{
				node->nl_MatchedBlock[node->nl_OuterPos - 1] = true;
				continue;		/* return to top of loop */
			}

			/*
			 * In a semijoin, we'll consider returning the first match, but
			 * after that we're done with this outer tuple.
			 * 对于semijoin(注：半连接指匹配满足条件的第一对）
			 * 返回第一对匹配的元组，并考察新的外层元组（注意第一总是指对于外层元组找到的第一个内层元组）
			 * 注意现在是内层在外，外层在内，所以如果对于一个内层找到了匹配的外层，
			 * 就执行continue重新回到循环开头，去找下一个外层元组（看两者还能不能匹配），而不是按原来的想法去找新的内层元组
			 * 另外注意一点这里不能标记匹配，因为当前的内层元组不一定是与该外层元组匹配的第一个，所以只能continue
			 * 是通过检查外层元组是否有过匹配了来决定是否continue
			 * 这样得到的Semi Join的结果和原来一致，但顺序会产生改变
			 */
			if (node->js.jointype == JOIN_SEMI && node->nl_MatchedBlock[node->nl_OuterPos - 1])		//如果该外层元组之前有过匹配了
				continue;

			node->nl_MatchedBlock[node->nl_OuterPos - 1] = true;		//标识当前元组是匹配的

			if (otherqual == NIL || ExecQual(otherqual, econtext, false))		//满足其他条件
			{
				/*
				 * qualification was satisfied so we project and return the
				 * slot containing the result tuple using ExecProject().
				 */
				TupleTableSlot* result;
				ExprDoneCond isDone;

				ENL1_printf("qualification succeeded, projecting tuple");

				result = ExecProject(node->js.ps.ps_ProjInfo, &isDone);

				if (isDone != ExprEndResult)
				{
					node->js.ps.ps_TupFromTlist =
						(isDone == ExprMultipleResult);
					return result;
				}
			}
		}

		/*
		 * Tuple fails qual, so free per-tuple memory and try again.
		 * 不满足连接条件，清理内存，考察下一个内层元组
		 */
		ResetExprContext(econtext);

		ENL1_printf("qualification failed, looping");
	}
}

/* ----------------------------------------------------------------
 *		ExecInitNestLoop
 * ----------------------------------------------------------------
 * 对循环嵌套进行初始化
 */
NestLoopState*
ExecInitNestLoop(NestLoop* node, EState* estate, int eflags)
{
	NestLoopState* nlstate;

	/* check for unsupported flags */
	Assert(!(eflags & (EXEC_FLAG_BACKWARD | EXEC_FLAG_MARK)));

	NL1_printf("ExecInitNestLoop: %s\n",
		"initializing node");

	/*
	 * create state structure
	 */
	nlstate = makeNode(NestLoopState);
	nlstate->js.ps.plan = (Plan*)node;
	nlstate->js.ps.state = estate;

	/*
	 * Miscellaneous initialization
	 *
	 * create expression context for node
	 */
	ExecAssignExprContext(estate, &nlstate->js.ps);

	/*
	 * initialize child expressions
	 */
	nlstate->js.ps.targetlist = (List*)
		ExecInitExpr((Expr*)node->join.plan.targetlist,
			(PlanState*)nlstate);
	nlstate->js.ps.qual = (List*)
		ExecInitExpr((Expr*)node->join.plan.qual,
			(PlanState*)nlstate);
	nlstate->js.jointype = node->join.jointype;
	nlstate->js.joinqual = (List*)
		ExecInitExpr((Expr*)node->join.joinqual,
			(PlanState*)nlstate);

	/*
	 * initialize child nodes
	 *
	 * If we have no parameters to pass into the inner rel from the outer,
	 * tell the inner child that cheap rescans would be good.  If we do have
	 * such parameters, then there is no point in REWIND support at all in the
	 * inner child, because it will always be rescanned with fresh parameter
	 * values.
	 */
	outerPlanState(nlstate) = ExecInitNode(outerPlan(node), estate, eflags);
	if (node->nestParams == NIL)
		eflags |= EXEC_FLAG_REWIND;
	else
		eflags &= ~EXEC_FLAG_REWIND;
	innerPlanState(nlstate) = ExecInitNode(innerPlan(node), estate, eflags);

	/*
	 * tuple table initialization
	 */
	ExecInitResultTupleSlot(estate, &nlstate->js.ps);

	switch (node->join.jointype)
	{
	case JOIN_INNER:
	case JOIN_SEMI:
		break;
	case JOIN_LEFT:
	case JOIN_ANTI:
		nlstate->nl_NullInnerTupleSlot =
			ExecInitNullTupleSlot(estate,
				ExecGetResultType(innerPlanState(nlstate)));
		break;
	default:
		elog(ERROR, "unrecognized join type: %d",
			(int)node->join.jointype);
	}

	/*
	 * initialize tuple type and projection info
	 */
	ExecAssignResultTypeFromTL(&nlstate->js.ps);
	ExecAssignProjectionInfo(&nlstate->js.ps, NULL);

	/*
	 * finally, wipe the current outer tuple clean.
	 */

	nlstate->js.ps.ps_TupFromTlist = false;
	/*nlstate->nl_NeedNewOuter = true;
	nlstate->nl_MatchedOuter = false;*/
	int i;

	nlstate->nl_NeedNewBlock = true;
	nlstate->nl_OuterSize = BLOCKSIZE;
	nlstate->nl_OuterPos = 0;
	nlstate->nl_Block = (TupleTableSlot**)malloc(sizeof(bool) * (BLOCKSIZE));
	for (i = 0; i < BLOCKSIZE; ++i)
		nlstate->nl_Block[i] = ExecInitNullTupleSlot(estate, ExecGetResultType(outerPlanState(nlstate)));
	nlstate->nl_MatchedBlock = (bool*)malloc(sizeof(bool) * (BLOCKSIZE));
	memset(nlstate->nl_MatchedBlock, false, sizeof(nlstate->nl_MatchedBlock));

	NL1_printf("ExecInitNestLoop: %s\n",
		"node initialized");

	return nlstate;
}

/* ----------------------------------------------------------------
 *		ExecEndNestLoop
 *
 *		closes down scans and frees allocated storage
 * ----------------------------------------------------------------
 */
void
ExecEndNestLoop(NestLoopState* node)
{
	NL1_printf("ExecEndNestLoop: %s\n",
		"ending node processing");

	/*
	 * Free the exprcontext
	 */
	ExecFreeExprContext(&node->js.ps);

	free(node->nl_Block);
	free(node->nl_MatchedBlock);

	/*
	 * clean out the tuple table
	 */
	ExecClearTuple(node->js.ps.ps_ResultTupleSlot);

	/*
	 * close down subplans
	 */
	ExecEndNode(outerPlanState(node));
	ExecEndNode(innerPlanState(node));

	NL1_printf("ExecEndNestLoop: %s\n",
		"node processing ended");
}

/* ----------------------------------------------------------------
 *		ExecReScanNestLoop
 * ----------------------------------------------------------------
 * 重新扫描
 * 与之前不同：之前是取了新外层元组重新扫描内层；
 * 现在是，对于每个块重新扫描内层(再对每个内层元组重新扫描块中的元组)
 */
void
ExecReScanNestLoop(NestLoopState* node)
{
	PlanState* outerPlan = outerPlanState(node);

	/*
	 * If outerPlan->chgParam is not null then plan will be automatically
	 * re-scanned by first ExecProcNode.
	 */
	if (outerPlan->chgParam == NULL)
		ExecReScan(outerPlan);

	/*
	 * innerPlan is re-scanned for each new outer tuple and MUST NOT be
	 * re-scanned from here or you'll get troubles from inner index scans when
	 * outer Vars are used as run-time keys...
	 */

	node->js.ps.ps_TupFromTlist = false;
	/*node->nl_NeedNewOuter = true;
	node->nl_MatchedOuter = false;*/

	node->nl_NeedNewBlock = true;
	node->nl_NeedNewInner = false;
}
