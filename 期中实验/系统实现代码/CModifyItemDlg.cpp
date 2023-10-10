// CModifyItemDlg.cpp: 实现文件
//

#include "pch.h"
#include "BookSystem.h"
#include "CModifyItemDlg.h"
#include "afxdialogex.h"


// CModifyItemDlg 对话框

IMPLEMENT_DYNAMIC(CModifyItemDlg, CDialogEx)

CModifyItemDlg::CModifyItemDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(DIALOG_MODIFYITEM, pParent)
	, newValue(_T(""))
{

}

CModifyItemDlg::~CModifyItemDlg()
{
}

void CModifyItemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT2, newValue);
}


BEGIN_MESSAGE_MAP(CModifyItemDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CModifyItemDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CModifyItemDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CModifyItemDlg 消息处理程序


void CModifyItemDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	
	//获取新值
	UpdateData(true);
	CDialogEx::OnOK();
}


void CModifyItemDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码

	//直接关闭，并设定state在CSearchDlg中判断
	state = false;
	CDialogEx::OnCancel();
}
