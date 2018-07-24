
// TegraRcmGUI.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CTegraRcmGUIApp:
// See TegraRcmGUI.cpp for the implementation of this class
//

class CTegraRcmGUIApp : public CWinApp
{
public:
	CTegraRcmGUIApp();

// Overrides
public:
	virtual BOOL InitInstance();
	int Run();

// Implementation

	DECLARE_MESSAGE_MAP()
};

class CCustomCommandLineInfo : public CCommandLineInfo
{
public:
	CCustomCommandLineInfo()
	{
		m_bAutostart = FALSE;
	}
	BOOL m_bAutostart;      
public:
	BOOL IsAutostart() { return m_bAutostart; };

	virtual void ParseParam(LPCTSTR pszParam, BOOL bFlag, BOOL bLast)
	{
		if (0 == wcscmp(pszParam, L"autostart"))
		{
			m_bAutostart = TRUE;
		}
	}
};

extern CTegraRcmGUIApp theApp;