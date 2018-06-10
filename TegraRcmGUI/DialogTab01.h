#pragma once
#include <string>
#include "TegraRcmGUIDlg.h"
#include "TegraRcm.h"

// DialogTab01 dialog

class DialogTab01 : 
	public CDialog
{
	DECLARE_DYNAMIC(DialogTab01)
public:
	DialogTab01(TegraRcm *pTegraRcm, CWnd* Parent = NULL);   // standard constructor
	virtual ~DialogTab01();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = ID_DIALOGTAB_01 };
#endif
	TCHAR* PAYLOAD_FILE;
	BOOL PREVENT_AUTOINJECT = TRUE;
	BOOL DELAY_AUTOINJECT = FALSE;
	CArray <CString, CString> m_ListBox;
	int m_SelectedItem = -1;
	CFont m_BoldFont;
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	
	afx_msg void OnEnChangePath();
	
private:
	TegraRcm *m_TegraRcm;
public:
	afx_msg void InjectPayload();
	afx_msg void OnBnClickedBrowse();
	afx_msg void OnBnClickedAddFav();
	afx_msg void OnBnClickedDelFav();
	afx_msg void OnDblclkList1();
	afx_msg void OnLbnSelchangeList1();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
