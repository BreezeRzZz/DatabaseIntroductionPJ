#pragma once


// CBuyConfirmDlg 对话框

class CBuyConfirmDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CBuyConfirmDlg)

public:
	CBuyConfirmDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CBuyConfirmDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = DIALOG_BUYCONFIRM };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
public:
	int buy_num = 0;
	bool state = true;
	virtual BOOL OnInitDialog();
};
