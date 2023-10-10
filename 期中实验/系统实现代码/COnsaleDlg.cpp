// COnsaleDlg.cpp: 实现文件
//

#include "pch.h"
#include "BookSystem.h"
#include "COnsaleDlg.h"
#include "CLoginDlg.h"
#include "COnsaleConfirm.h"


// COnsaleDlg

IMPLEMENT_DYNCREATE(COnsaleDlg, CFormView)

COnsaleDlg::COnsaleDlg()
	: CFormView(DIALOG_ONSALE)
	, sale_info(_T(""))
{

}

COnsaleDlg::~COnsaleDlg()
{
}

void COnsaleDlg::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, onsale_list);
	DDX_Text(pDX, IDC_EDIT1, sale_info);
}

BEGIN_MESSAGE_MAP(COnsaleDlg, CFormView)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &COnsaleDlg::OnNMDblclkList1)
	ON_BN_CLICKED(IDC_BUTTON1, &COnsaleDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// COnsaleDlg 诊断

#ifdef _DEBUG
void COnsaleDlg::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void COnsaleDlg::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// COnsaleDlg 消息处理程序


void COnsaleDlg::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 在此添加专用代码和/或调用基类


	CString str[] = { _T("ISBN"),_T("书籍名称"),
		_T("作者"),_T("出版社"),_T("进货价"),_T("进货量"),_T("余量"),
		_T("订单编号"),_T("付款状态"),_T("是否到货") };
	for (int i = 0; i < 10; ++i)
		onsale_list.InsertColumn(i, str[i], LVCFMT_CENTER, 100);

	//设定显示格式：可选中整行、带网格的报表
	onsale_list.SetExtendedStyle(onsale_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	//显示已到货的进货列表(按orderID升序)
	char query[120] = "select * from book natural join restocklist where arrival='已到货' order by orderID asc";
	auto res = PQexec(CLoginDlg::conn, query);
	int rows = PQntuples(res), cols = PQnfields(res);
	for (int i = 0; i < rows; ++i)
	{
		//插入一行
		onsale_list.InsertItem(i, CString(PQgetvalue(res, i, 0)));
		//插入该行后续列
		for (int j = 1; j < cols; ++j)
			onsale_list.SetItemText(i, j, CString(PQgetvalue(res, i, j)));
	}
	PQclear(res);
}

//双击上架
void COnsaleDlg::OnNMDblclkList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	//获取双击处的行
	int row = pNMItemActivate->iItem;
	if (row == -1)			//越界处理
	{
		MessageBox(_T("请选择表格内的区域!"));
		return;
	}

	//确认余量
	if (onsale_list.GetItemText(row, 6) == _T("0"))
	{
		MessageBox(_T("余量不足，请重试"));
		return;
	}

	//弹出确认窗口
	COnsaleConfirm dlg;
	dlg.DoModal();

	if (dlg.state == false)
		return;

	//确认余量
	if (_ttoi(onsale_list.GetItemText(row, 6)) - dlg.onsale_num < 0)
	{
		MessageBox(_T("余量不足，请重试"));
		return;
	}

	//将进货添加到book表
	CString orderID = onsale_list.GetItemText(row, 7);

	//确定零售价格(对于新书)
	CString price;
	price.Format(_T("%.2lf"), dlg.onsale_price);
	//确定上架数量
	CString num;
	num.Format(_T("%d"), dlg.onsale_num);



	//先获取对应ISBN的bookID和售价
	CString temp_query = _T("select bookID,price from sale where ISBN='")
		+ onsale_list.GetItemText(row, 0)
		+ _T("';");
	CStringA query(temp_query);
	auto res = PQexec(CLoginDlg::conn, query.GetBuffer());

	//检测是否有售价相同的书，获取相应的bookID
	CString duplicate_ID;
	for (int i = 0; i < PQntuples(res); ++i)
	{
		if (abs(atof(PQgetvalue(res, i, 1)) - dlg.onsale_price) < 0.001)
			duplicate_ID = CString(PQgetvalue(res, i, 0));
	}

	PQclear(res);
	//在sale表中新插入1条：
	//以下有2种情况：
	//1、ISBN不在sale表中，说明是新书(不需要向book表插入，因为在前面进货时就已经添加过了)
	//2、ISBN在sale表中，但给定的售价不同
	if (duplicate_ID.IsEmpty())
	{
		temp_query = _T("insert into sale(ISBN,price,inventory) values('")
			+ onsale_list.GetItemText(row, 0)					//ISBN
			+ _T("',") + price									//price
			+ _T(",") + num										//num	
			+ _T(");");
	}

	//否则说明，sale表中存在与之售价相同且ISBN相同的书
	//将这本书的存货与之相加
	else
	{
		temp_query = _T("update sale set inventory = inventory+")
			+ num + _T(" where bookID=")
			+ duplicate_ID + _T(";");
	}

	//更新restocklist的存量
	temp_query += _T("update restocklist set remain=remain-")
		+ num + _T("where orderID =")
		+ orderID + _T(";");
	query = temp_query;
	res = PQexec(CLoginDlg::conn, query.GetBuffer());


	if (PQresultStatus(res) == PGRES_COMMAND_OK)
	{
		//更新显示
		CString cur_remain;
		cur_remain.Format(_T("%d"), _ttoi(onsale_list.GetItemText(row, 6)) - dlg.onsale_num);
		onsale_list.SetItemText(row, 6, cur_remain);
		MessageBox(_T("上架成功!"));
	}
	else
		MessageBox(_T("上架失败!"));

	*pResult = 0;
}


void COnsaleDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码

	//将输入内容同步到变量
	UpdateData(true);
	//清空上次查询的内容
	onsale_list.DeleteAllItems();
	//判断查找字符串长度，如果为空，则恢复到初始状态(显示所有数据)
	if (sale_info.IsEmpty())
	{
		Update();
		MessageBox(_T("请输入查询内容!"));
		return;
	}

	CString temp_query = _T("select * from book natural join restocklist where arrival='已到货' and (ISBN='")
		+ sale_info + _T("' or title like '%") + sale_info + _T("%' or author like '%")
		+ sale_info + _T("%' or press like '%") + sale_info + _T("%');");
	CStringA query;
	query = temp_query;
	auto res = PQexec(CLoginDlg::conn, query.GetBuffer());
	int rows = PQntuples(res), cols = PQnfields(res);
	for (int i = 0; i < rows; ++i)
	{
		//插入一行
		onsale_list.InsertItem(i, CString(PQgetvalue(res, i, 0)));
		//插入该行后续列
		for (int j = 1; j < cols; ++j)
			onsale_list.SetItemText(i, j, CString(PQgetvalue(res, i, j)));
	}
	PQclear(res);

	//查找ID，当且仅当上面查找到0条时
	if (rows == 0)
	{
		temp_query = _T("select * from book natural join restocklist where arrival='已到货' and orderID=")
			+ sale_info + _T(";");
		query = temp_query;
		res = PQexec(CLoginDlg::conn, query.GetBuffer());

		rows = PQntuples(res), cols = PQnfields(res);
		for (int i = 0; i < rows; ++i)
		{
			//插入一行
			onsale_list.InsertItem(i, CString(PQgetvalue(res, i, 0)));
			//插入该行后续列
			for (int j = 1; j < cols; ++j)
				onsale_list.SetItemText(i, j, CString(PQgetvalue(res, i, j)));
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
void COnsaleDlg::Update()
{
	//默认显示所有订单
	char query[200] = "select * from book natural join restocklist where arrival='已到货' order by orderID asc";
	auto res = PQexec(CLoginDlg::conn, query);
	int rows = PQntuples(res), cols = PQnfields(res);
	for (int i = 0; i < rows; ++i)
	{
		//插入一行
		onsale_list.InsertItem(i, CString(PQgetvalue(res, i, 0)));
		//插入该行后续列
		for (int j = 1; j < cols; ++j)
			onsale_list.SetItemText(i, j, CString(PQgetvalue(res, i, j)));
	}
	PQclear(res);
}