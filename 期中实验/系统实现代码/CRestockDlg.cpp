// CRestockDlg.cpp: 实现文件
//

#include "pch.h"
#include "BookSystem.h"
#include "CRestockDlg.h"
#include "CLoginDlg.h"


// CRestockDlg

IMPLEMENT_DYNCREATE(CRestockDlg, CFormView)

CRestockDlg::CRestockDlg()
	: CFormView(DIALOG_RESTOCK)
	, restock_ISBN(_T(""))
	, restock_title(_T(""))
	, restock_author(_T(""))
	, restock_press(_T(""))
	, restock_price(_T(""))
	, restock_num(_T(""))
{

}

CRestockDlg::~CRestockDlg()
{
}

void CRestockDlg::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, restock_cbx);
	DDX_Text(pDX, IDC_EDIT2, restock_ISBN);
	DDX_Text(pDX, IDC_EDIT1, restock_title);
	DDX_Text(pDX, IDC_EDIT3, restock_author);
	DDX_Text(pDX, IDC_EDIT4, restock_press);
	DDX_Text(pDX, IDC_EDIT6, restock_price);
	DDX_Text(pDX, IDC_EDIT7, restock_num);
}

BEGIN_MESSAGE_MAP(CRestockDlg, CFormView)
	ON_BN_CLICKED(IDC_BUTTON1, &CRestockDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CRestockDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CRestockDlg 诊断

#ifdef _DEBUG
void CRestockDlg::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CRestockDlg::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CRestockDlg 消息处理程序


void CRestockDlg::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 在此添加专用代码和/或调用基类

	//库存中有过的ID即指book表，曾经即包含现在库存为0的图书。
	//添加初始下拉框内容：获取现有的图书id
	char query[120] = "select bookid from sale order by bookid asc;";
	auto res = PQexec(CLoginDlg::conn, query);

	int rows = PQntuples(res);
	for (int i = 0; i < rows; ++i)
		restock_cbx.AddString(CString(PQgetvalue(res, i, 0)));
	PQclear(res);

	//默认选中第一个
	restock_cbx.SetCurSel(0);
	return;
}

//旧书进货
void CRestockDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码

	//更新控件值到变量
	UpdateData(true);
	if (restock_price.IsEmpty() || restock_num.IsEmpty())
	{
		MessageBox(_T("请输入进货价格和数量"));
		return;
	}
	CString bookid;
	restock_cbx.GetLBText(restock_cbx.GetCurSel(), bookid);
	//获取对应bookid书籍的ISBN

	//在book表中找
	CString temp_query = _T("select ISBN from sale where bookid=")
		+ bookid + _T(";");
	CStringA query(temp_query);
	auto res = PQexec(CLoginDlg::conn, query.GetBuffer());

	temp_query = _T("insert into restocklist values('")
		+ CString(PQgetvalue(res, 0, 0)) + _T("',")				//ISBN
		+ restock_price + _T(",")								//price
		+ restock_num + _T(",")									//num;
		+ restock_num + _T(");");								//remain
	query = temp_query;
	PQclear(res);
	res = PQexec(CLoginDlg::conn, query.GetBuffer());
	if (PQresultStatus(res) == PGRES_COMMAND_OK)
		MessageBox(_T("添加进货记录成功!"));
	else
		MessageBox(_T("添加进货记录失败!"));
	PQclear(res);
	return;
}

//新书进货
void CRestockDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码

	//更新控件值到变量
	UpdateData(true);
	if (restock_ISBN.IsEmpty() || restock_title.IsEmpty() || restock_author.IsEmpty()
		|| restock_press.IsEmpty())
	{
		MessageBox(_T("书籍信息不完整，请重试!"));
		return;
	}
	else if (restock_price.IsEmpty() || restock_num.IsEmpty())
	{
		MessageBox(_T("请输入进货价格和数量!"));
		return;
	}
	else if (!checkISBN(restock_ISBN))
	{
		MessageBox(_T("您输入的ISBN不合理，请重试。"));
		return;
	}

	//1、添加到restocklist表
	CString temp_query = _T("insert into restocklist values('")
		+ restock_ISBN + _T("',")
		+ restock_price + _T(",")
		+ restock_num + _T(",")
		+ restock_num + _T(");");
	CStringA query(temp_query);
	auto res = PQexec(CLoginDlg::conn, query.GetBuffer());
	//查找异常
	if (PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		PQclear(res);
		return;
	}

	//2、添加新书到book表(如果这本书实际上在book表里已经有了就会插入失败，这不影响功能的实现)
	temp_query = _T("insert into book values('")
		+ restock_ISBN + _T("','")
		+ restock_title + _T("','")
		+ restock_author + _T("','")
		+ restock_press + _T("');");
	query = temp_query;
	res = PQexec(CLoginDlg::conn, query.GetBuffer());
	if (PQresultStatus(res) == PGRES_COMMAND_OK)
		MessageBox(_T("添加进货记录成功!"));
	else
		MessageBox(_T("添加进货记录失败!"));
	PQclear(res);

	return;
}

//检查ISBN是否合理：通过检查校验位
bool CRestockDlg::checkISBN(CString& ISBN)
{
	//必须为13位
	if (ISBN.GetLength() != 13)
		return false;
	//必须均为数字
	for (int i = 0; i < 13; ++i)
	{
		if (!isdigit(ISBN[i]))
			return false;
	}
	int sum = 0;
	for (int i = 0; i < 12; ++i)
		sum += (i % 2) ? (3 * (ISBN[i] - '0')) : (ISBN[i] - '0');
	if ((10 - sum % 10) % 10 + '0' == ISBN[12])
		return true;
	else
		return false;
}