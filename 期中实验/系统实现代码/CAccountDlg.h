#pragma once



// CAccountDlg 窗体视图

class CAccountDlg : public CFormView
{
	DECLARE_DYNCREATE(CAccountDlg)

protected:
	CAccountDlg();           // 动态创建所使用的受保护的构造函数
	virtual ~CAccountDlg();

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = DIALOG_ACCOUNT };
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
	CListCtrl account_list;
public:
	virtual void OnInitialUpdate();
private:
	int from_year;
	int from_month;
	int from_day;
	int to_year;
	int to_month;
	int to_day;
public:
	afx_msg void OnBnClickedButton1();
private:
	CComboBox from_hour;
	CComboBox from_min;
	CComboBox from_sec;
	CComboBox to_hour;
	CComboBox to_min;
	CComboBox to_sec;
};


