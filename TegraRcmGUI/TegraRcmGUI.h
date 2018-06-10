
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

extern CTegraRcmGUIApp theApp;