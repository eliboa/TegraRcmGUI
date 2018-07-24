/*
DialogTab03.cpp

MIT License

Copyright(c) 2018 eliboa

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
//

#include "stdafx.h"
#include "TegraRcmGUI.h"
#include "DialogTab03.h"
#include "afxdialogex.h"


// DialogTab03 dialog

IMPLEMENT_DYNAMIC(DialogTab03, CDialogEx)

DialogTab03::DialogTab03(TegraRcm *pTegraRcm, CWnd* Parent /*=NULL*/)
	: CDialogEx(ID_DIALOGTAB_03, Parent)
{
	m_TegraRcm = pTegraRcm;
}

DialogTab03::~DialogTab03()
{
}

void DialogTab03::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BOOL DialogTab03::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	string value = m_TegraRcm->GetPreset("AUTO_INJECT");
	if (value == "TRUE")
	{
		m_TegraRcm->AUTOINJECT_CURR = TRUE;
		CMFCButton*checkbox = (CMFCButton*)GetDlgItem(AUTO_INJECT);
		checkbox->SetCheck(BST_CHECKED);
	}

	value = m_TegraRcm->GetPreset("MIN_TO_TRAY");
	if (value == "TRUE")
	{
		m_TegraRcm->MIN_TO_TRAY_CURR = TRUE;
		CMFCButton*checkbox = (CMFCButton*)GetDlgItem(MIN_TO_TRAY);
		checkbox->SetCheck(BST_CHECKED);
	}

	HKEY hKey;
	const std::string key = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
	const std::string subkey = "TegraRcmGUI";

	// Open Run Registry location 
	LONG lnRes = RegOpenKeyExA(HKEY_CURRENT_USER,
		key.c_str(), 0, KEY_READ, &hKey);

	if (ERROR_SUCCESS == lnRes)
	{
		lnRes = RegQueryValueExA(hKey, subkey.c_str(), NULL, NULL, NULL, NULL);
		if (lnRes != ERROR_FILE_NOT_FOUND)
		{
			CMFCButton*checkbox = (CMFCButton*)GetDlgItem(RUN_WINSTART);
			checkbox->SetCheck(BST_CHECKED);
		}
	}


	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(DialogTab03, CDialogEx)
	ON_BN_CLICKED(AUTO_INJECT, &DialogTab03::OnClickedAutoInject)
	ON_BN_CLICKED(MIN_TO_TRAY, &DialogTab03::OnClickedMinToTray)
	ON_BN_CLICKED(ID_INSTALL_DRIVER, &DialogTab03::OnBnClickedInstallDriver)
	ON_BN_CLICKED(RUN_WINSTART, &DialogTab03::OnBnClickedWinstart)
END_MESSAGE_MAP()


// DialogTab03 message handlers


void DialogTab03::OnClickedAutoInject()
{
	// Get Auto inject checkbox value (checked, unchecked)
	CButton *m_ctlCheck = (CButton*)GetDlgItem(AUTO_INJECT);
	BOOL IsCheckChecked = (m_ctlCheck->GetCheck() == 1) ? true : false;

	if (m_TegraRcm->AUTOINJECT_CURR != IsCheckChecked)
	{
		// Auto inject option enabled
		if (IsCheckChecked)
		{
			m_TegraRcm->SetPreset("AUTO_INJECT", "TRUE");
			m_TegraRcm->DELAY_AUTOINJECT = TRUE;
		}
		// Auto inject option disabled
		else
		{
			m_TegraRcm->SetPreset("AUTO_INJECT", "FALSE");
			m_TegraRcm->DELAY_AUTOINJECT = FALSE;
		}
		// Save current checkbox value
		m_TegraRcm->AUTOINJECT_CURR = IsCheckChecked;
	}
}


void DialogTab03::OnClickedMinToTray()
{
	// Get Minimize to tray checkbox value (checked, unchecked)
	CButton *m_ctlCheck = (CButton*)GetDlgItem(MIN_TO_TRAY);
	BOOL IsCheckChecked = (m_ctlCheck->GetCheck() == 1) ? true : false;
	if (m_TegraRcm->MIN_TO_TRAY_CURR != IsCheckChecked)
	{
		if (IsCheckChecked) m_TegraRcm->SetPreset("MIN_TO_TRAY", "TRUE");
		else m_TegraRcm->SetPreset("MIN_TO_TRAY", "FALSE");
		m_TegraRcm->MIN_TO_TRAY_CURR = IsCheckChecked;
	}
}


void DialogTab03::OnBnClickedInstallDriver()
{
	m_TegraRcm->InstallDriver();
}



void DialogTab03::OnBnClickedWinstart()
{
	// Init
	HKEY hKey;
	const std::string key = "TegraRcmGUI";
	std::vector<char> buffer;

	// Get checkbox value (checked, unchecked)
	CButton *m_ctlCheck = (CButton*)GetDlgItem(RUN_WINSTART);
	BOOL IsCheckChecked = (m_ctlCheck->GetCheck() == 1) ? true : false;

	// Get application absolute path
	TCHAR szPath[_MAX_PATH];
	VERIFY(::GetModuleFileName(AfxGetApp()->m_hInstance, szPath, _MAX_PATH));
	// Convert path to ANSI string
	int size = WideCharToMultiByte(CP_UTF8, 0, szPath, -1, NULL, 0, NULL, NULL);
	if (size > 0) {
		buffer.resize(size);
		WideCharToMultiByte(CP_UTF8, 0, szPath, -1, (LPSTR)(&buffer[0]), buffer.size(), NULL, NULL);
	}
	std::string appPath(&buffer[0]);
	std::string keyValue;
	keyValue.append("\"");
	keyValue.append(appPath);
	keyValue.append("\" /autostart");
	
	// Open Run Registry location 
	LONG lnRes = RegOpenKeyEx(HKEY_CURRENT_USER,
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"),
		0L, KEY_WRITE,
		&hKey);

	if (ERROR_SUCCESS == lnRes)
	{
		if (IsCheckChecked)
		{
			// Set full application path with a keyname to registry
			lnRes = RegSetValueExA(hKey,
				key.c_str(),
				0,
				REG_SZ,
				(LPBYTE)(keyValue.c_str()),
				keyValue.size() + 1);
		}
		else
		{
			lnRes = RegDeleteValueA(hKey, key.c_str());
		}
		if (ERROR_SUCCESS != lnRes)
		{
			AfxMessageBox(_T("Failed to set/unset at startup"));
		}
	}
	else
	{
		AfxMessageBox(_T("Failed to access registry"));
	}
}
