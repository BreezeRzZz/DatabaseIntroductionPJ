#pragma once


// CModifyItemDlg 对话框

class CModifyItemDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CModifyItemDlg)

public:
	CModifyItemDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CModifyItemDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = DIALOG_MODIFYITEM };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString newValue;
	bool state = true;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
