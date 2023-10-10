// CPayDlg.cpp: 实现文件
//

#include "pch.h"
#include "BookSystem.h"
#include "CPayDlg.h"
#include "CPayConfirmDlg.h"
#include "CLoginDlg.h"


// CPayDlg

IMPLEMENT_DYNCREATE(CPayDlg, CFormView)

CPayDlg::CPayDlg()
	: CFormView(DIALOG_PAY)
	, pay_info(_T(""))
{

}

CPayDlg::~CPayDlg()
{
}

void CPayDlg::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, pay_list);
	DDX_Text(pDX, IDC_EDIT1, pay_info);
}

BEGIN_MESSAGE_MAP(CPayDlg, CFormView)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CPayDlg::OnNMDblclkList1)
	ON_BN_CLICKED(IDC_BUTTON1, &CPayDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CPayDlg 诊断

#ifdef _DEBUG
void CPayDlg::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPayDlg::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CPayDlg 消息处理程序


void CPayDlg::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 在此添加专用代码和/或调用基类

	CString str[] = { _T("ISBN"),_T("书籍名称"),
		_T("作者"),_T("出版社"),_T("进货价"),_T("进货量"),_T("订单编号"),
		_T("付款状态"),_T("是否到货") };
	for (int i = 0; i < 9; ++i)
		pay_list.InsertColumn(i, str[i], LVCFMT_CENTER, 100);

	//设定显示格式：可选中整行、带网格的报表
	pay_list.SetExtendedStyle(pay_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	//显示进货列表(按orderID升序)
	char query[200] = "select ISBN,title,author,press,restockprice,restocknumber,orderID,status,arrival from book natural join restocklist where arrival='未到货' order by orderID asc";
	auto res = PQexec(CLoginDlg::conn, query);
	int rows = PQntuples(res), cols = PQnfields(res);
	for (int i = 0; i < rows; ++i)
	{
		//插入一行
		pay_list.InsertItem(i, CString(PQgetvalue(res, i, 0)));
		//插入该行后续列
		for (int j = 1; j < cols; ++j)
			pay_list.SetItemText(i, j, CString(PQgetvalue(res, i, j)));
	}
	PQclear(res);
}

//双击付款/退货
void CPayDlg::OnNMDblclkList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	//获取双击处的行
	int row = pNMItemActivate->iItem;
	if (row == -1)			//越界处理
	{
		MessageBox(_T("请选择表格内的区域!"));
		return;
	}
	//确认付款状态
	CString old_status = pay_list.GetItemText(row, 7);
	if (old_status == _T("已付款"))
	{
		MessageBox(_T("该订单已付款!"));
		return;
	}
	else if (old_status == _T("已退货"))
	{
		MessageBox(_T("该订单已退货!"));
		return;
	}
	//弹出确认窗口
	CPayConfirmDlg dlg;
	dlg.DoModal();

	if (dlg.s == CPayConfirmDlg::Cancel)
		return;

	//修改付款状态到数据库的restocklist表中,用orderID选取对应行
	CString order;
	if (dlg.s == CPayConfirmDlg::PayConfirm)
		order = _T("已付款");
	else if (dlg.s == CPayConfirmDlg::RefundConfirm)
		order = _T("已退货");
	CString orderID = pay_list.GetItemText(row, 6);
	CString temp_query = _T("update restocklist set status='") + order +
		_T("' where status='未付款' and orderID=") + orderID + _T(";");


	if (old_status == _T("未付款"))
	{
		if (dlg.s == CPayConfirmDlg::PayConfirm)
		{
			//若付款，应在account中插入一条支出

			//获取该行的进货价格和数量
			double price = _ttof(pay_list.GetItemText(row, 4));
			int num = _ttoi(pay_list.GetItemText(row, 5));
			CString diff;
			diff.Format(_T("%lf"), price * num);
			temp_query += _T("insert into account values(-") + diff + _T(");end;");
			CStringA query(temp_query);
			auto res = PQexec(CLoginDlg::conn, query.GetBuffer());

			if (PQresultStatus(res) == PGRES_COMMAND_OK)
			{
				pay_list.SetItemText(row, 7, _T("已付款"));
				MessageBox(_T("付款成功!"));
			}
			else
				MessageBox(_T("付款失败!"));
			PQclear(res);
		}
		else if (dlg.s == CPayConfirmDlg::RefundConfirm)
		{
			temp_query += _T("end;");
			CStringA query(temp_query);
			auto res = PQexec(CLoginDlg::conn, query.GetBuffer());
			if (PQresultStatus(res) == PGRES_COMMAND_OK)
			{
				pay_list.SetItemText(row, 7, _T("已退货"));
				MessageBox(_T("退货成功!"));
			}
			else
				MessageBox(_T("退货失败"));
			PQclear(res);
		}
	}

	*pResult = 0;
}


void CPayDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码

	//将输入内容同步到变量
	UpdateData(true);
	//清空上次查询的内容
	pay_list.DeleteAllItems();
	//判断查找字符串长度，如果为空，则恢复到初始状态(显示所有数据)
	if (pay_info.IsEmpty())
	{
		Update();
		MessageBox(_T("请输入查询内容!"));
		return;
	}

	CString temp_query = _T("select ISBN,title,author,press,restockprice,restocknumber,orderID,status,arrival from book natural join restocklist where arrival='未到货' and (ISBN='")
		+ pay_info + _T("' or title like '%") + pay_info + _T("%' or author like '%")
		+ pay_info + _T("%' or press like '%") + pay_info + _T("%');");
	CStringA query;
	query = temp_query;
	auto res = PQexec(CLoginDlg::conn, query.GetBuffer());
	int rows = PQntuples(res), cols = PQnfields(res);
	for (int i = 0; i < rows; ++i)
	{
		//插入一行
		pay_list.InsertItem(i, CString(PQgetvalue(res, i, 0)));
		//插入该行后续列
		for (int j = 1; j < cols; ++j)
			pay_list.SetItemText(i, j, CString(PQgetvalue(res, i, j)));
	}
	PQclear(res);

	//查找ID，当且仅当上面查找到0条时
	if (rows == 0)
	{
		temp_query = _T("select ISBN, title, author, press, restockprice, restocknumber, orderID, status, arrival from book natural join restocklist where arrival = '未到货' and orderID=")
			+ pay_info + _T(";");
		query = temp_query;
		res = PQexec(CLoginDlg::conn, query.GetBuffer());

		rows = PQntuples(res), cols = PQnfields(res);
		for (int i = 0; i < rows; ++i)
		{
			//插入一行
			pay_list.InsertItem(i, CString(PQgetvalue(res, i, 0)));
			//插入该行后续列
			for (int j = 1; j < cols; ++j)
				pay_list.SetItemText(i, j, CString(PQgetvalue(res, i, j)));
		}
		PQclear(res);
	}

	//通知查找到的数据条数
	CString str;
	str.Format(_T("共查找到%d条相关数据"), rows);
	if (rows > 0)
		MessageBox(str);
	else
		MessageBox(_T("未查找到相关数据!"));
}

//更新到初始化列表
void CPayDlg::Update()
{
	//默认显示所有订单
	char query[200] = "select ISBN,title,author,press,restockprice,restocknumber,orderID,status,arrival from book natural join restocklist where arrival='未到货' order by orderID asc";
	auto res = PQexec(CLoginDlg::conn, query);
	int rows = PQntuples(res), cols = PQnfields(res);
	for (int i = 0; i < rows; ++i)
	{
		//插入一行
		pay_list.InsertItem(i, CString(PQgetvalue(res, i, 0)));
		//插入该行后续列
		for (int j = 1; j < cols; ++j)
			pay_list.SetItemText(i, j, CString(PQgetvalue(res, i, j)));
	}
	PQclear(res);
}