/*
TegraRcm.cpp

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
#include <stdlib.h>
#include "stdafx.h"
#include "TegraRcm.h"
#include <sstream>
#include <fstream>
#include <codecvt>

using namespace std;

TegraRcm::TegraRcm(CDialog* pParent /*=NULL*/)
{
	m_Parent = pParent;
	m_hWnd = AfxGetMainWnd()->GetSafeHwnd();
	GetFavorites();
	//SendUserMessage("Waiting for device in RCM mode");
}

TegraRcm::~TegraRcm()
{
}

void TegraRcm::InitCtrltbDlgs(CDialog* pCtrltb1, CDialog* pCtrltb2, CDialog* pCtrltb3)
{
	m_Ctrltb1 = pCtrltb1;
	m_Ctrltb2 = pCtrltb2;
	m_Ctrltb3 = pCtrltb3;
}

int TegraRcm::GetRcmStatus()
{
	return m_Device.RcmStatus();
}


//
// Tray icon
//
BOOL TegraRcm::CreateTrayIcon()
{
	memset(&m_NID, 0, sizeof(m_NID));
	ULONGLONG ullVersion = GetDllVersion(_T("Shell32.dll"));
	if (ullVersion >= MAKEDLLVERULL(5, 0, 0, 0))
		m_NID.cbSize = sizeof(NOTIFYICONDATA);
	else m_NID.cbSize = NOTIFYICONDATA_V2_SIZE;

	// set tray icon ID
	m_NID.uID = ID_SYSTEMTRAY;

	// set handle to the window that receives tray icon notifications
	m_NID.hWnd = m_hWnd;

	// set message that will be sent from tray icon to the window
	m_NID.uCallbackMessage = WM_SYSICON;

	// fields that are being set when adding tray icon
	m_NID.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

	// set image
	if (m_RC >= 0) m_NID.hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(ID_TRAYICON_RCM));
	else m_NID.hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(ID_TRAYICON_NO_RCM));

	if (!m_NID.hIcon)
		return FALSE;

	if (!Shell_NotifyIcon(NIM_ADD, &m_NID))
		return FALSE;

	return Shell_NotifyIcon(NIM_SETVERSION, &m_NID);
}
BOOL TegraRcm::DestroyTrayIcon()
{
	return Shell_NotifyIcon(NIM_DELETE, &m_NID);
}
BOOL TegraRcm::SetTrayIconTipText(LPCTSTR pszText)
{
	if (StringCchCopy(m_NID.szTip, sizeof(m_NID.szTip), pszText) != S_OK)
		return FALSE;

	m_NID.uFlags |= NIF_TIP;
	return Shell_NotifyIcon(NIM_MODIFY, &m_NID);
}
BOOL TegraRcm::ShowTrayIconBalloon(LPCTSTR pszTitle, LPCTSTR pszText, UINT unTimeout, DWORD dwInfoFlags)
{
	m_NID.uFlags |= NIF_INFO;
	m_NID.uTimeout = unTimeout;
	m_NID.dwInfoFlags = dwInfoFlags;

	if (StringCchCopy(m_NID.szInfoTitle, sizeof(m_NID.szInfoTitle), pszTitle) != S_OK)
		return FALSE;

	if (StringCchCopy(m_NID.szInfo, sizeof(m_NID.szInfo), pszText) != S_OK)
		return FALSE;

	return Shell_NotifyIcon(NIM_MODIFY, &m_NID);
}
BOOL TegraRcm::SetTrayIcon(HICON hIcon)
{
	m_NID.hIcon = hIcon;
	m_NID.uFlags |= NIF_ICON;

	return Shell_NotifyIcon(NIM_MODIFY, &m_NID);
}
BOOL TegraRcm::SetTrayIcon(WORD wIconID)
{
	HICON hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(wIconID));

	if (!hIcon)
		return FALSE;

	return SetTrayIcon(hIcon);
}
void TegraRcm::ShowContextMenu(HWND hWnd)
{
	POINT pt;
	GetCursorPos(&pt);
	HMENU hMenu = CreatePopupMenu();
	if (hMenu)
	{

		CString autoinjectLb;
		if (AUTOINJECT_CURR) autoinjectLb = _T("Auto inject (On)");
		else autoinjectLb = _T("Auto inject (Off)");
		InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_AUTOINJECT, autoinjectLb);

		if (m_RC == 0)
		{

			HMENU hSubmenu = CreatePopupMenu();
			UINT uID = 0;

			for (int i = 0; i < Favorites.GetCount(); i++)
			{
				if (i < 9)
				{
					uID++;
					int swm;
					switch (i)
					{
						case 0:
							swm = SWM_FAV01;
							break;
						case 1:
							swm = SWM_FAV02;
							break;
						case 2:
							swm = SWM_FAV03;
							break;
						case 3:
							swm = SWM_FAV04;
							break;
						case 4:
							swm = SWM_FAV05;
							break;
						case 5:
							swm = SWM_FAV06;
							break;
						case 6:
							swm = SWM_FAV07;
							break;
						case 7:
							swm = SWM_FAV08;
							break;
						case 8:
							swm = SWM_FAV09;
							break;
						case 9:
							swm = SWM_FAV10;
							break;
						default:
							break;
					}

					int nIndex = Favorites[i].ReverseFind(_T('\\'));
					if (nIndex > 0)
					{

						CString csFilename, csPath, Item;
						csFilename = Favorites[i].Right(Favorites[i].GetLength() - nIndex - 1);
						csPath = Favorites[i].Left(nIndex);
						if (csPath.GetLength() > 30)
						{
							csPath = csPath.Left(30);
							Item = csFilename + _T(" (") + csPath + _T("...)");
						}
						else
						{
							Item = csFilename + _T(" (") + csPath + _T(")");
						}
						InsertMenu(hSubmenu, -1, MF_BYPOSITION, swm, Item);
					}
					else
					{
						InsertMenu(hSubmenu, -1, MF_BYPOSITION, swm, Favorites[i]);
					}
				}
			}
			
			MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
			mii.fMask = MIIM_SUBMENU | MIIM_STRING | MIIM_ID;
			mii.wID = uID;
			mii.hSubMenu = hSubmenu;
			mii.dwTypeData = _T("Favorites");

			CString csPathf, csFilename, payload;

			m_Ctrltb1->GetDlgItem(PAYLOAD_PATH)->GetWindowTextW(csPathf);
			int nIndex = csPathf.ReverseFind(_T('\\'));
			if (nIndex > 0)
			{
				csFilename = csPathf.Right(csPathf.GetLength() - nIndex - 1);
				payload = _T("Inject ") + csFilename;
				InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_INJECT, payload);
			}
			//InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_BROWSE, _T("Browse..."));
			InsertMenuItem(hMenu, -1, TRUE, &mii);
			InsertMenu(hMenu, -1, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
			InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_MOUNT, _T("Mount SD"));
			InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_LINUX, _T("Linux"));			
		}
		InsertMenu(hMenu, -1, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
		if (IsWindowVisible(hWnd))
			InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_HIDE, _T("Hide"));
		else
			InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_SHOW, _T("Show"));		
		InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_EXIT, _T("Exit"));

		// note:	must set window to the foreground or the
		//			menu won't disappear when it should
		SetForegroundWindow(hWnd);

		TrackPopupMenu(hMenu, TPM_BOTTOMALIGN,
			pt.x, pt.y, 0, hWnd, NULL);
		DestroyMenu(hMenu);
	}
}
LRESULT TegraRcm::OnTrayIconEvent(WPARAM wParam, LPARAM lParam)
{
	if ((UINT)wParam != ID_SYSTEMTRAY)
	{
		int t = 1;
	}
	if ((UINT)wParam != ID_SYSTEMTRAY)
		return ERROR_SUCCESS;

	switch ((UINT)lParam)
	{
		case WM_LBUTTONDOWN:
		{
			if (CmdShow) 
			{
				AfxGetMainWnd()->ShowWindow(SW_HIDE);
				CmdShow = FALSE;
			}
			else
			{
				AfxGetMainWnd()->ShowWindow(SW_RESTORE);
				AfxGetMainWnd()->SetForegroundWindow();
				AfxGetMainWnd()->SetFocus();
				AfxGetMainWnd()->SetActiveWindow();
				CmdShow = TRUE;
			}
			break;
		}
		case WM_LBUTTONUP:
		{
			//ShowTrayIconBalloon(TEXT("Baloon message title"), TEXT("Left click!"), 1000, NIIF_INFO);
			break;
		}
		case WM_RBUTTONUP:
		{
			ShowContextMenu(m_Parent->GetSafeHwnd());
			// e.g. show context menu or disable tip and display baloon:
			//SetTrayIconTipText((LPCTSTR)TEXT("Salut"));
			//ShowTrayIconBalloon(TEXT("Baloon message title"), TEXT("Right click!"), 1000, NIIF_INFO);
			break;
		}

	}

	return ERROR_SUCCESS;
}

void TegraRcm::SetLocale()
{
	TCHAR *rfile = GetAbsolutePath(TEXT("locale\\french.txt"), CSIDL_APPDATA);
	std::wifstream wif(rfile);
	wif.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
	if (wif.is_open())
	{
		wif.seekg(0, std::ios::beg);
		std::wstring wline;
		std::wstring stringName;
		std::wstring value;
		std::wstring delimiter = L"=";

		while (wif.good())
		{
			std::getline(wif, wline);
			if (!wif.eof())
			{
				if (wline.find(delimiter) != std::string::npos) {
					stringName = wline.substr(0, wline.find(delimiter));
					value = wline.substr(wline.find(delimiter) + 1, wline.length() + 1);
					CString value2 = value.c_str();
					value2.Replace(_T('#'), '\n');
					int intValue = stoi(stringName.c_str());;
					if (intValue > 0) {
						
						//TCITEM tcItem1;
						//tcItem1.mask = TCIF_TEXT;
						//tcItem1.pszText = _T("Test");
					
						//AfxGetMainWnd()->m_tbCtrl->
						//m_tbCtrl.InsertItem(0, &tcItem1);
						//m_Ctrltb1->SetWindowText(_T("test"));


						if (m_Ctrltb1->GetDlgItem(intValue)) {
							m_Ctrltb1->SetDlgItemText(intValue, value2);
						}
						if (m_Ctrltb2->GetDlgItem(intValue)) {
							m_Ctrltb2->SetDlgItemText(intValue, value2);
						}
						if (m_Ctrltb3->GetDlgItem(intValue)) {
							m_Ctrltb3->SetDlgItemText(intValue, value2);
							
						}
					}
				}
			}
		}
		wif.close();
	}
}

void TegraRcm::AppendLogBox(CString line) {
	CEdit* pBox = (CEdit*)AfxGetMainWnd()->GetDlgItem(IDC_LOG_BOX);
	CString Content;
	pBox->GetWindowText(Content);
	Content.Append(line);
	AfxGetMainWnd()->SetDlgItemText(IDC_LOG_BOX, Content);
	pBox->LineScroll(pBox->GetLineCount());
	AfxGetMainWnd()->UpdateWindow();

}
void TegraRcm::UpdateLogBox() {
	TCHAR *rfile = GetAbsolutePath(TEXT("out.log"), CSIDL_APPDATA);
	CString Cline;
	std::wifstream fin(rfile, std::ios::binary);
	fin.imbue(std::locale(fin.getloc(), new std::codecvt_utf8_utf16<wchar_t>));
	for (wchar_t c; fin.get(c); ) {
		CString Cchar(c);
		if (Cchar == TEXT("\n")) {
			Cline.Append(TEXT("\r\n"));
			AppendLogBox(Cline);
			Cline.Empty();
		}
		else if(Cchar != TEXT("\r")) {
			Cline.Append(Cchar);
		}
	}
	fin.close();

}


//
// Presets functions
//
string TegraRcm::GetPreset(string param)
{
	TCHAR *rfile = GetAbsolutePath(TEXT("presets.conf"), CSIDL_APPDATA);
	//CT2A rfile_c(rfile, CP_UTF8);
	//TRACE(_T("UTF8: %S\n"), rfile_c.m_psz);

	ifstream readFile(rfile);
	string readout;
	string search = param + "=";
	std::string value = "";
	if (readFile.is_open())
	{
		while (getline(readFile, readout)) {
			if (readout.find(search) != std::string::npos) {
				std::string delimiter = "=";
				value = readout.substr(readout.find(delimiter) + 1, readout.length() + 1);
			}
		}
	}
	readFile.close();
	return value;
}
void TegraRcm::SetPreset(string param, string value)
{
	TCHAR *rfile = GetAbsolutePath(TEXT("presets.conf"), CSIDL_APPDATA);
	TCHAR *wfile = GetAbsolutePath(TEXT("presets.conf.tmp"), CSIDL_APPDATA);
	// Replace or create preset in file
	ofstream outFile(wfile);
	ifstream readFile(rfile);
	string readout;
	string search = param + "=";
	string replace = search + value + "\n";
	BOOL found = FALSE;
	while (getline(readFile, readout)) {
		if (readout.find(search) != std::string::npos) {
			outFile << replace;
			found = TRUE;
		}
		else if(sizeof(readout)>0) {
			outFile << readout + "\n";
		}
	}
	if (!found) {
		outFile << replace;
	}
	outFile.close();
	readFile.close();
	remove(CT2A(rfile));
	rename(CT2A(wfile), CT2A(rfile));

	CString paramStr(param.c_str()), valueStr(value.c_str());
	AppendLogBox(TEXT("Preset \"") + paramStr + TEXT("\" set to : ") + valueStr + TEXT("\r\n"));

}
void TegraRcm::GetFavorites()
{	
	
	Favorites.RemoveAll();
	TCHAR *rfile = GetAbsolutePath(TEXT("favorites.conf"), CSIDL_APPDATA);
	string readout;
	AppendLog("Reading favorites.conf");
	wstring wfilename(rfile);
	string filename(wfilename.begin(), wfilename.end());
	AppendLog(filename);

	ifstream readFile(rfile);
	if (readFile.is_open()) {
		AppendLog("Reading values from favorites.conf");
		while (getline(readFile, readout)) {
			CString fav(readout.c_str(), readout.length());
			wstring wfav = fav;
			string sfav(wfav.begin(), wfav.end());
			AppendLog("Append new favorite : ");
			AppendLog(sfav);

			// For relative path
			int nIndex = fav.ReverseFind(_T(':'));
			if (nIndex <= 0)
			{				
				// Get current directory
				CString csPath;
				TCHAR szPath[_MAX_PATH];
				VERIFY(::GetModuleFileName(AfxGetApp()->m_hInstance, szPath, _MAX_PATH));
				CString csPathf(szPath);
				int nIndex = csPathf.ReverseFind(_T('\\'));
				if (nIndex > 0) csPath = csPathf.Left(nIndex);
				csPath.Append(_T("\\"));
				csPath.Append(fav);
				fav = csPath;
				// Get absolute path
				TCHAR  buffer[4096] = TEXT("");
				GetFullPathName(fav, 4096, buffer, NULL);
				fav = buffer;
			}
			
			Favorites.Add(fav);
		}
	}
	else {
		AppendLog("Error reading favorites.conf");
	}
}
void TegraRcm::AddFavorite(CString value)
{
	// Get current directory
	CString csPath;
	TCHAR szPath[_MAX_PATH];
	VERIFY(::GetModuleFileName(AfxGetApp()->m_hInstance, szPath, _MAX_PATH));
	CString csPathf(szPath);
	int nIndex = csPathf.ReverseFind(_T('\\'));
	if (nIndex > 0) csPath = csPathf.Left(nIndex);
	else csPath.Empty();

	CT2A pPath(csPath.GetBuffer(csPath.GetLength()));
	CT2A pvalue(value.GetBuffer(value.GetLength()));
	char* rvalue = GetRelativeFilename(pPath, pvalue);
	value = rvalue;

	/*
	if (value.Find(csPath) != -1)
	{
		csPath.Append(_T("\\"));
		value.Replace(csPath, _T(""));
	}
	*/

	CString CoutLine(value + _T('\n'));
	CT2CA pszConvertedAnsiString(CoutLine);
	std::string outLine = pszConvertedAnsiString;
	fstream outFile;
	outFile.open(GetAbsolutePath(TEXT("favorites.conf"), CSIDL_APPDATA), fstream::in | fstream::out | fstream::app);
	outFile << outLine;
	outFile.close();

}
void TegraRcm::SaveFavorites()
{
	TCHAR *rfile = GetAbsolutePath(TEXT("favorites.conf"), CSIDL_APPDATA);
	remove(CT2A(rfile));
	for (int i = 0; i < Favorites.GetCount(); i++)
	{
		AddFavorite(Favorites[i]);
	}
}
//
// User message & log
//
void TegraRcm::AppendLog(string message)
{

	// DISABLED
	return;


	// Get time
	char str[32];
	struct tm time_info;
	time_t a = time(nullptr);
	if (localtime_s(&time_info, &a) == 0) {
		//strftime(str, sizeof(str), "%d-%m-%Y %I:%M:%S", &time_info);
		strftime(str, sizeof(str), "%I:%M:%S", &time_info);
	}
	std::string current_time(str);

	// Format line
	string outline;
	outline = current_time + " : ";
	outline += message;
	outline += "\n";

	// Append line in log file
	fstream outFile;
	outFile.open(GetAbsolutePath(TEXT("output_log.txt"), CSIDL_APPDATA), fstream::in | fstream::out | fstream::app);
	outFile << outline;
	outFile.close();
}
void TegraRcm::SendUserMessage(string message, int type)
{
	CA2T wmessage(message.c_str());
	switch (type) {
	case INVALID:
		LabelColor = RGB(255, 0, 0);
		break;
	case VALID:
		LabelColor = RGB(0, 150, 0);
		break;
	default:
		LabelColor = RGB(0, 0, 0);
		break;
	}
	//AfxGetMainWnd()->SetDlgItemText(INFO_LABEL, wmessage);
	AppendLog(message);
	/*
	CString msgStr(message.c_str());
	msgStr.Append(TEXT("\r\n"));
	AppendLogBox(msgStr);
	*/
}


//
// Driver detection & installation
//
void TegraRcm::InstallDriver()
{
	if (ASK_FOR_DRIVER) return;
	CString message = _T("APX device driver is missing. Do you want to install it now ?");
	const int result = MessageBox(m_hWnd, message, _T("APX driver not found !"), MB_YESNOCANCEL | MB_ICONQUESTION);
	if (result == IDYES)
	{
		SHELLEXECUTEINFO shExInfo = { 0 };
		shExInfo.cbSize = sizeof(shExInfo);
		shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		shExInfo.hwnd = 0;
		shExInfo.lpVerb = _T("runas");
		CString csPath;
		TCHAR szPath[_MAX_PATH];
		VERIFY(::GetModuleFileName(AfxGetApp()->m_hInstance, szPath, _MAX_PATH));
		CString csPathf(szPath);
		int nIndex = csPathf.ReverseFind(_T('\\'));
		if (nIndex > 0) csPath = csPathf.Left(nIndex);
		else csPath.Empty();
		CString exe_file = csPath + _T("\\apx_driver\\InstallDriver.exe");
		shExInfo.lpFile = exe_file;
		shExInfo.lpDirectory = 0;
		shExInfo.nShow = SW_SHOW;
		shExInfo.hInstApp = 0;

		if (ShellExecuteEx(&shExInfo))
		{
			CloseHandle(shExInfo.hProcess);
		}
	}
	ASK_FOR_DRIVER = TRUE;
}
typedef int(__cdecl *MYPROC)(LPWSTR);
BOOL TegraRcm::LookForAPXDevice()
{
	unsigned index;
	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA DeviceInfoData;
	TCHAR HardwareID[1024];
	// List all connected USB devices
	hDevInfo = SetupDiGetClassDevs(NULL, TEXT("USB"), NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);
	for (index = 0; ; index++) {
		DeviceInfoData.cbSize = sizeof(DeviceInfoData);
		if (!SetupDiEnumDeviceInfo(hDevInfo, index, &DeviceInfoData)) {
			return FALSE;     // no match
		}
		SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_HARDWAREID, NULL, (BYTE*)HardwareID, sizeof(HardwareID), NULL);
		if (_tcsstr(HardwareID, _T("VID_0955&PID_7321"))) {
			return TRUE;     // match
		}
	}
	return FALSE;
}

void TegraRcm::BitmapDisplay(int IMG)
{
	
	// Init & bitmap pointers
	CStatic*pRcm_not_detected = (CStatic*)AfxGetMainWnd()->GetDlgItem(RCM_PIC_1);
	CStatic*pDriverKO = (CStatic*)AfxGetMainWnd()->GetDlgItem(RCM_PIC_2);
	CStatic*pRcm_detected = (CStatic*)AfxGetMainWnd()->GetDlgItem(RCM_PIC_3);
	CStatic*pInitLogo = (CStatic*)AfxGetMainWnd()->GetDlgItem(RCM_PIC_4);
	CStatic*pLoading = (CStatic*)AfxGetMainWnd()->GetDlgItem(RCM_PIC_5);
	CStatic*pLoaded = (CStatic*)AfxGetMainWnd()->GetDlgItem(RCM_PIC_6);
	CStatic*pError = (CStatic*)AfxGetMainWnd()->GetDlgItem(RCM_PIC_7);

	pRcm_not_detected->ShowWindow(SW_HIDE);
	pDriverKO->ShowWindow(SW_HIDE);
	pRcm_detected->ShowWindow(SW_HIDE);
	pInitLogo->ShowWindow(SW_HIDE);
	pLoading->ShowWindow(SW_HIDE);
	pLoaded->ShowWindow(SW_HIDE);
	pError->ShowWindow(SW_HIDE);

	switch (IMG)
	{
	case INIT_LOGO:
		pInitLogo->ShowWindow(SW_SHOW);
		break;
	case RCM_NOT_DETECTED:
		pRcm_not_detected->ShowWindow(SW_SHOW);
		break;
	case DRIVER_KO:
		pDriverKO->ShowWindow(SW_SHOW);
		break;
	case RCM_DETECTED:
		pRcm_detected->ShowWindow(SW_SHOW);
		break;
	case LOADING:
		pLoading->ShowWindow(SW_SHOW);
		AfxGetMainWnd()->UpdateWindow();
		break;
	case LOADED:
		pLoaded->ShowWindow(SW_SHOW);
		break;
	case LOAD_ERROR:
		pError->ShowWindow(SW_SHOW);
		break;
	default:
		break;
	}
}

//
// Lookup
//
void TegraRcm::LookUp()
{
	// Exit when PAUSE_LKP_DEVICE flag is TRUE
	if (PAUSE_LKP_DEVICE) return;

	// Get RCM device Status
	// This feature has been developped by Rajkosto (copied from TegraRcmSmash)
	TegraRcmSmash device;
	int rc = device.RcmStatus();

	// RCM Status = "RCM detected"
	if (rc >= 0)
	{
		m_Ctrltb1->GetDlgItem(IDC_INJECT)->EnableWindow(TRUE);
		m_Ctrltb2->GetDlgItem(IDC_SHOFEL2)->EnableWindow(TRUE);
		m_Ctrltb2->GetDlgItem(IDC_MOUNT_SD)->EnableWindow(TRUE);
		m_Ctrltb2->GetDlgItem(IDC_DUMP_BISKEY)->EnableWindow(TRUE);
		m_Ctrltb3->GetDlgItem(ID_INSTALL_DRIVER)->EnableWindow(FALSE);
		

	}
	// RCM Status = "USB Driver KO"
	else if (rc > -5)
	{
		m_Ctrltb1->GetDlgItem(IDC_INJECT)->EnableWindow(FALSE);
		m_Ctrltb2->GetDlgItem(IDC_SHOFEL2)->EnableWindow(FALSE);
		m_Ctrltb2->GetDlgItem(IDC_MOUNT_SD)->EnableWindow(FALSE);
		m_Ctrltb2->GetDlgItem(IDC_DUMP_BISKEY)->EnableWindow(FALSE);
		m_Ctrltb3->GetDlgItem(ID_INSTALL_DRIVER)->EnableWindow(TRUE);
	}
	// RCM Status = "RCM not detected"
	else
	{
		m_Ctrltb1->GetDlgItem(IDC_INJECT)->EnableWindow(FALSE);
		m_Ctrltb2->GetDlgItem(IDC_SHOFEL2)->EnableWindow(FALSE);
		m_Ctrltb2->GetDlgItem(IDC_MOUNT_SD)->EnableWindow(FALSE);
		m_Ctrltb2->GetDlgItem(IDC_DUMP_BISKEY)->EnableWindow(FALSE);
		m_Ctrltb3->GetDlgItem(ID_INSTALL_DRIVER)->EnableWindow(TRUE);
		// Delay Auto inject if needed
		if (AUTOINJECT_CURR) DELAY_AUTOINJECT = TRUE;
	}

	// On change RCM status
	if (rc != m_RC || m_RC == -99)
	{
		m_RC = rc;
		//CStatic*pCtrl0 = (CStatic*) m_Parent->GetDlgItem(RCM_PIC_4);
		//pCtrl0->ShowWindow(SW_HIDE);

		// Status changed to "RCM Detected"
		if (rc == 0)
		{
			AppendLogBox(TEXT("RCM Device detected\r\n"));

			BitmapDisplay(RCM_DETECTED);

			CString file;
			m_Ctrltb1->GetDlgItem(PAYLOAD_PATH)->GetWindowTextW(file);

			// Trigger auto inject if payload injection scheduled
			//if (!FIRST_LOOKUP && DELAY_AUTOINJECT && file.GetLength() > 0)
			if (FIRST_LOOKUP) {
				if(AUTOINJECT_CURR) DELAY_AUTOINJECT = TRUE;
				else DELAY_AUTOINJECT = FALSE;
			}
			if (DELAY_AUTOINJECT && file.GetLength() > 0 && AUTOINJECT_CURR)
			{

				BitmapDisplay(LOADING);
				PAYLOAD_FILE = _tcsdup(file);
				TCHAR cmd[MAX_PATH] = TEXT("\"");
				lstrcat(cmd, PAYLOAD_FILE);
				lstrcat(cmd, TEXT("\""));

				int rc = Smasher(cmd);
				if (rc >= 0)
				{
					BitmapDisplay(LOADED);
					//SendUserMessage("Payload injected !", VALID);
					AppendLogBox(TEXT("Payload successfully injected\r\n"));
					if (!CmdShow) ShowTrayIconBalloon(TEXT("Payload injected"), TEXT(" "), 1000, NIIF_INFO);
					WAITING_RECONNECT = TRUE;
				}
				else
				{
					BitmapDisplay(LOAD_ERROR);
					string s = "Error while injecting payload (RC=" + std::to_string(rc) + ")";
					CString rc_str;
					rc_str.Format(L"%d", rc);
					CString error = TEXT("Error while injecting payload (RC=") + rc_str + TEXT(")");
					if (!CmdShow) ShowTrayIconBalloon(TEXT("Error"), error, 1000, NIIF_ERROR);
					//SendUserMessage(s.c_str(), INVALID);
					AppendLogBox(error);

				}
				DELAY_AUTOINJECT = FALSE;
			}
			else
			{
				SendUserMessage(labels[20].name);
			}
		}
		// Status changed to "RCM not detected" or "USB driver KO"
		else
		{
			// Ask for driver install
			if (rc > -5)
			{
				BitmapDisplay(DRIVER_KO);
				AppendLogBox(TEXT("RCM device detected but APX driver is not installed\r\n"));
				InstallDriver();
			}
			else
			{
				if (LookForAPXDevice())
				{
					BitmapDisplay(DRIVER_KO);
					AppendLogBox(TEXT("RCM device detected but APX driver is not installed\r\n"));
					InstallDriver();
				}
				else
				{
					BitmapDisplay(RCM_NOT_DETECTED);
					if (!FIRST_LOOKUP) AppendLogBox(TEXT("RCM device disconnected\r\n"));
					else AppendLogBox(TEXT("No RCM device detected\r\n"));
					if (FIRST_LOOKUP) SendUserMessage(labels[21].name);
				}
			}
		}
		// Status changed to "RCM not detected" -> Disable WAITING_RECONNECT flag
		if (rc <= -5) WAITING_RECONNECT = FALSE;

		DestroyTrayIcon();
		CreateTrayIcon();
	}
	m_RC = rc;
	FIRST_LOOKUP = FALSE;
}

//
// Smasher => TegraRcmSmash.exe calls
//
int TegraRcm::Smasher(TCHAR args[4096], BOOL bInheritHandles)
{
	if (WAITING_RECONNECT)
	{
		CString message = _T("Payload already injected. Are you sure you want to overwrite the stack again ?");
		const int result = MessageBox(m_hWnd, message, _T("WARNING !"), MB_YESNOCANCEL | MB_ICONQUESTION);
		if (result != IDYES)
		{
			DELAY_AUTOINJECT = FALSE;
			m_RC = -99;
			return -99;
		}
		WAITING_RECONNECT = FALSE;
	}

	// This should fix RC-50 issue, ! At last.
	TCHAR szPath[_MAX_PATH];
	VERIFY(::GetModuleFileName(AfxGetApp()->m_hInstance, szPath, _MAX_PATH));
	CString csPathf(szPath);
	int nIndex = csPathf.ReverseFind(_T('\\'));
	if (nIndex > 0) csPath = csPathf.Left(nIndex);
	else csPath.Empty();
	CString csPath2(csPath);
	csPath.Append(TEXT(".\\TegraRcmSmash.exe "));
	TCHAR cmd[4096];
	_tcscpy_s(cmd, csPath);
	lstrcat(cmd, args);


	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	TCHAR *rfile = GetAbsolutePath(TEXT("out.log"), CSIDL_APPDATA);
	remove(CT2A(rfile));

	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	BOOL ret = FALSE;
	DWORD flags = CREATE_NO_WINDOW;
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags |= STARTF_USESTDHANDLES;
	si.hStdInput = NULL;
	if (bInheritHandles) {
	    HANDLE h = CreateFile(rfile,
			GENERIC_WRITE,
			FILE_SHARE_WRITE | FILE_SHARE_READ,
			&sa,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		si.hStdError = h;
		si.hStdOutput = h;
	}

	CString argsStr(args);
	CString msgStr(TEXT("Invoking TegraRcmSmash.exe with args : ") + argsStr + TEXT("\r\n"));
	AppendLogBox(msgStr);


	ret = CreateProcess(NULL, cmd, NULL, NULL, bInheritHandles, flags, NULL, NULL, &si, &pi);
	int rc = -50;
	if (NULL != ret)
	{
		WaitForSingleObject(pi.hProcess, INFINITE);
		DWORD exit_code;
		if (FALSE != GetExitCodeProcess(pi.hProcess, &exit_code))
		{
			AppendLog("GetExitCodeProcess != FALSE");
			if (STILL_ACTIVE != exit_code)
			{
				rc = exit_code;
			}
			else
			{
				rc = -52;
				AppendLog("RC = -52");
			}

		}
		else
		{
			rc = -51;
		}
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else {
		int lastErrorCode = GetLastError();
		LPWSTR lpMsgBuf = NULL;
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
			NULL,
			lastErrorCode,
			0,
			(LPWSTR)&lpMsgBuf,
			0,
			NULL);
		std::string MyString = CW2A(lpMsgBuf);
		std::string intStr = std::to_string(lastErrorCode);
		//AppendLog(intStr);
		//AppendLog(MyString);
		
		CString CLogBuf(lpMsgBuf);
		msgStr = TEXT("Error : ") + CLogBuf + TEXT("\r\n");
	}
	if (bInheritHandles) {
		UpdateLogBox();
	}
	return rc;
}
//
// System functions
//
ULONGLONG TegraRcm::GetDllVersion(LPCTSTR lpszDllName)
{
	ULONGLONG ullVersion = 0;
	HINSTANCE hinstDll;
	hinstDll = LoadLibrary(lpszDllName);
	if (hinstDll)
	{
		DLLGETVERSIONPROC pDllGetVersion;
		pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, "DllGetVersion");
		if (pDllGetVersion)
		{
			DLLVERSIONINFO dvi;
			HRESULT hr;
			ZeroMemory(&dvi, sizeof(dvi));
			dvi.cbSize = sizeof(dvi);
			hr = (*pDllGetVersion)(&dvi);
			if (SUCCEEDED(hr))
				ullVersion = MAKEDLLVERULL(dvi.dwMajorVersion, dvi.dwMinorVersion, 0, 0);
		}
		FreeLibrary(hinstDll);
	}
	return ullVersion;
}

void TegraRcm::KillRunningProcess(CString process) {

	CString compare;
	DWORD processID;
	DWORD currentProcessID = GetCurrentProcessId();
	bool procRunning = false;

	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hProcessSnap == INVALID_HANDLE_VALUE) {
		procRunning = false;
	}
	else {
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hProcessSnap, &pe32)) { // Gets first running process
			if (pe32.szExeFile == process) {
				procRunning = true;
			}
			else {
				// loop through all running processes looking for process
				while (Process32Next(hProcessSnap, &pe32)) {
					// Set to an AnsiString instead of Char[] to make compare easier
					compare = pe32.szExeFile;
					processID = pe32.th32ProcessID;
					if (compare == process && processID != currentProcessID) {
						// if found process is running, set to true and break from loop
						procRunning = true;
						HWND hProcess = find_main_window(processID);
						SendMessage(hProcess, WM_CLOSE, 0, 0);
						DWORD err = GetLastError();
						int test = 1;
						//break;
					}
				}
			}
			// clean the snapshot object
			CloseHandle(hProcessSnap);
		}
	}
}


struct handle_data {
	unsigned long process_id;
	HWND window_handle;
};
BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam)
{
	handle_data& data = *(handle_data*)lParam;
	unsigned long process_id = 0;
	GetWindowThreadProcessId(handle, &process_id);
	if (data.process_id != process_id)
		return TRUE;
	data.window_handle = handle;
	return FALSE;
}

HWND TegraRcm::find_main_window(unsigned long process_id)
{
	handle_data data;
	data.process_id = process_id;
	data.window_handle = 0;
	EnumWindows(enum_windows_callback, (LPARAM)&data);
	return data.window_handle;
}

TCHAR* TegraRcm::GetAbsolutePath(TCHAR* relative_path, DWORD  dwFlags)
{
	//
	//
	BOOL PORTABLE = TRUE;
	//
	//

	if (PORTABLE) {
		// Get current directory
		CString csPath;
		TCHAR szPath[_MAX_PATH];
		VERIFY(::GetModuleFileName(AfxGetApp()->m_hInstance, szPath, _MAX_PATH));
		CString csPathf(szPath);
		int nIndex = csPathf.ReverseFind(_T('\\'));
		if (nIndex > 0) csPath = csPathf.Left(nIndex);
		else csPath.Empty();
		CString csPath2;
		csPath2 = csPath;
		csPath2 += TEXT("\\");
		csPath2 += relative_path;
		return _tcsdup(csPath2);
	}
	else
	{
		// USE THIS INSTEAD TO BUILD FOR MSI PACKAGER
		TCHAR szPath[MAX_PATH];
		if (SUCCEEDED(SHGetFolderPath(NULL, dwFlags, NULL, SHGFP_TYPE_CURRENT, szPath)))
		{
			if (dwFlags == CSIDL_APPDATA)   PathAppend(szPath, _T("\\TegraRcmGUI"));
			PathAppend(szPath, relative_path);
			return _tcsdup(szPath);
		}
		return _T("");
	}
}


// GetRelativeFilename(), by Rob Fisher.
// rfisher@iee.org
// http://come.to/robfisher
// defines
#define MAX_FILENAME_LEN 512
// The number of characters at the start of an absolute filename.  e.g. in DOS,
// absolute filenames start with "X:\" so this value should be 3, in UNIX they start
// with "\" so this value should be 1.
#define ABSOLUTE_NAME_START 3
// set this to '\\' for DOS or '/' for UNIX
#define SLASH '\\'
// Given the absolute current directory and an absolute file name, returns a relative file name.
// For example, if the current directory is C:\foo\bar and the filename C:\foo\whee\text.txt is given,
// GetRelativeFilename will return ..\whee\text.txt.
char* TegraRcm::GetRelativeFilename(char *currentDirectory, char *absoluteFilename)
{
	// declarations - put here so this should work in a C compiler
	int afMarker = 0, rfMarker = 0;
	int cdLen = 0, afLen = 0;
	int i = 0;
	int levels = 0;
	static char relativeFilename[MAX_FILENAME_LEN + 1];
	cdLen = strlen(currentDirectory);
	afLen = strlen(absoluteFilename);

	// make sure the names are not too long or too short
	if (cdLen > MAX_FILENAME_LEN || cdLen < ABSOLUTE_NAME_START + 1 ||
		afLen > MAX_FILENAME_LEN || afLen < ABSOLUTE_NAME_START + 1)
	{
		return NULL;
	}

	// Handle DOS names that are on different drives:
	if (currentDirectory[0] != absoluteFilename[0])
	{
		// not on the same drive, so only absolute filename will do
		strcpy(relativeFilename, absoluteFilename);
		return relativeFilename;
	}
	// they are on the same drive, find out how much of the current directory
	// is in the absolute filename
	i = ABSOLUTE_NAME_START;
	while (i < afLen && i < cdLen && currentDirectory[i] == absoluteFilename[i])
	{
		i++;
	}
	if (i == cdLen && (absoluteFilename[i] == SLASH || absoluteFilename[i - 1] == SLASH))
	{
		// the whole current directory name is in the file name,
		// so we just trim off the current directory name to get the
		// current file name.
		if (absoluteFilename[i] == SLASH)
		{
			// a directory name might have a trailing slash but a relative
			// file name should not have a leading one...
			i++;
		}
		strcpy(relativeFilename, &absoluteFilename[i]);
		return relativeFilename;
	}
	// The file is not in a child directory of the current directory, so we
	// need to step back the appropriate number of parent directories by
	// using "..\"s.  First find out how many levels deeper we are than the
	// common directory
	afMarker = i;
	levels = 1;
	// count the number of directory levels we have to go up to get to the
	// common directory
	while (i < cdLen)
	{
		i++;
		if (currentDirectory[i] == SLASH)
		{
			// make sure it's not a trailing slash
			i++;
			if (currentDirectory[i] != '\0')
			{
				levels++;
			}
		}
	}
	// move the absolute filename marker back to the start of the directory name
	// that it has stopped in.
	while (afMarker > 0 && absoluteFilename[afMarker - 1] != SLASH)
	{
		afMarker--;
	}
	// check that the result will not be too long
	if (levels * 3 + afLen - afMarker > MAX_FILENAME_LEN)
	{
		return NULL;
	}

	// add the appropriate number of "..\"s.
	rfMarker = 0;
	for (i = 0; i < levels; i++)
	{
		relativeFilename[rfMarker++] = '.';
		relativeFilename[rfMarker++] = '.';
		relativeFilename[rfMarker++] = SLASH;
	}
	// copy the rest of the filename into the result string
	strcpy(&relativeFilename[rfMarker], &absoluteFilename[afMarker]);
	return relativeFilename;
}