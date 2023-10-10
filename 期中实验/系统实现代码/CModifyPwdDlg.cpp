// CModifyPwdDlg.cpp: 实现文件
//

#include "pch.h"
#include "BookSystem.h"
#include "CModifyPwdDlg.h"
#include "afxdialogex.h"


// CModifyPwdDlg 对话框

IMPLEMENT_DYNAMIC(CModifyPwdDlg, CDialogEx)

CModifyPwdDlg::CModifyPwdDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(DIALOG_MODIFYPWD, pParent)
	, new_pwd(_T(""))
	, repeat_pwd(_T(""))
{

}

CModifyPwdDlg::~CModifyPwdDlg()
{
}

void CModifyPwdDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, new_pwd);
	DDX_Text(pDX, IDC_EDIT2, repeat_pwd);
}


BEGIN_MESSAGE_MAP(CModifyPwdDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CModifyPwdDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CModifyPwdDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CModifyPwdDlg 消息处理程序


void CModifyPwdDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	
	UpdateData(true);

	//检查密码长度，必须在4到20位之间
	if (new_pwd.GetLength() < 4 || new_pwd.GetLength() > 20)
	{
		MessageBox(_T("密码长度应在4到20位之间，请重试!"));
		return;
	}
	//检查两次输入密码是否一致
	if (new_pwd != repeat_pwd)
	{
		MessageBox(_T("两次输入密码不一致，请重试!"));
		return;
	}

	CDialogEx::OnOK();
}


void CModifyPwdDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	state = false;
	CDialogEx::OnCancel();
}
