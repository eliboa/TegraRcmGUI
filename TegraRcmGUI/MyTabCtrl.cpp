#include "stdafx.h"
#include "MyTabCtrl.h"
#include "DialogTab01.h"
#include "DialogTab02.h"
#include "DialogTab03.h"
#include "resource.h"
using namespace std;


MyTabCtrl::MyTabCtrl()
{

	m_DialogID[0] = ID_DIALOGTAB_01;
	m_DialogID[1] = ID_DIALOGTAB_02;
	m_DialogID[2] = ID_DIALOGTAB_03;

	m_nPageCount = 3;
}


MyTabCtrl::~MyTabCtrl()
{
}

BEGIN_MESSAGE_MAP(MyTabCtrl, CTabCtrl)
	ON_NOTIFY_REFLECT(TCN_SELCHANGE, &MyTabCtrl::OnTcnSelchange)
END_MESSAGE_MAP()

//This function creates the Dialog boxes once
void MyTabCtrl::InitDialogs(TegraRcm *parent)
{
	m_TegraRcm = parent;
	m_Dialog[0] = new DialogTab01(m_TegraRcm);
	m_Dialog[1] = new DialogTab02(m_TegraRcm);
	m_Dialog[2] = new DialogTab03(m_TegraRcm);
	m_Dialog[0]->Create(m_DialogID[0], GetParent());
	m_Dialog[1]->Create(m_DialogID[1], GetParent());
	m_Dialog[2]->Create(m_DialogID[2], GetParent());
}

void MyTabCtrl::OnTcnSelchange(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	ActivateTabDialogs();
	*pResult = 0;
}

void MyTabCtrl::ActivateTabDialogs()
{
	//ASSERT(MyParentDlg);
	//.OnPaint();

	int nSel = GetCurSel();
	if (m_Dialog[nSel]->m_hWnd)
		m_Dialog[nSel]->ShowWindow(SW_HIDE);

	CRect l_rectClient;
	CRect l_rectWnd;

	GetClientRect(l_rectClient);
	AdjustRect(FALSE, l_rectClient);
	GetWindowRect(l_rectWnd);
	GetParent()->ScreenToClient(l_rectWnd);
	l_rectClient.OffsetRect(l_rectWnd.left, l_rectWnd.top);
	for (int nCount = 0; nCount < m_nPageCount; nCount++) {
		m_Dialog[nCount]->SetWindowPos(&wndTop, l_rectClient.left, l_rectClient.top, l_rectClient.Width(), l_rectClient.Height(), SWP_HIDEWINDOW);
	}
	m_Dialog[nSel]->SetWindowPos(&wndTop, l_rectClient.left, l_rectClient.top, l_rectClient.Width(), l_rectClient.Height(), SWP_SHOWWINDOW);

	m_Dialog[nSel]->ShowWindow(SW_SHOW);

}