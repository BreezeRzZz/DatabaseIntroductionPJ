#pragma once



// COnsaleDlg 窗体视图

class COnsaleDlg : public CFormView
{
	DECLARE_DYNCREATE(COnsaleDlg)

protected:
	COnsaleDlg();           // 动态创建所使用的受保护的构造函数
	virtual ~COnsaleDlg();

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = DIALOG_ONSALE };
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
private:
	CListCtrl onsale_list;
public:
	virtual void OnInitialUpdate();
	afx_msg void OnNMDblclkList1(NMHDR* pNMHDR, LRESULT* pResult);
	CString sale_info;
	afx_msg void OnBnClickedButton1();
	void Update();
};


