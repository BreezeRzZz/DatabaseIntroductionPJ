#pragma once


// CSearchTag 对话框

class CSearchTag : public CDialogEx
{
	DECLARE_DYNAMIC(CSearchTag)

public:
	CSearchTag(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CSearchTag();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = DIALOG_SRARCHTAG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString tag_ISBN;
	CString tag_ID;
	CString tag_title;
	CString tag_author;
	CString tag_press;
	bool state = true;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
