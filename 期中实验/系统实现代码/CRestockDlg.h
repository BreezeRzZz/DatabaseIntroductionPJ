#pragma once


// CRestockDlg 窗体视图

class CRestockDlg : public CFormView
{
	DECLARE_DYNCREATE(CRestockDlg)

protected:
	CRestockDlg();           // 动态创建所使用的受保护的构造函数
	virtual ~CRestockDlg();

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = DIALOG_RESTOCK };
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
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	bool checkISBN(CString& ISBN);
private:
	CComboBox restock_cbx;
	CString restock_ISBN;
	CString restock_title;
	CString restock_author;
	CString restock_press;
	CString restock_price;
	CString restock_num;
};


