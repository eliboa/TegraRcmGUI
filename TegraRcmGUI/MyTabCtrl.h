#pragma once
#include "afxcmn.h"
#include "TegraRcm.h"

class MyTabCtrl :
	public CTabCtrl
{
public:
	MyTabCtrl();
	~MyTabCtrl();

	//Array to hold the list of dialog boxes/tab pages for CTabCtrl
	int m_DialogID[3];
	//CDialog Array Variable to hold the dialogs
	CDialog *m_Dialog[3];
	int m_nPageCount;
	//Function to Create the dialog boxes during startup
	void InitDialogs(TegraRcm *parent);

	//Function to activate the tab dialog boxes
	void ActivateTabDialogs();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnTcnSelchange(NMHDR *pNMHDR, LRESULT *pResult);
	TegraRcm *m_TegraRcm;
};
