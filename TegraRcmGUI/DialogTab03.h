#pragma once
#include <string>
#include "TegraRcmGUIDlg.h"
#include "TegraRcm.h"

// DialogTab03 dialog

class DialogTab03 : public CDialogEx
{
	DECLARE_DYNAMIC(DialogTab03)

public:
	DialogTab03(TegraRcm *pTegraRcm, CWnd* Parent = NULL);   // standard constructor
	virtual ~DialogTab03();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = ID_DIALOGTAB_03 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	TegraRcm *m_TegraRcm;
public:
	afx_msg void OnClickedAutoInject();
	afx_msg void OnClickedMinToTray();
	afx_msg void OnBnClickedInstallDriver();
	afx_msg void OnBnClickedWinstart();

};
