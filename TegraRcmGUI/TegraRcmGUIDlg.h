// TegraRcmGUIDlg.h : header file
//
#pragma once
#include "res/BitmapPicture.h"
#include "resource.h"
#include <string>
#include "TegraRcmSmash.h"
#include "res/BitmapPicture.h"
#include <windows.h>
#include <string>
#include <time.h>
#include <thread>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <setupapi.h>
#include <stdio.h>
#include <Strsafe.h>
#include "afxcmn.h"
#include "MyTabCtrl.h"
#include "TegraRcm.h"
#include "DialogTab01.h"
#include "DialogTab02.h"
#include "DialogTab03.h"


#pragma comment (lib, "setupapi.lib")

// CTegraRcmGUIDlg dialog
class CTegraRcmGUIDlg :
	public CDialog
{
	// Construction
public:
	CTegraRcmGUIDlg(CWnd* pParent = NULL);	// standard constructor

	CBitmapPicture RCM_BITMAP0;
	CBitmapPicture RCM_BITMAP1;
	CBitmapPicture RCM_BITMAP2;
	CBitmapPicture RCM_BITMAP3;
	CBitmapPicture RCM_BITMAP4;
	CBitmapPicture RCM_BITMAP5;
	CBitmapPicture RCM_BITMAP6;
	HICON StatusIcon;
	CMFCEditBrowseCtrl m_EditBrowse;

	BOOL AUTOINJECT_CURR = FALSE;
	TCHAR* PAYLOAD_FILE;
	int RCM_STATUS = -10;
	BOOL WAITING_RECONNECT = FALSE;
	BOOL PREVENT_AUTOINJECT = TRUE;
	BOOL DELAY_AUTOINJECT = TRUE;
	BOOL ASK_FOR_DRIVER = FALSE;
	BOOL PAUSE_LKP_DEVICE = FALSE;
	BOOL AUTOSTART = FALSE;
	BOOL CmdShow;
	COLORREF LabelColor = RGB(0, 0, 0);
	COLORREF WhiteRGB = RGB(255, 255, 255);
	COLORREF BlackRGB = RGB(0, 0, 0);
	COLORREF RedRGB = RGB(255, 0, 0);
	COLORREF GreenRGB = RGB(0, 100, 0);

	const UINT ID_TIMER_MINUTE = 0x1001;
	const UINT ID_TIMER_SECONDS = 0x1000;

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TEGRARCMGUI_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

														// Implementation

	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnIdle();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnTrayIconEvent(UINT wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

public:
	void StartTimer();
	void StopTimer();
	void OnTimer(UINT nIDEvent);
	int STATUS;
	afx_msg void BitmapDisplay(int IMG);
	afx_msg void OnEnChangePath();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd *pWnd, UINT nCtlColor);
	void ShowWindowCommand();
	void HideWindowCommand();
	void InjectCommand();
	void BrowseCommand();
	void LinuxCommand();
	void MountCommand();
	void InjectFavCommand(int i);
	void InjectFav01Command();
	void InjectFav02Command();
	void InjectFav03Command();
	void InjectFav04Command();
	void InjectFav05Command();
	void InjectFav06Command();
	void InjectFav07Command();
	void InjectFav08Command();
	void InjectFav09Command();
	void InjectFav10Command();
	void AutoInjectCommand();

	CTegraRcmGUIDlg *m_pMainWnd = this;
private:
	MyTabCtrl m_tbCtrl;
	TegraRcm *m_TegraRcm;
};
