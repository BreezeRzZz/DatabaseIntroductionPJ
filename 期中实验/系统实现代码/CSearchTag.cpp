// CSearchTag.cpp: 实现文件
//

#include "pch.h"
#include "BookSystem.h"
#include "CSearchTag.h"
#include "afxdialogex.h"


// CSearchTag 对话框

IMPLEMENT_DYNAMIC(CSearchTag, CDialogEx)

CSearchTag::CSearchTag(CWnd* pParent /*=nullptr*/)
	: CDialogEx(DIALOG_SRARCHTAG, pParent)
	, tag_ISBN(_T(""))
	, tag_ID(_T(""))
	, tag_title(_T(""))
	, tag_author(_T(""))
	, tag_press(_T(""))
{

}

CSearchTag::~CSearchTag()
{
}

void CSearchTag::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, tag_ISBN);
	DDX_Text(pDX, IDC_EDIT2, tag_ID);
	DDX_Text(pDX, IDC_EDIT3, tag_title);
	DDX_Text(pDX, IDC_EDIT4, tag_author);
	DDX_Text(pDX, IDC_EDIT5, tag_press);
}


BEGIN_MESSAGE_MAP(CSearchTag, CDialogEx)
	ON_BN_CLICKED(IDOK, &CSearchTag::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CSearchTag::OnBnClickedCancel)
END_MESSAGE_MAP()


// CSearchTag 消息处理程序


void CSearchTag::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(true);
	if (tag_ISBN.IsEmpty() && tag_ID.IsEmpty() && tag_title.IsEmpty() && tag_author.IsEmpty() && tag_press.IsEmpty())
	{
		MessageBox(_T("请至少输入一条信息!"));
		return;
	}
	CDialogEx::OnOK();
}


void CSearchTag::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	state = false;
	CDialogEx::OnCancel();
}
