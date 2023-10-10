#pragma once



// CPayDlg 窗体视图

class CPayDlg : public CFormView
{
	DECLARE_DYNCREATE(CPayDlg)

protected:
	CPayDlg();           // 动态创建所使用的受保护的构造函数
	virtual ~CPayDlg();

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = DIALOG_PAY };
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
	CListCtrl pay_list;
public:
	virtual void OnInitialUpdate();
	afx_msg void OnNMDblclkList1(NMHDR* pNMHDR, LRESULT* pResult);
	CString pay_info;
	afx_msg void OnBnClickedButton1();
	void Update();
};


