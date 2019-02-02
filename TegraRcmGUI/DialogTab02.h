#pragma once
#include <string>
#include "TegraRcmGUIDlg.h"
#include "TegraRcm.h"




// DialogTab02 dialog

class DialogTab02 : 
	public CDialogEx
{
	DECLARE_DYNAMIC(DialogTab02)
public:
	DialogTab02(TegraRcm *pTegraRcm, CWnd* pParent = NULL);   // standard constructor
	virtual ~DialogTab02();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = ID_DIALOGTAB_02 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CFont m_BoldFont;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedMountSd();
	afx_msg void OnBnClickedShofel2();
private:
	TegraRcm *m_TegraRcm;
public:
	virtual BOOL OnInitDialog();
//	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedDumpBiskey();
};
