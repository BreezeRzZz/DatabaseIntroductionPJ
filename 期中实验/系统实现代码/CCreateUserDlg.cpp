// CCreateUserDlg.cpp: 实现文件
//

#include "pch.h"
#include "BookSystem.h"
#include "CCreateUserDlg.h"
#include "afxdialogex.h"
#include "CLoginDlg.h"


// CCreateUserDlg 对话框

IMPLEMENT_DYNAMIC(CCreateUserDlg, CDialogEx)

CCreateUserDlg::CCreateUserDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(DIALOG_CREATEUSER, pParent)
	, create_username(_T(""))
	, create_pwd(_T(""))
{

}

CCreateUserDlg::~CCreateUserDlg()
{
}

void CCreateUserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, create_username);
	DDX_Text(pDX, IDC_EDIT2, create_pwd);
}


BEGIN_MESSAGE_MAP(CCreateUserDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CCreateUserDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CCreateUserDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CCreateUserDlg 消息处理程序

//创建用户
void CCreateUserDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码

	UpdateData(true);

	//检查密码长度，必须在4到20位之间
	if (create_pwd.GetLength() < 4 || create_pwd.GetLength() > 20)
	{
		MessageBox(_T("密码长度应在4到20位之间，请重试!"));
		return;
	}


	//向administrator表插入新用户信息
	CStringA temp_pwd(create_pwd);
	CString encrypted_pwd = CString(PQencryptPasswordConn(CLoginDlg::conn, temp_pwd.GetBuffer(),"", "md5"));
	encrypted_pwd = encrypted_pwd.Right(encrypted_pwd.GetLength() - 3);	//去掉开头的md5这3个字符

	CString temp_query = _T("insert into administrator(username,password) values('")
		+ create_username + _T("','")
		+ encrypted_pwd + _T("');");
	CStringA query(temp_query);
	auto res = PQexec(CLoginDlg::conn, query.GetBuffer());
	if (PQresultStatus(res) == PGRES_COMMAND_OK)
	{
		MessageBox(_T("创建成功!"));
		ifOK = true;
		PQclear(res);
		CDialogEx::OnOK();
	}
	else
	{
		MessageBox(_T("创建失败，该用户已存在!"));
		PQclear(res);
		return;
	}
}


void CCreateUserDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}
