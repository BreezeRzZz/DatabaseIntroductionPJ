#pragma once



// CBuyDlg 窗体视图

class CBuyDlg : public CFormView
{
	DECLARE_DYNCREATE(CBuyDlg)

protected:
	CBuyDlg();           // 动态创建所使用的受保护的构造函数
	virtual ~CBuyDlg();

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = DIALOG_BUYBOOK };
#endif
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
private:
	CListCtrl buy_list;
public:
	afx_msg void OnNMDblclkList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedButton1();
	CString buy_info;
	void Update();
};


