#pragma once


// CModifyPwdDlg 对话框

class CModifyPwdDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CModifyPwdDlg)

public:
	CModifyPwdDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CModifyPwdDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = DIALOG_MODIFYPWD };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CString new_pwd;
	CString repeat_pwd;
	bool state = true;
};
