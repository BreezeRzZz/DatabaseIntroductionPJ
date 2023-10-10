// CLoginDlg.cpp: 实现文件
//

#include "pch.h"
#include "BookSystem.h"
#include "CLoginDlg.h"
#include "afxdialogex.h"
#include <fstream>

CString CLoginDlg::login_username = _T("");
CString CLoginDlg::login_password = _T("");
PGconn* CLoginDlg::conn = 0;
// CLoginDlg 对话框

IMPLEMENT_DYNAMIC(CLoginDlg, CDialogEx)

CLoginDlg::CLoginDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(DIALOG_LOGIN, pParent)
{

}

CLoginDlg::~CLoginDlg()
{
}

void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, login_username);
	DDX_Text(pDX, IDC_EDIT2, login_password);
}


BEGIN_MESSAGE_MAP(CLoginDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CLoginDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CLoginDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CLoginDlg 消息处理程序


void CLoginDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码

	//将输入内容同步到变量
	UpdateData(true);

	//读入配置文件登录
	std::ifstream in("login.ini");
	std::string username, password;
	in >> username >> password;
	char conninfo[200] = "host=127.0.0.1 port=5432 user=";
	strcat(conninfo, username.c_str());
	strcat(conninfo, " password=");
	strcat(conninfo, password.c_str());
	strcat(conninfo, " dbname=BookSalesManagementSystem client_encoding=GBK");
	
	//提醒不能输入为空
	if (login_username.IsEmpty() || login_password.IsEmpty())
	{
		MessageBox(_T("输入内容不能为空"));
		return;
	}
	//先登录数据库，通过检查administrator表来决定是否登入
	conn = PQconnectdb(conninfo);
	//连接成功
	if (PQstatus(conn) == CONNECTION_OK)
	{
		CStringA temp_pwd(login_password);
		CString encrypted_pwd = CString(PQencryptPasswordConn(conn, temp_pwd.GetBuffer(),"", "md5"));
		encrypted_pwd = encrypted_pwd.Right(encrypted_pwd.GetLength() - 3);	//去掉开头的md5这3个字符

		CString temp_query = _T("select username,password from administrator where username='")
			+ login_username + _T("' and password='") + encrypted_pwd
			+ _T("';");
		CStringA query(temp_query);
		auto res = PQexec(conn, query.GetBuffer());
		if (PQntuples(res) > 0)
		{
			MessageBox(_T("登录成功!"));
			CDialogEx::OnOK();
		}
		else
		{
			MessageBox(_T("用户名或密码不正确，请重试"));
			return;
		}
		PQclear(res);
	}
	else					//连接失败
	{
		MessageBox(_T("用户名或密码不正确，请重试"));
		return;
	}
}



void CLoginDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	exit(0);
}


BOOL CLoginDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
