#pragma once


// CCreateUserDlg 对话框

class CCreateUserDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCreateUserDlg)

public:
	CCreateUserDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CCreateUserDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = DIALOG_CREATEUSER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CString create_username;
	CString create_pwd;
	bool ifOK = false;
};
