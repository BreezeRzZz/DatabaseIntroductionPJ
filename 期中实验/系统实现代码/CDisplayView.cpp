// CDisplayView.cpp: 实现文件
//

#include "pch.h"
#include "BookSystem.h"
#include "CDisplayView.h"


// CDisplayView

IMPLEMENT_DYNCREATE(CDisplayView, CView)

CDisplayView::CDisplayView()
{

}

CDisplayView::~CDisplayView()
{
}

BEGIN_MESSAGE_MAP(CDisplayView, CView)
END_MESSAGE_MAP()


// CDisplayView 绘图

void CDisplayView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO:  在此添加绘制代码
}


// CDisplayView 诊断

#ifdef _DEBUG
void CDisplayView::AssertValid() const
{
	CView::AssertValid();
}

#ifndef _WIN32_WCE
void CDisplayView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif
#endif //_DEBUG


// CDisplayView 消息处理程序
