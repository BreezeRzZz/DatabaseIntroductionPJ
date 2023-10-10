// CBuyDlg.cpp: 实现文件
//

#include "pch.h"
#include "BookSystem.h"
#include "CBuyDlg.h"
#include "CLoginDlg.h"
#include "CBuyConfirmDlg.h"


// CBuyDlg

IMPLEMENT_DYNCREATE(CBuyDlg, CFormView)

CBuyDlg::CBuyDlg()
	: CFormView(DIALOG_BUYBOOK)
	, buy_info(_T(""))
{

}

CBuyDlg::~CBuyDlg()
{
}

void CBuyDlg::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, buy_list);
	DDX_Text(pDX, IDC_EDIT1, buy_info);
}

BEGIN_MESSAGE_MAP(CBuyDlg, CFormView)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CBuyDlg::OnNMDblclkList1)
	ON_BN_CLICKED(IDC_BUTTON1, &CBuyDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CBuyDlg 诊断

#ifdef _DEBUG
void CBuyDlg::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CBuyDlg::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CBuyDlg 消息处理程序


//初始化：与查询界面初始化是一样的
void CBuyDlg::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 在此添加专用代码和/或调用基类

	CString str[] = { _T("ISBN"),_T("书籍名称"),
		_T("作者"),_T("出版社"),_T("书籍编号"),_T("零售价"),_T("库存量") };
	for (int i = 0; i < 7; ++i)
		buy_list.InsertColumn(i, str[i], LVCFMT_CENTER, 100);

	//设定显示格式：可选中整行、带网格的报表
	buy_list.SetExtendedStyle(buy_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	//默认显示所有图书
	char query[120] = "select * from book natural join sale order by bookid asc";
	auto res = PQexec(CLoginDlg::conn, query);
	int rows = PQntuples(res), cols = PQnfields(res);
	for (int i = 0; i < rows; ++i)
	{
		//插入一行
		buy_list.InsertItem(i, CString(PQgetvalue(res, i, 0)));
		//插入该行后续列
		for (int j = 1; j < cols; ++j)
			buy_list.SetItemText(i, j, CString(PQgetvalue(res, i, j)));
	}
	PQclear(res);
}


void CBuyDlg::OnNMDblclkList1(NMHDR* pNMHDR, LRESULT* pResult)
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
	//获取当前行的书籍ID
	CString cur_ID = buy_list.GetItemText(row, 4);

	//弹出确认窗口
	CBuyConfirmDlg dlg;
	dlg.DoModal();

	if (dlg.state == false)
		return;

	//按指定数量购买书籍
	//1、数据库中，减少sale表对应书籍的库存，并更新显示
	CString temp_string;
	temp_string.Format(_T("%d"), dlg.buy_num);
	int old_inventory = _ttoi(buy_list.GetItemText(row, 6));
	CString temp_query = _T("update sale set inventory=inventory-")
		+ temp_string + _T(" where bookid='") + cur_ID + _T("';");

	//2.如果购买成功，在数据库account表中添加一条账单记录

	//获取零售价，计算收入
	CString diff;
	double price = _ttof(buy_list.GetItemText(row, 5));
	diff.Format(_T("%lf"), price * dlg.buy_num);
	//插入一条记录
	temp_query += _T("insert into account values(") + diff + _T(");");
	CStringA query(temp_query);
	auto res = PQexec(CLoginDlg::conn, query.GetBuffer());
	if (PQresultStatus(res) == PGRES_COMMAND_OK)
	{
		temp_string.Format(_T("%d"), old_inventory - dlg.buy_num);
		buy_list.SetItemText(row, 6, temp_string);
		MessageBox(_T("购买成功!"));
	}
	else
		MessageBox(_T("库存不足，购买失败!"));
	PQclear(res);



	*pResult = 0;
}


//更新到初始化列表
void CBuyDlg::Update()
{
	//默认显示所有图书
	char query[120] = "select * from book natural join sale order by bookid asc";
	auto res = PQexec(CLoginDlg::conn, query);
	int rows = PQntuples(res), cols = PQnfields(res);
	for (int i = 0; i < rows; ++i)
	{
		//插入一行
		buy_list.InsertItem(i, CString(PQgetvalue(res, i, 0)));
		//插入该行后续列
		for (int j = 1; j < cols; ++j)
			buy_list.SetItemText(i, j, CString(PQgetvalue(res, i, j)));
	}
	PQclear(res);
}
//查找
void CBuyDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码

	//将输入内容同步到变量
	UpdateData(true);
	//清空上次查询的内容
	buy_list.DeleteAllItems();
	//判断查找字符串长度，如果为空，则恢复到初始状态(显示所有数据)
	if (buy_info.IsEmpty())
	{
		Update();
		MessageBox(_T("请输入查询内容!"));
		return;
	}
	//为方便查询，数据库种前5种属性都设为了字符相关类型
	//查找中，ISBN和编号必须输全,而书名、作者和出版社可以输入部分来匹配

	//注意，由于bookID是serial(int)类型，应与这些字符串类型的匹配分开，否则，
	//输入非数时整条语句就无法执行。

	//1、查找非ID
	CString temp_query = _T("select * from book natural join sale where ISBN ='")
		+ buy_info + _T("' or title like '%") + buy_info + _T("%' or author like '%")
		+ buy_info + _T("%' or press like '%") + buy_info + _T("%';");
	CStringA query;
	query = temp_query;
	auto res = PQexec(CLoginDlg::conn, query.GetBuffer());
	int rows = PQntuples(res), cols = PQnfields(res);
	for (int i = 0; i < rows; ++i)
	{
		//插入一行
		buy_list.InsertItem(i, CString(PQgetvalue(res, i, 0)));
		//插入该行后续列
		for (int j = 1; j < cols; ++j)
			buy_list.SetItemText(i, j, CString(PQgetvalue(res, i, j)));
	}
	PQclear(res);

	//2、查找ID，当且仅当上面查找到0条时
	if (rows == 0)
	{
		temp_query = _T("select * from book natural join sale where bookID=")
			+ buy_info + _T(";");
		query = temp_query;
		res = PQexec(CLoginDlg::conn, query.GetBuffer());

		rows = PQntuples(res), cols = PQnfields(res);
		for (int i = 0; i < rows; ++i)
		{
			//插入一行
			buy_list.InsertItem(i, CString(PQgetvalue(res, i, 0)));
			//插入该行后续列
			for (int j = 1; j < cols; ++j)
				buy_list.SetItemText(i, j, CString(PQgetvalue(res, i, j)));
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
