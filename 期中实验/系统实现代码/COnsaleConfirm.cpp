// COnsaleConfirm.cpp: 实现文件
//

#include "pch.h"
#include "BookSystem.h"
#include "COnsaleConfirm.h"
#include "afxdialogex.h"


// COnsaleConfirm 对话框

IMPLEMENT_DYNAMIC(COnsaleConfirm, CDialogEx)

COnsaleConfirm::COnsaleConfirm(CWnd* pParent /*=nullptr*/)
	: CDialogEx(DIALOG_ONSALECONFIRM, pParent)
	, onsale_price(0)
	, onsale_num(0)
{

}

COnsaleConfirm::~COnsaleConfirm()
{
}

void COnsaleConfirm::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, onsale_price);
	DDX_Text(pDX, IDC_EDIT2, onsale_num);
}


BEGIN_MESSAGE_MAP(COnsaleConfirm, CDialogEx)
	ON_BN_CLICKED(IDOK, &COnsaleConfirm::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &COnsaleConfirm::OnBnClickedCancel)
END_MESSAGE_MAP()


// COnsaleConfirm 消息处理程序


void COnsaleConfirm::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	
	UpdateData(true);
	//检测给定的price，必须大于0
	if (onsale_price <= 0)
	{
		MessageBox(_T("零售价格不合理，请重新输入。"));
		return;
	}
	//检查上架数量，应大于0
	if (onsale_num <= 0)
	{
		MessageBox(_T("上架数量，请重新输入。"));
		return;
	}
	CDialogEx::OnOK();
}


void COnsaleConfirm::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	state = false;
	CDialogEx::OnCancel();
}
