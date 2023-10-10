// CAccountDlg.cpp: 实现文件
//

#include "pch.h"
#include "BookSystem.h"
#include "CAccountDlg.h"
#include "CLoginDlg.h"


// CAccountDlg

IMPLEMENT_DYNCREATE(CAccountDlg, CFormView)

CAccountDlg::CAccountDlg()
	: CFormView(DIALOG_ACCOUNT)
	, from_year(0)
	, from_month(0)
	, from_day(0)
	, to_year(0)
	, to_month(0)
	, to_day(0)
{

}

CAccountDlg::~CAccountDlg()
{
}

void CAccountDlg::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, account_list);
	DDX_Text(pDX, IDC_EDIT1, from_year);
	DDX_Text(pDX, IDC_EDIT2, from_month);
	DDX_Text(pDX, IDC_EDIT3, from_day);
	DDX_Text(pDX, IDC_EDIT4, to_year);
	DDX_Text(pDX, IDC_EDIT5, to_month);
	DDX_Text(pDX, IDC_EDIT6, to_day);
	DDX_Control(pDX, IDC_COMBO1, from_hour);
	DDX_Control(pDX, IDC_COMBO2, from_min);
	DDX_Control(pDX, IDC_COMBO3, from_sec);
	DDX_Control(pDX, IDC_COMBO4, to_hour);
	DDX_Control(pDX, IDC_COMBO5, to_min);
	DDX_Control(pDX, IDC_COMBO6, to_sec);
}

BEGIN_MESSAGE_MAP(CAccountDlg, CFormView)
	ON_BN_CLICKED(IDC_BUTTON1, &CAccountDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CAccountDlg 诊断

#ifdef _DEBUG
void CAccountDlg::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CAccountDlg::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CAccountDlg 消息处理程序


//默认显示所有收支
void CAccountDlg::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 在此添加专用代码和/或调用基类

	CString str[] = { _T("收入/支出"),_T("收支时间") };
	for (int i = 0; i < 2; ++i)
		account_list.InsertColumn(i, str[i], LVCFMT_CENTER, 200);

	//设定显示格式：可选中整行、带网格的报表
	account_list.SetExtendedStyle(account_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	//设置下拉框
	CString temp;
	//hour
	for (int i = 0; i < 24; ++i)
	{
		temp.Format(_T("%02d"), i);
		from_hour.AddString(temp);
		to_hour.AddString(temp);
	}
	//minute
	for (int i = 0; i < 60; ++i)
	{
		temp.Format(_T("%02d"), i);
		from_min.AddString(temp);
		to_min.AddString(temp);
	}
	//second
	for (int i = 0; i < 60; ++i)
	{
		temp.Format(_T("%02d"), i);
		from_sec.AddString(temp);
		to_sec.AddString(temp);
	}

	from_hour.SetCurSel(0);
	from_min.SetCurSel(0);
	from_sec.SetCurSel(0);
	to_hour.SetCurSel(0);
	to_min.SetCurSel(0);
	to_sec.SetCurSel(0);

	//默认显示所有收支
	char query[120] = "select * from account order by difftime desc";
	auto res = PQexec(CLoginDlg::conn, query);
	int rows = PQntuples(res), cols = PQnfields(res);
	for (int i = 0; i < rows; ++i)
	{
		//插入一行
		//如果是正数前面添加'+'号，否则不变
		if (atof(PQgetvalue(res, i, 0)) > 0)
			account_list.InsertItem(i, _T("+")+CString(PQgetvalue(res, i, 0)));
		else
			account_list.InsertItem(i, CString(PQgetvalue(res, i, 0)));
		//插入该行后续列
		for (int j = 1; j < cols; ++j)
			account_list.SetItemText(i, j, CString(PQgetvalue(res, i, j)));
	}
	PQclear(res);
}


void CAccountDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码

	UpdateData(true);
	CString from, to;
	from.Format(_T("%04d-%02d-%02d"), from_year, from_month, from_day);
	to.Format(_T("%04d-%02d-%02d"), to_year, to_month, to_day);

	CString f_hour, f_min, f_sec, t_hour, t_min, t_sec;
	from_hour.GetLBText(from_hour.GetCurSel(), f_hour);
	from_min.GetLBText(from_min.GetCurSel(), f_min);
	from_sec.GetLBText(from_sec.GetCurSel(), f_sec);
	to_hour.GetLBText(to_hour.GetCurSel(), t_hour);
	to_min.GetLBText(to_min.GetCurSel(), t_min);
	to_sec.GetLBText(to_sec.GetCurSel(), t_sec);

	from += _T(" ") + f_hour + _T(":") + f_min + _T(":") + f_sec;
	to += _T(" ") + t_hour + _T(":") + t_min + _T(":") + t_sec;

	CString temp_query = _T("select * from account where difftime between '")
		+ from + _T("' and '") + to + _T("';");
	CStringA query(temp_query);
	auto res = PQexec(CLoginDlg::conn, query.GetBuffer());

	//重新显示内容
	account_list.DeleteAllItems();
	int rows = PQntuples(res), cols = PQnfields(res);
	for (int i = 0; i < rows; ++i)
	{
		//插入一行
		account_list.InsertItem(i, CString(PQgetvalue(res, i, 0)));
		//插入该行后续列
		for (int j = 1; j < cols; ++j)
			account_list.SetItemText(i, j, CString(PQgetvalue(res, i, j)));
	}
	PQclear(res);
}
