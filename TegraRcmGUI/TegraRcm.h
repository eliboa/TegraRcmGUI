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
#include <tlhelp32.h>

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
	void SetLocale();
	void AppendLogBox(CString line);
	void UpdateLogBox();
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
	int Smasher(TCHAR args[], BOOL bInheritHandles = TRUE);
	char* GetRelativeFilename(char *currentDirectory, char *absoluteFilename);
	

	void KillRunningProcess(CString process);
	HWND find_main_window(unsigned long process_id);

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
	BOOL DELAY_AUTOINJECT = TRUE;
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

	struct Label {
		int uid;
		string name;
	};

	Label labels[23] = {
		{ 0, "TegraRcmGUI"},
		{ 1, "No file selected" },
		{ 2, "File doesn't exist" },
		{ 3, "Injecting payload..." },
		{ 4, "Payload injected !" },
		{ 5, "Error while injecting payload" },
		{ 6, "Favorite already exists" },
		{ 7, "Favorite added" },
		{ 8, "Favorite removed" },
		{ 9, "UMS Tool injected" },
		{ 10, "Linux coreboot not found in \\shofel2 dir" },
		{ 11, "Kernel not found in shofel2 directory. Do you want to automatically download arch linux kernel from SoulCipher repo ?" },
		{ 12, "Kernel not found" },
		{ 13, "Loading coreboot. Please wait." },
		{ 14, "Coreboot injected. Waiting 5s for device..." },
		{ 15, "Coreboot loaded " },
		{ 16, "Error" },
		{ 17, "Error while loading imx_usb.exe" },
		{ 18, "APX device driver is missing. Do you want to install it now ?" },
		{ 19, "APX driver not found !" },
		{ 20, "Waiting for user action" },
		{ 21, "Waiting for device in RCM mode" },
		{ 22, "Payload already injected. Are you sure you want to overwrite the stack again ?" }
	};

private:
	HWND m_hWnd;
	TegraRcmSmash m_Device;
	int m_RC = -99;;
	BOOL FIRST_LOOKUP = TRUE;

};

