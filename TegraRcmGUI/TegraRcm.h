#pragma once
#include <string>
#include <windows.h>
#include <time.h>
#include <iostream>
#include <sstream>
#pragma once
#include <fstream>
#include <iostream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include "resource.h"
#include "TegraRcmSmash.h"
#include <setupapi.h>
#include <stdio.h>
#include <Strsafe.h>
#include "afxcmn.h"
#pragma comment (lib, "setupapi.lib")

class TegraRcm
{
public:
	TegraRcm(CDialog* pParent = NULL);
	~TegraRcm();
public:
	void InitCtrltbDlgs(CDialog* pCtrltb1, CDialog* pCtrltb2, CDialog* pCtrltb3);
	int GetRcmStatus();	
	ULONGLONG GetDllVersion(LPCTSTR lpszDllName);
	TCHAR* GetAbsolutePath(TCHAR* relative_path, DWORD dwFlags);
	string GetPreset(string param);
	void InstallDriver();
	BOOL LookForAPXDevice();
	void AppendLog(string message);
	void SendUserMessage(string message, int type = NEUTRAL);	
	void SetPreset(string param, string value);
	void GetFavorites();
	void AddFavorite(CString value);
	void SaveFavorites();
	void BitmapDisplay(int IMG);
	void LookUp();
	int Smasher(TCHAR args[]);
	char* GetRelativeFilename(char *currentDirectory, char *absoluteFilename);

	BOOL CmdShow = TRUE;
	// Notify Icon
	NOTIFYICONDATA m_NID;
	CPoint m_ptMouseHoverEvent;

	BOOL CreateTrayIcon();
	BOOL SetTrayIconTipText(LPCTSTR pszText);
	BOOL ShowTrayIconBalloon(LPCTSTR pszTitle, LPCTSTR pszText, UINT unTimeout, DWORD dwInfoFlags);
	BOOL SetTrayIcon(HICON hIcon);
	BOOL SetTrayIcon(WORD wIconID);
	void ShowContextMenu(HWND hWnd);
	BOOL DestroyTrayIcon();
	LRESULT OnTrayIconEvent(UINT wParam, LPARAM lParam);

	BOOL PAUSE_LKP_DEVICE = FALSE;
	BOOL AUTOINJECT_CURR = FALSE;
	BOOL DELAY_AUTOINJECT = FALSE;
	BOOL WAITING_RECONNECT = FALSE;
	BOOL ASK_FOR_DRIVER = FALSE;
	BOOL MIN_TO_TRAY_CURR = FALSE;

	CString csPath;
	COLORREF LabelColor = RGB(0, 0, 0);
	COLORREF WhiteRGB = RGB(255, 255, 255);
	COLORREF BlackRGB = RGB(0, 0, 0);
	COLORREF RedRGB = RGB(255, 0, 0);
	COLORREF GreenRGB = RGB(0, 100, 0);

	TCHAR* PAYLOAD_FILE;
	CArray <CString, CString> Favorites;
	CDialog* m_Parent;
	CDialog* m_Ctrltb1;
	CDialog* m_Ctrltb2;
	CDialog* m_Ctrltb3;
private:
	HWND m_hWnd;
	TegraRcmSmash m_Device;
	int m_RC = -99;;
	BOOL FIRST_LOOKUP = TRUE;

};

