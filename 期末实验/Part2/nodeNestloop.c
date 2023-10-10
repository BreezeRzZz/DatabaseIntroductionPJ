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
	 * ��һ�����ڼ���Ƿ���ɸ����Ԫ�������
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
		 * �����ǰû�����Ԫ�飬ȡ��һ���Ԫ�鲢���¿�ʼ���ڲ�ɨ��
		 */
		if (node->nl_NeedNewBlock)				//��Ҫ�¿�
		{
			int i;			//ѭ������

			/*ENL1_printf("getting new outer tuple");
			outerTupleSlot = ExecProcNode(outerPlan);*/

			if (node->nl_OuterSize < BLOCKSIZE)
			{
				return NULL;
			}

			for (i = 0; i < BLOCKSIZE; ++i)			//ÿ��ȡһ��
			{
				outerTupleSlot = ExecProcNode(outerPlan);			//ȡ���Ԫ��

				if (TupIsNull(outerTupleSlot))		//��ǰû�п�ȡ����
				{
					ENL1_printf("no outer tuple, ending join");
					if (i == 0)			//˵��һ����ûȡ����������������
					{
						return NULL;
					}
					//���򣬸ÿ黹��ȡ����һЩ���Ԫ��ģ�������������
					break;
				}

				//��ȡ�������Ԫ��ŵ�NestLoopState�Ŀ��Ա��
				ExecCopySlot(node->nl_Block[i], outerTupleSlot);
			}
			//�ÿ��ж��ٸ�Ԫ��
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
			 * ����Ľ�������Ϊ����ɨ���ڲ�Ԫ����׼��
			 * ����ÿ��Blockֻ�����һ�Ρ�
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


		if (node->nl_NeedNewInner)		//ȡ�µ��ڲ�Ԫ�飬����Ҫ�����������������Ԫ��
		{
			/*
			 * we have an outerTuple, try to get the next inner tuple.
			 * ��������Ƕ��ѭ����inner����㣬outer���ڲ㣬��������Ҫ��ԭ�洦��outer�ķ�ʽ�������ڵ�inner
			 */
			ENL1_printf("getting new inner tuple");

			innerTupleSlot = ExecProcNode(innerPlan);			//ȡ�ڲ�Ԫ��
			econtext->ecxt_innertuple = innerTupleSlot;
			node->nl_NeedNewInner = false;

			if (TupIsNull(innerTupleSlot))				//��û���ڲ�Ԫ��
			{
				ENL1_printf("no inner tuple, need new outer tuple");

				if (node->js.jointype == JOIN_LEFT || node->js.jointype == JOIN_ANTI)		//�������ӻ�anti join������ͽ�������
				{
					int i;
					//���ڿ�����е�ÿ�����Ԫ��
					for (i = 0; i < node->nl_OuterSize; ++i)
					{
						if (!node->nl_MatchedBlock[i])				//��λ�õ����Ԫ���������ڲ�Ԫ�鶼��ƥ��
						{
							econtext->ecxt_outertuple = node->nl_Block[i];		//����Context

						/*
						 * We are doing an outer join and there were no join matches
						 * for this outer tuple.  Generate a fake join tuple with
						 * nulls for the inner tuple, and return it if it passes the
						 * non-join quals.
						 * ��ǰ���Ԫ��û����֮ƥ����ڲ�Ԫ�顣������������һ���յ�
						 * "��"Ԫ��䵱�ڲ�Ԫ����֮���ӣ����������"������"��������
						 */
							econtext->ecxt_innertuple = node->nl_NullInnerTupleSlot;

							ENL1_printf("testing qualification for outer-join tuple");

							if (otherqual == NIL || ExecQual(otherqual, econtext, false))		//���û����������������������������
							{
								/*
								 * qualification was satisfied so we project and return
								 * the slot containing the result tuple using
								 * ExecProject().
								 * ��Ϊ��ʱ��Ҫ��Щ����������������Ԫ�飬���䷵�أ�������Ҫ��������������)
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
				//������û�д����淵�أ���˵���������鶼�������������Ӷ���Ҫȡ�¿�(�ڲ��ѽ���)
				node->nl_NeedNewBlock = true;
				continue;
			}
			node->nl_OuterPos = 0;			//ȡ���ڲ�Ԫ�飬����0��ʼѭ��
		}

		//����Ҫ���ڲ㣬˵�����ڿ����Ӧ�����Ԫ��
		if (node->nl_OuterPos >= node->nl_OuterSize)		//�����˿��С��˵��������������飬����ȡ��һ���ڲ�Ԫ��(�������������)
		{
			node->nl_NeedNewInner = true;
			continue;
		}
		outerTupleSlot = node->nl_Block[node->nl_OuterPos++];			//ȡ��һ���Ԫ��
		Assert(!TupIsNull(outerTupleSlot));							//Ҫ�����Ԫ��ǿ�
		econtext->ecxt_outertuple = outerTupleSlot;					//����Context

		/*
		 * at this point we have a new pair of inner and outer tuples so we
		 * test the inner and outer tuples to see if they satisfy the node's
		 * qualification.
		 * �����ڲ�Ԫ�飬�����Ƿ�������������
		 *
		 * Only the joinquals determine MatchedOuter status, but all quals
		 * must pass to actually return the tuple.
		 * ����������������MatchOuter״̬����ʶ�Ƿ�ƥ�䣩
		 * �����ص�Ԫ�����������������
		 */
		ENL1_printf("testing qualification");

		if (ExecQual(joinqual, econtext, false))		//��������������
		{
			/* In an antijoin, we never return a matched tuple
			 * ����Ϊanti join����ô���ǾͲ���Ҫ�����ƥ���Ԫ��
			 * ����anti join��������not in�򲻵��������ʵ�֣���ô
			 * ������Ԫ��Ͳ��ÿ����ˣ�ֱ��ȥ����һ�����Ԫ�飬�����ǰ�ԭ�����뷨���µ��ڲ�Ԫ��
			 * �����õ���Anti Join�Ľ����˳���ԭ������һ�µģ���Ϊ����ǰ��ͳһ���ؽ����
			 */
			if (node->js.jointype == JOIN_ANTI)
			{
				node->nl_MatchedBlock[node->nl_OuterPos - 1] = true;
				continue;		/* return to top of loop */
			}

			/*
			 * In a semijoin, we'll consider returning the first match, but
			 * after that we're done with this outer tuple.
			 * ����semijoin(ע��������ָƥ�����������ĵ�һ�ԣ�
			 * ���ص�һ��ƥ���Ԫ�飬�������µ����Ԫ�飨ע���һ����ָ�������Ԫ���ҵ��ĵ�һ���ڲ�Ԫ�飩
			 * ע���������ڲ����⣬������ڣ������������һ���ڲ��ҵ���ƥ�����㣬
			 * ��ִ��continue���»ص�ѭ����ͷ��ȥ����һ�����Ԫ�飨�����߻��ܲ���ƥ�䣩�������ǰ�ԭ�����뷨ȥ���µ��ڲ�Ԫ��
			 * ����ע��һ�����ﲻ�ܱ��ƥ�䣬��Ϊ��ǰ���ڲ�Ԫ�鲻һ����������Ԫ��ƥ��ĵ�һ��������ֻ��continue
			 * ��ͨ��������Ԫ���Ƿ��й�ƥ�����������Ƿ�continue
			 * �����õ���Semi Join�Ľ����ԭ��һ�£���˳�������ı�
			 */
			if (node->js.jointype == JOIN_SEMI && node->nl_MatchedBlock[node->nl_OuterPos - 1])		//��������Ԫ��֮ǰ�й�ƥ����
				continue;

			node->nl_MatchedBlock[node->nl_OuterPos - 1] = true;		//��ʶ��ǰԪ����ƥ���

			if (otherqual == NIL || ExecQual(otherqual, econtext, false))		//������������
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
		 * ���������������������ڴ棬������һ���ڲ�Ԫ��
		 */
		ResetExprContext(econtext);

		ENL1_printf("qualification failed, looping");
	}
}

/* ----------------------------------------------------------------
 *		ExecInitNestLoop
 * ----------------------------------------------------------------
 * ��ѭ��Ƕ�׽��г�ʼ��
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
 * ����ɨ��
 * ��֮ǰ��ͬ��֮ǰ��ȡ�������Ԫ������ɨ���ڲ㣻
 * �����ǣ�����ÿ��������ɨ���ڲ�(�ٶ�ÿ���ڲ�Ԫ������ɨ����е�Ԫ��)
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
