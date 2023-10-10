#pragma once



// CSearchDlg 窗体视图

class CSearchDlg : public CFormView
{
	DECLARE_DYNCREATE(CSearchDlg)

protected:
	CSearchDlg();           // 动态创建所使用的受保护的构造函数
	virtual ~CSearchDlg();

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = DIALOG_BOOKSEARCH };
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
	void Update();
private:
	CListCtrl list;
	CString SearchText;
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnNMDblclkList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedButton2();
};


