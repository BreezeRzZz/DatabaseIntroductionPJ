// CPayConfirmDlg.cpp: 实现文件
//

#include "pch.h"
#include "BookSystem.h"
#include "CPayConfirmDlg.h"
#include "afxdialogex.h"


// CPayConfirmDlg 对话框

IMPLEMENT_DYNAMIC(CPayConfirmDlg, CDialogEx)

CPayConfirmDlg::CPayConfirmDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(DIALOG_PAYCONFIRM, pParent)
{

}

CPayConfirmDlg::~CPayConfirmDlg()
{
}

void CPayConfirmDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPayConfirmDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CPayConfirmDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CPayConfirmDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CPayConfirmDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// CPayConfirmDlg 消息处理程序


void CPayConfirmDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	s = Cancel;
	CDialogEx::OnCancel();
}


void CPayConfirmDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	s = PayConfirm;
	CDialogEx::OnOK();
}


void CPayConfirmDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	s = RefundConfirm;
	CDialogEx::OnOK();
}
