#pragma once


// CPayConfirmDlg 对话框

class CPayConfirmDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPayConfirmDlg)

public:
	CPayConfirmDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CPayConfirmDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = DIALOG_PAYCONFIRM };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	enum state { PayConfirm, RefundConfirm, Cancel };
	state s = Cancel;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
};
