// CBuyConfirmDlg.cpp: 实现文件
//

#include "pch.h"
#include "BookSystem.h"
#include "CBuyConfirmDlg.h"
#include "afxdialogex.h"


// CBuyConfirmDlg 对话框

IMPLEMENT_DYNAMIC(CBuyConfirmDlg, CDialogEx)

CBuyConfirmDlg::CBuyConfirmDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(DIALOG_BUYCONFIRM, pParent)
	, buy_num(0)
{

}

CBuyConfirmDlg::~CBuyConfirmDlg()
{
}

void CBuyConfirmDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, buy_num);
}


BEGIN_MESSAGE_MAP(CBuyConfirmDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CBuyConfirmDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CBuyConfirmDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CBuyConfirmDlg 消息处理程序


void CBuyConfirmDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(true);
	if (buy_num <= 0)
	{
		MessageBox(_T("购买数量不合理，请重试"));
		return;
	}
	CDialogEx::OnOK();
}


void CBuyConfirmDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	state = false;
	CDialogEx::OnCancel();
}


BOOL CBuyConfirmDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	UpdateData(false);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
