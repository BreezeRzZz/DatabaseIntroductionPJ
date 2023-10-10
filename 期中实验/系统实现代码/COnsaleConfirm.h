#pragma once


// COnsaleConfirm 对话框

class COnsaleConfirm : public CDialogEx
{
	DECLARE_DYNAMIC(COnsaleConfirm)

public:
	COnsaleConfirm(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~COnsaleConfirm();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = DIALOG_ONSALECONFIRM };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	bool state = true;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	double onsale_price;
	int onsale_num;
};
