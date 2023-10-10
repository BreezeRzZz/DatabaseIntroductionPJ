// CSelectView.cpp: 实现文件
//

#include "pch.h"
#include "BookSystem.h"
#include "CSelectView.h"
#include "MainFrm.h"


// CSelectView

IMPLEMENT_DYNCREATE(CSelectView, CTreeView)

CSelectView::CSelectView()
{

}

CSelectView::~CSelectView()
{
}

BEGIN_MESSAGE_MAP(CSelectView, CTreeView)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, &CSelectView::OnTvnSelchanged)
END_MESSAGE_MAP()


// CSelectView 诊断

#ifdef _DEBUG
void CSelectView::AssertValid() const
{
	CTreeView::AssertValid();
}

#ifndef _WIN32_WCE
void CSelectView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif
#endif //_DEBUG


// CSelectView 消息处理程序


void CSelectView::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();

	// TODO: 在此添加专用代码和/或调用基类

	//初始化
	m_treeCtrl = &GetTreeCtrl();
	//读入图标
	HICON icon[7] =
	{ 
		AfxGetApp()->LoadIconW(IDI_ICON_USER),
		AfxGetApp()->LoadIconW(IDI_ICON_BOOK),
		AfxGetApp()->LoadIconW(IDI_ICON_RESTOCK),
		AfxGetApp()->LoadIconW(IDI_ICON_PAYREFUND),
		AfxGetApp()->LoadIconW(IDI_ICON_SALE),
		AfxGetApp()->LoadIconW(IDI_ICON_BUY),
		AfxGetApp()->LoadIconW(IDI_ICON_BILL)
	};

	//写入图标列表，参数分别为：宽、高、格式、写入图标数
	m_imageList.Create(30, 30, ILC_COLOR32, 1, 1);
	for (int i = 0; i < 7; ++i)
		m_imageList.Add(icon[i]);
	//获取树控件:载入图标列表并选择显示模式
	m_treeCtrl->SetImageList(&m_imageList, TVSIL_NORMAL);
	//设置节点，这里设置并行的根节点
	m_treeCtrl->InsertItem(_T("个人信息"), 0, 0, NULL);
	m_treeCtrl->InsertItem(_T("图书信息查询及修改"), 1, 1, NULL);
	m_treeCtrl->InsertItem(_T("图书进货"), 2, 2, NULL);
	m_treeCtrl->InsertItem(_T("进货付款与退货"), 3, 3, NULL);
	m_treeCtrl->InsertItem(_T("添加新书"), 4, 4, NULL);
	m_treeCtrl->InsertItem(_T("书籍购买"), 5, 5, NULL);
	m_treeCtrl->InsertItem(_T("查看账单"), 6, 6, NULL);

	//设置控件Item高度
	m_treeCtrl->SetItemHeight(100);
}


void CSelectView::OnTvnSelchanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	//获取当前节点选中的项目
	HTREEITEM item = m_treeCtrl->GetSelectedItem();

	//获取选中项的文本内容
	CString str = m_treeCtrl->GetItemText(item);

	//根据文本内容进行跳转选择
	if (str == _T("个人信息"))
	{
		::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), NM_A, (WPARAM)NM_A, (LPARAM)0);
	}
	else if (str == _T("图书信息查询及修改"))
	{
		::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), NM_B, (WPARAM)NM_B, (LPARAM)0);
	}
	else if (str == _T("图书进货"))
	{
		::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), NM_C, (WPARAM)NM_C, (LPARAM)0);
	}
	else if (str == _T("进货付款与退货"))
	{
		::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), NM_D, (WPARAM)NM_D, (LPARAM)0);
	}
	else if (str == _T("添加新书"))
	{
		::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), NM_E, (WPARAM)NM_E, (LPARAM)0);
	}
	else if (str == _T("书籍购买"))
	{
		::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), NM_F, (WPARAM)NM_F, (LPARAM)0);
	}
	else if (str == _T("查看账单"))
	{
		::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), NM_G, (WPARAM)NM_G, (LPARAM)0);
	}
}

