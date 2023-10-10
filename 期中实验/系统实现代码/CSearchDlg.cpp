// CSearchDlg.cpp: 实现文件
//

#include "pch.h"
#include "BookSystem.h"
#include "CSearchDlg.h"
#include "CLoginDlg.h"
#include "CModifyItemDlg.h"
#include "CSearchTag.h"

// CSearchDlg

IMPLEMENT_DYNCREATE(CSearchDlg, CFormView)

CSearchDlg::CSearchDlg()
	: CFormView(DIALOG_BOOKSERACH)
	, SearchText(_T(""))
{

}

CSearchDlg::~CSearchDlg()
{
}

void CSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, list);
	DDX_Text(pDX, IDC_EDIT1, SearchText);
}

BEGIN_MESSAGE_MAP(CSearchDlg, CFormView)
	ON_BN_CLICKED(IDC_BUTTON1, &CSearchDlg::OnBnClickedButton1)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CSearchDlg::OnNMDblclkList1)
	ON_BN_CLICKED(IDC_BUTTON2, &CSearchDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CSearchDlg 诊断

#ifdef _DEBUG
void CSearchDlg::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CSearchDlg::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CSearchDlg 消息处理程序


void CSearchDlg::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 在此添加专用代码和/或调用基类

	CString str[] = { _T("ISBN"),_T("书籍名称"),
		_T("作者"),_T("出版社"),_T("书籍编号"),_T("零售价"),_T("库存量") };
	for (int i = 0; i < 7; ++i)
		list.InsertColumn(i, str[i], LVCFMT_CENTER, 100);

	//设定显示格式：可选中整行、带网格的报表
	list.SetExtendedStyle(list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	Update();
}

//更新到初始化列表
void CSearchDlg::Update()
{
	//默认显示所有图书
	char query[120] = "select * from book natural join sale order by bookid asc";
	auto res = PQexec(CLoginDlg::conn, query);
	int rows = PQntuples(res), cols = PQnfields(res);
	for (int i = 0; i < rows; ++i)
	{
		//插入一行
		list.InsertItem(i, CString(PQgetvalue(res, i, 0)));
		//插入该行后续列
		for (int j = 1; j < cols; ++j)
			list.SetItemText(i, j, CString(PQgetvalue(res, i, j)));
	}
	PQclear(res);
}
void CSearchDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	//将输入内容同步到变量
	UpdateData(true);
	//清空上次查询的内容
	list.DeleteAllItems();
	//判断查找字符串长度，如果为空，则恢复到初始状态(显示所有数据)
	if (SearchText.IsEmpty())
	{
		Update();
		MessageBox(_T("请输入查询内容!"));
		return;
	}
	//查找中，ISBN和编号必须输全,而书名、作者和出版社可以输入部分来匹配
	//注意，由于bookID是serial(int)类型，应与这些字符串类型的匹配分开，否则，
	//输入非数时整条语句就无法执行。

	//1、查找非ID
	CString temp_query = _T("select * from book natural join sale where ISBN ='")
		+ SearchText + _T("' or title like '%") + SearchText + _T("%' or author like '%")
		+ SearchText + _T("%' or press like '%") + SearchText + _T("%';");
	CStringA query;
	query = temp_query;
	auto res = PQexec(CLoginDlg::conn, query.GetBuffer());
	int rows = PQntuples(res), cols = PQnfields(res);
	for (int i = 0; i < rows; ++i)
	{
		//插入一行
		list.InsertItem(i, CString(PQgetvalue(res, i, 0)));
		//插入该行后续列
		for (int j = 1; j < cols; ++j)
			list.SetItemText(i, j, CString(PQgetvalue(res, i, j)));
	}
	PQclear(res);

	//2、查找ID，当且仅当上面查找到0条时
	if (rows == 0)
	{
		temp_query = _T("select * from book natural join sale where bookID=")
			+ SearchText + _T(";");
		query = temp_query;
		res = PQexec(CLoginDlg::conn, query.GetBuffer());

		rows = PQntuples(res), cols = PQnfields(res);
		for (int i = 0; i < rows; ++i)
		{
			//插入一行
			list.InsertItem(i, CString(PQgetvalue(res, i, 0)));
			//插入该行后续列
			for (int j = 1; j < cols; ++j)
				list.SetItemText(i, j, CString(PQgetvalue(res, i, j)));
		}
		PQclear(res);
	}
	//通知查找到的数据条数
	CString str;
	str.Format(_T("共查找到%d条相关数据"), rows);
	if (rows > 0)
		MessageBox(str);
	else
		MessageBox(_T("未查找到相关数据!"));
}

//双击列表直接修改
void CSearchDlg::OnNMDblclkList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	//获取双击处的行、列
	int row = pNMItemActivate->iItem, col = pNMItemActivate->iSubItem;
	if (row == -1)			//越界处理
	{
		MessageBox(_T("请选择表格内的区域!"));
		return;
	}
	if (col == 0)
	{
		MessageBox(_T("不能修改ISBN"));
		return;
	}
	if (col == 4)
	{
		MessageBox(_T("不能修改书籍编号(ID)"));
		return;
	}
	//获取当前行的书籍ID和ISBN
	CString cur_ID = list.GetItemText(row, 4);
	CString cur_ISBN = list.GetItemText(row, 0);

	//弹出修改窗口，阻塞进程，直到该窗口关闭
	//转到CModifyItemDlg的执行进程
	CModifyItemDlg dlg;
	dlg.DoModal();

	//判断状态
	if (dlg.state == false)
		return;

	//修改到数据库中
	CString temp_query;
	switch (col)			//根据列号决定修改哪项属性
	{
	case 1:
		temp_query = _T("update book set title='") + dlg.newValue + _T("' where ISBN='")
			+ cur_ISBN + _T("';");
		break;
	case 2:
		temp_query = _T("update book set author='") + dlg.newValue + _T("' where ISBN='")
			+ cur_ISBN + _T("';");
		break;
	case 3:
		temp_query = _T("update book set press='") + dlg.newValue + _T("' where ISBN='")
			+ cur_ISBN + _T("';");
		break;
	case 5:
		temp_query = _T("update sale set price=") + dlg.newValue + _T(" where bookid=")
			+ cur_ID + _T(";");
		break;
	case 6:
		temp_query = _T("update sale set inventory=") + dlg.newValue + _T(" where bookid=")
			+ cur_ID + _T(";");
		break;
	}

	CStringA query(temp_query);
	auto res = PQexec(CLoginDlg::conn, query.GetBuffer());
	if (PQresultStatus(res) == PGRES_COMMAND_OK)
	{
		//设置列表显示
		list.SetItemText(row, col, dlg.newValue);
		MessageBox(_T("修改成功!"));
	}
	else		//可能出现的情况：修改了单价或库存触发了check语句
		MessageBox(_T("输入信息不合理,修改失败!"));
	PQclear(res);


	*pResult = 0;
}

//详细查找书籍
void CSearchDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码

	//弹出窗口
	CSearchTag dlg;
	dlg.DoModal();

	if (dlg.state == false)
		return;

	//清空内容
	list.DeleteAllItems();

	//根据获取的信息查找
	//定义查找语句(保证了不全为空)
	CString temp_query = _T("select * from book natural join sale where ");
	CString tagQuery[5];
	tagQuery[0] = dlg.tag_ISBN.IsEmpty() ? _T("") : (_T("ISBN like '%") + dlg.tag_ISBN + _T("%' "));
	tagQuery[1] = dlg.tag_ID.IsEmpty() ? _T("") : (_T("ID=") + dlg.tag_ID + _T(" "));
	tagQuery[2] = dlg.tag_title.IsEmpty() ? _T("") : (_T("title like '%") + dlg.tag_title + _T("%' "));
	tagQuery[3] = dlg.tag_author.IsEmpty() ? _T("") : (_T("author like '%") + dlg.tag_author + _T("%' "));
	tagQuery[4] = dlg.tag_press.IsEmpty() ? _T("") : (_T("press like '%") + dlg.tag_press + _T("%' "));
	//整理查找语句
	int j = 0;
	for (int i = 0; i < 5; ++i)
	{
		if (!tagQuery[i].IsEmpty())
			tagQuery[j++] = tagQuery[i];
	}
	for (int i = 0; i < j; ++i)
	{
		if (i != j - 1)			//不是最后一条查询
			tagQuery[i] += _T("and ");
		temp_query += tagQuery[i];
	}

	CStringA query(temp_query);
	auto res = PQexec(CLoginDlg::conn, query.GetBuffer());
	int rows = PQntuples(res), cols = PQnfields(res);

	if (rows > 0)
	{
		CString str;
		str.Format(_T("共查找到%d条相关数据"), rows);
		//显示内容
		for (int i = 0; i < rows; ++i)
		{
			//插入一行
			list.InsertItem(i, CString(PQgetvalue(res, i, 0)));
			//插入该行后续列
			for (int j = 1; j < cols; ++j)
				list.SetItemText(i, j, CString(PQgetvalue(res, i, j)));
		}
		MessageBox(str);
	}
	else
		MessageBox(_T("未查找到相关数据!"));
	PQclear(res);
}
