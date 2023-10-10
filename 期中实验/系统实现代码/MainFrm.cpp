
// MainFrm.cpp: CMainFrame 类的实现
//

#include "pch.h"
#include "framework.h"
#include "BookSystem.h"

#include "MainFrm.h"
#include "CSelectView.h"
#include "CDisplayView.h"
#include "CUserDlg.h"
#include "CSearchDlg.h"
#include "CRestockDlg.h"
#include "CPayDlg.h"
#include "COnsaleDlg.h"
#include "CBuyDlg.h"
#include "CAccountDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()

	ON_MESSAGE(NM_A, OnMyChange)
	ON_MESSAGE(NM_B, OnMyChange)
	ON_MESSAGE(NM_C, OnMyChange)
	ON_MESSAGE(NM_D, OnMyChange)
	ON_MESSAGE(NM_E, OnMyChange)
	ON_MESSAGE(NM_F, OnMyChange)
	ON_MESSAGE(NM_G, OnMyChange)

END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // 状态行指示器
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame 构造/析构

CMainFrame::CMainFrame() noexcept
{
	// TODO: 在此添加成员初始化代码
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("未能创建状态栏\n");
		return -1;      // 未能创建
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT));

	//设置图标
	SetClassLongPtr(m_hWnd, GCLP_HICON, (LONG)AfxGetApp()->LoadIconW(IDI_ICON_WIN));
	//设置窗口标题
	SetTitle(_T("Book Sales Management System"));
	//窗口大小并居中
	MoveWindow(0, 0, 1280, 800);
	CenterWindow();

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CFrameWnd::PreCreateWindow(cs))
		return FALSE;
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return TRUE;
}

// CMainFrame 诊断

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG


// CMainFrame 消息处理程序



BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	// TODO: 在此添加专用代码和/或调用基类

	//拆分窗口为1行2列（呈现左右分布形式）
	m_spliter.CreateStatic(this, 1, 2);

	//左边部分的窗口
	m_spliter.CreateView(0, 0, RUNTIME_CLASS(CSelectView), CSize(200, 800), pContext);
	//右边部分的窗口
	m_spliter.CreateView(0, 1, RUNTIME_CLASS(CDisplayView), CSize(1080, 800), pContext);
	//return CFrameWnd::OnCreateClient(lpcs, pContext);
	return true;
}

afx_msg LRESULT CMainFrame::OnMyChange(WPARAM wParam, LPARAM lParam)
{
	CCreateContext Context;
	switch (wParam)
	{
	case NM_A:
	{
		Context.m_pNewViewClass = RUNTIME_CLASS(CUserDlg);
		Context.m_pCurrentFrame = this;
		Context.m_pLastView = (CFormView*)m_spliter.GetPane(0, 1);
		m_spliter.DeleteView(0, 1);
		m_spliter.CreateView(0, 1, RUNTIME_CLASS(CUserDlg), CSize(600, 500), &Context);
		CUserDlg* pNewView = (CUserDlg*)m_spliter.GetPane(0, 1);
		m_spliter.RecalcLayout();
		pNewView->OnInitialUpdate();
		m_spliter.SetActivePane(0, 1);
		break;
	}
	case NM_B:
	{
		Context.m_pNewViewClass = RUNTIME_CLASS(CSearchDlg);
		Context.m_pCurrentFrame = this;
		Context.m_pLastView = (CFormView*)m_spliter.GetPane(0, 1);
		m_spliter.DeleteView(0, 1);
		m_spliter.CreateView(0, 1, RUNTIME_CLASS(CSearchDlg), CSize(600, 500), &Context);
		CSearchDlg* pNewView = (CSearchDlg*)m_spliter.GetPane(0, 1);
		m_spliter.RecalcLayout();
		pNewView->OnInitialUpdate();
		m_spliter.SetActivePane(0, 1);
		break;
	}
	case NM_C:
; {
	Context.m_pNewViewClass = RUNTIME_CLASS(CRestockDlg);
	Context.m_pCurrentFrame = this;
	Context.m_pLastView = (CFormView*)m_spliter.GetPane(0, 1);
	m_spliter.DeleteView(0, 1);
	m_spliter.CreateView(0, 1, RUNTIME_CLASS(CRestockDlg), CSize(600, 500), &Context);
	CRestockDlg* pNewView = (CRestockDlg*)m_spliter.GetPane(0, 1);
	m_spliter.RecalcLayout();
	pNewView->OnInitialUpdate();
	m_spliter.SetActivePane(0, 1);
	break;
}
	case NM_D:
	{
		Context.m_pNewViewClass = RUNTIME_CLASS(CPayDlg);
		Context.m_pCurrentFrame = this;
		Context.m_pLastView = (CFormView*)m_spliter.GetPane(0, 1);
		m_spliter.DeleteView(0, 1);
		m_spliter.CreateView(0, 1, RUNTIME_CLASS(CPayDlg), CSize(600, 500), &Context);
		CPayDlg* pNewView = (CPayDlg*)m_spliter.GetPane(0, 1);
		m_spliter.RecalcLayout();
		pNewView->OnInitialUpdate();
		m_spliter.SetActivePane(0, 1);
		break;
	}
	case NM_E:
	{
		Context.m_pNewViewClass = RUNTIME_CLASS(COnsaleDlg);
		Context.m_pCurrentFrame = this;
		Context.m_pLastView = (CFormView*)m_spliter.GetPane(0, 1);
		m_spliter.DeleteView(0, 1);
		m_spliter.CreateView(0, 1, RUNTIME_CLASS(COnsaleDlg), CSize(600, 500), &Context);
		COnsaleDlg* pNewView = (COnsaleDlg*)m_spliter.GetPane(0, 1);
		m_spliter.RecalcLayout();
		pNewView->OnInitialUpdate();
		m_spliter.SetActivePane(0, 1);
		break;
	}
	case NM_F:
	{
		Context.m_pNewViewClass = RUNTIME_CLASS(CBuyDlg);
		Context.m_pCurrentFrame = this;
		Context.m_pLastView = (CFormView*)m_spliter.GetPane(0, 1);
		m_spliter.DeleteView(0, 1);
		m_spliter.CreateView(0, 1, RUNTIME_CLASS(CBuyDlg), CSize(600, 500), &Context);
		CBuyDlg* pNewView = (CBuyDlg*)m_spliter.GetPane(0, 1);
		m_spliter.RecalcLayout();
		pNewView->OnInitialUpdate();
		m_spliter.SetActivePane(0, 1);
		break;
	}
	case NM_G:
	{
		Context.m_pNewViewClass = RUNTIME_CLASS(CAccountDlg);
		Context.m_pCurrentFrame = this;
		Context.m_pLastView = (CFormView*)m_spliter.GetPane(0, 1);
		m_spliter.DeleteView(0, 1);
		m_spliter.CreateView(0, 1, RUNTIME_CLASS(CAccountDlg), CSize(600, 500), &Context);
		CAccountDlg* pNewView = (CAccountDlg*)m_spliter.GetPane(0, 1);
		m_spliter.RecalcLayout();
		pNewView->OnInitialUpdate();
		m_spliter.SetActivePane(0, 1);
		break;
	}

	}
	return 0;
}
