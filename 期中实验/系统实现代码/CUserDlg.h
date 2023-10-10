#pragma once



// CUserDlg 窗体视图

class CUserDlg : public CFormView
{
	DECLARE_DYNCREATE(CUserDlg)

protected:
	CUserDlg();           // 动态创建所使用的受保护的构造函数
	virtual ~CUserDlg();

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = DIALOG_USER };
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
	CString username;
	CString permission;
	CString pwd;
	CString realname;
	CString workid;
	CString gender;
	CString age;
public:
	virtual void OnInitialUpdate();
private:
	CListCtrl user_list;
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnNMClickList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedButton2();
private:
	CButton btn_create;
public:
	afx_msg void OnBnClickedButton3();
	CButton modify_btn;
	CButton modifypwd_btn;
};


