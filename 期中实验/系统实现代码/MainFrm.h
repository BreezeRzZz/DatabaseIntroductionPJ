
// MainFrm.h: CMainFrame 类的接口
//

#pragma once

#define NM_A (WM_USER+1)
#define NM_B (WM_USER+2)
#define NM_C (WM_USER+3)
#define NM_D (WM_USER+4)
#define NM_E (WM_USER+5)
#define NM_F (WM_USER+6)
#define NM_G (WM_USER+7)

class CMainFrame : public CFrameWnd
{
	
protected: // 仅从序列化创建
	CMainFrame() noexcept;
	DECLARE_DYNCREATE(CMainFrame)

// 特性
public:

// 操作
public:

// 重写
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 实现
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // 控件条嵌入成员
	CStatusBar        m_wndStatusBar;

// 生成的消息映射函数
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
private:
	CSplitterWnd m_spliter;
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

	//消息响应函数
	afx_msg LRESULT OnMyChange(WPARAM wParam, LPARAM lParam);
};


