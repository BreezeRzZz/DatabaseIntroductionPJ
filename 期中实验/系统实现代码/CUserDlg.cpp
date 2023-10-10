// CUserDlg.cpp: 实现文件
//

#include "pch.h"
#include "BookSystem.h"
#include "CUserDlg.h"
#include "CLoginDlg.h"
#include "CCreateUserDlg.h"
#include "CModifyPwdDlg.h"


// CUserDlg

IMPLEMENT_DYNCREATE(CUserDlg, CFormView)

CUserDlg::CUserDlg()
	: CFormView(DIALOG_USER)
	, username(_T(""))
	, permission(_T(""))
	, pwd(_T(""))
	, realname(_T(""))
	, workid(_T(""))
	, gender(_T(""))
	, age(_T(""))
{

}

CUserDlg::~CUserDlg()
{
}

void CUserDlg::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, username);
	DDX_Text(pDX, IDC_EDIT2, permission);
	DDX_Text(pDX, IDC_EDIT3, pwd);
	DDX_Text(pDX, IDC_EDIT4, realname);
	DDX_Text(pDX, IDC_EDIT5, workid);
	DDX_Text(pDX, IDC_EDIT6, gender);
	DDX_Text(pDX, IDC_EDIT7, age);
	DDX_Control(pDX, IDC_LIST1, user_list);
	DDX_Control(pDX, IDC_BUTTON2, btn_create);
	DDX_Control(pDX, IDC_BUTTON1, modify_btn);
	DDX_Control(pDX, IDC_BUTTON3, modifypwd_btn);
}

BEGIN_MESSAGE_MAP(CUserDlg, CFormView)
	ON_BN_CLICKED(IDC_BUTTON1, &CUserDlg::OnBnClickedButton1)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &CUserDlg::OnNMClickList1)
	ON_BN_CLICKED(IDC_BUTTON2, &CUserDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CUserDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// CUserDlg 诊断

#ifdef _DEBUG
void CUserDlg::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CUserDlg::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CUserDlg 消息处理程序

//初始化显示
void CUserDlg::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 在此添加专用代码和/或调用基类

	//1、从数据库中获取对应的信息(通过与登录信息提供的username比对)
	CString temp_query = _T("select * from administrator where username='")
		+ CLoginDlg::login_username + _T("';");
	CStringA query(temp_query);
	auto res = PQexec(CLoginDlg::conn, query.GetBuffer());
	username = CLoginDlg::login_username;
	pwd = CLoginDlg::login_password;
	permission = CString(PQgetvalue(res, 0, 2));
	realname = CString(PQgetvalue(res, 0, 3));
	workid = CString(PQgetvalue(res, 0, 4));
	gender = CString(PQgetvalue(res, 0, 5));
	age = CString(PQgetvalue(res, 0, 6));

	PQclear(res);

	//2、显示administrator表的用户名
	//对permission为Super的显示全部，否则只显示自己
	user_list.InsertColumn(0, _T("用户名"), LVCFMT_CENTER, 200);

	//设定显示格式：可选中整行、带网格的报表
	user_list.SetExtendedStyle(user_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	//检查权限
	if (permission == _T("超级管理员"))
	{
		char list_query[120] = "select username from administrator order by type";
		res = PQexec(CLoginDlg::conn, list_query);
		int rows = PQntuples(res);
		for (int i = 0; i < rows; ++i)
			user_list.InsertItem(i, CString(PQgetvalue(res, i, 0)));
		PQclear(res);

		//能显示创建用户按钮
		btn_create.ShowWindow(true);
	}
	else
	{
		user_list.InsertItem(0, username);
		//不显示创建用户按钮
		btn_create.ShowWindow(false);
	}

	//更新显示
	UpdateData(false);
}

//修改信息
void CUserDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码

	CString old_username = username;
	//更新同步到变量
	UpdateData(true);

	//更新同步到数据库存储(username,name,jobnumber,gender,age)
	CString temp_query = _T("update administrator set username='")
		+ username
		+ _T("',name='") + realname
		+ _T("',jobnumber=") + workid
		+ _T(",gender='") + gender
		+ _T("',age=") + age + _T(" where username='")
		+ old_username + _T("';");
	CStringA query(temp_query);
	auto res = PQexec(CLoginDlg::conn, query.GetBuffer());

	if (PQresultStatus(res) == PGRES_COMMAND_OK)
	{	
		//修改列表的旧用户名
		for (int i = 0; i < user_list.GetItemCount();++i)
		{
			if (user_list.GetItemText(i, 0) == old_username)
			{
				user_list.SetItemText(i, 0, username);
				MessageBox(_T("修改成功"));
				return;
			}
		}
		//修改登录用户名
		CLoginDlg::login_username = username;
	}
	else
		MessageBox(_T("修改失败,请检查各项内容是否合理!"));
	PQclear(res);
}

//选择用户查看信息
void CUserDlg::OnNMClickList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	//获取双击处的行、列
	int row = pNMItemActivate->iItem;
	if (row == -1)			//越界处理
	{
		MessageBox(_T("请选择表格内的区域!"));
		return;
	}
	//获取用户名
	username = user_list.GetItemText(row, 0);
	//获取其他信息
	CString temp_query = _T("select * from administrator where username='")
		+ username + _T("';");
	CStringA query(temp_query);
	auto res = PQexec(CLoginDlg::conn, query.GetBuffer());

	//更新信息
	//如果选择自己，则显示自己登录的密码
	//否则选择别人时，不显示密码项(对普通管理员来说，没区别，因为他只能看到自己)
	//(由于数据库中只存放了加密后的密码，所以超级管理员查看信息得到的密码也只能是加密后的，
	//且反正看到的密码都是星号，因此这里就设定为查看别人信息的密码为空。)
	if (username == CLoginDlg::login_username)
		pwd = CLoginDlg::login_password;
	else
		pwd = _T("");
	permission = CString(PQgetvalue(res, 0, 2));
	realname = CString(PQgetvalue(res, 0, 3));
	workid = CString(PQgetvalue(res, 0, 4));
	gender = CString(PQgetvalue(res, 0, 5));
	age = CString(PQgetvalue(res, 0, 6));
	PQclear(res);

	//只能修改自己的信息，所以非自己时，隐藏两个修改按钮
	if (username == CLoginDlg::login_username)
	{
		modify_btn.ShowWindow(true);
		modifypwd_btn.ShowWindow(true);
	}
	else
	{
		modify_btn.ShowWindow(false);
		modifypwd_btn.ShowWindow(false);
	}
	//更新显示
	UpdateData(false);

	*pResult = 0;
}

//创建用户按钮
void CUserDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码

	//弹出创建用户窗口
	CCreateUserDlg dlg;
	dlg.DoModal();

	//更新新用户用户名到列表尾部
	if (dlg.ifOK)
		user_list.InsertItem(user_list.GetItemCount(), dlg.create_username);

	return;
}

//修改密码
void CUserDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码

	//弹出修改密码窗口
	CModifyPwdDlg dlg;
	dlg.DoModal();

	if (dlg.state == false)
		return;

	if (dlg.new_pwd == pwd)
	{
		MessageBox(_T("新密码不能与旧密码一致"));
		return;
	}

	//用md5加密算法计算加密后的新密码(需要原始密码和用户名)
	CStringA temp_pwd(dlg.new_pwd);
	CString encrypted_pwd = CString(PQencryptPasswordConn(CLoginDlg::conn, temp_pwd.GetBuffer(),"" , "md5"));
	encrypted_pwd = encrypted_pwd.Right(encrypted_pwd.GetLength() - 3);	//去掉开头的md5这3个字符

	//在数据库中修改对应用户的密码
	CString temp_query = _T("update administrator set password='")
		+ encrypted_pwd + _T("' where username='")
		+ username + _T("';");
	CStringA query(temp_query);
	auto res = PQexec(CLoginDlg::conn, query.GetBuffer());
	if (PQresultStatus(res) == PGRES_COMMAND_OK)
	{
		MessageBox(_T("修改密码成功"));
		//更新密码
		CLoginDlg::login_password = dlg.new_pwd;
		pwd = dlg.new_pwd;
	}
	else
		MessageBox(_T("修改失败!"));

	UpdateData(false);
	PQclear(res);
}
