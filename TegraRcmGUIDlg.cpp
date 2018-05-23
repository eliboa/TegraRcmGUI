
// TegraRcmGUIDlg.cpp : implementation file
//

#include "stdafx.h"
#include "afxdialogex.h"
#include "TegraRcmGUI.h"
#include "TegraRcmGUIDlg.h"


using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

TCHAR* PAYLOAD_FILE;
int RCM_STATUS = -10;
BOOL WAITING_RECONNECT = FALSE;
BOOL AUTOINJECT_CURR = FALSE;
BOOL PREVENT_AUTOINJECT = TRUE;
BOOL DELAY_AUTOINJECT = FALSE;
BOOL ASK_FOR_DRIVER = FALSE;
BOOL PAUSE_LKP_DEVICE = FALSE;

CString csPath;

// CTegraRcmGUIDlg dialog

CTegraRcmGUIDlg::CTegraRcmGUIDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_TEGRARCMGUI_DIALOG, pParent)
	, STATUS(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTegraRcmGUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, RCM_PIC_1, RCM_BITMAP1);
	DDX_Control(pDX, RCM_PIC_2, RCM_BITMAP2);
	DDX_Control(pDX, RCM_PIC_3, RCM_BITMAP3);
	DDX_Control(pDX, RCM_PIC_4, RCM_BITMAP0);
	DDX_Control(pDX, RCM_PIC_5, RCM_BITMAP4);
	DDX_Control(pDX, RCM_PIC_6, RCM_BITMAP5);
	DDX_Control(pDX, RCM_PIC_7, RCM_BITMAP6);
	DDX_Control(pDX, PAYLOAD_PATH, m_EditBrowse);
}

BEGIN_MESSAGE_MAP(CTegraRcmGUIDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(PAYLOAD_PATH, &CTegraRcmGUIDlg::OnEnChangePath)
	ON_BN_CLICKED(IDC_INJECT, &CTegraRcmGUIDlg::InjectPayload)
	ON_BN_CLICKED(IDC_SHOFEL2, &CTegraRcmGUIDlg::OnBnClickedShofel2)
	ON_BN_CLICKED(IDC_MOUNT_SD, &CTegraRcmGUIDlg::OnBnClickedMountSd)
END_MESSAGE_MAP()



HBRUSH CTegraRcmGUIDlg::OnCtlColor(CDC* pDC, CWnd *pWnd, UINT nCtlColor)
{
	switch (nCtlColor)
	{
	case CTLCOLOR_STATIC:
		if (GetDlgItem(IDC_RAJKOSTO)->GetSafeHwnd() == pWnd->GetSafeHwnd() || GetDlgItem(SEPARATOR)->GetSafeHwnd() == pWnd->GetSafeHwnd())
		{
			pDC->SetTextColor(RGB(192, 192, 192));
			pDC->SetBkMode(TRANSPARENT);
			return (HBRUSH)GetStockObject(NULL_BRUSH);
		}	
	default:
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
}
// CTegraRcmGUIDlg message handlers

BOOL CTegraRcmGUIDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Accessibility
	EnableActiveAccessibility();

	// Get current directory
	TCHAR szPath[_MAX_PATH];
	VERIFY(::GetModuleFileName(AfxGetApp()->m_hInstance, szPath, _MAX_PATH));
	CString csPathf(szPath);
	int nIndex = csPathf.ReverseFind(_T('\\'));
	if (nIndex > 0) csPath = csPathf.Left(nIndex);
	else csPath.Empty();

	// Initialize bitmap
	RCM_BITMAP0.SetBitmap(INIT_LOGO);
	RCM_BITMAP1.SetBitmap(RCM_NOT_DETECTED);
	RCM_BITMAP2.SetBitmap(DRIVER_KO);
	RCM_BITMAP3.SetBitmap(RCM_DETECTED);
	RCM_BITMAP4.SetBitmap(LOADING);
	RCM_BITMAP5.SetBitmap(LOADED);
	RCM_BITMAP6.SetBitmap(LOAD_ERROR);

	BitmapDisplay(INIT_LOGO);

	// Check for APX driver at startup
	//BOOL isDriverInstalled = LookForDriver();
	//if (!isDriverInstalled) InstallDriver();

	// Read & apply presets
	string value = GetPreset("AUTO_INJECT");
	if (value == "TRUE")
	{
		AUTOINJECT_CURR = TRUE;
		CMFCButton*checkbox = (CMFCButton*)GetDlgItem(AUTO_INJECT);
		checkbox->SetCheck(BST_CHECKED);
		
	}
	
	string pfile = GetPreset("PAYLOAD_FILE");
	CString file(pfile.c_str());
	this->GetDlgItem(PAYLOAD_PATH)->SetWindowTextW(file);
	//PREVENT_AUTOINJECT = TRUE;
	
	// Menu
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set icons
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

									// Start timer to check RCM status every second
	CTegraRcmGUIDlg::StartTimer();

	return TRUE;
}

void CTegraRcmGUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);
}


void CTegraRcmGUIDlg::OnPaint()
{

	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}
// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTegraRcmGUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
const UINT ID_TIMER_MINUTE = 0x1001;
const UINT ID_TIMER_SECONDS = 0x1000;
// Start the timers.
void CTegraRcmGUIDlg::StartTimer()
{
	// Set timer for Minutes.
	//SetTimer(ID_TIMER_MINUTE, 60 * 1000, 0);
	// Set timer for Seconds.
	SetTimer(ID_TIMER_SECONDS, 1000, 0);
}

// Stop the timers.
void CTegraRcmGUIDlg::StopTimer()
{
	// Stop both timers.
	KillTimer(ID_TIMER_MINUTE);
	KillTimer(ID_TIMER_SECONDS);
}

void CTegraRcmGUIDlg::BitmapDisplay(int IMG)
{
	// Init & bitmap pointers
	CStatic*pRcm_not_detected = (CStatic*)GetDlgItem(RCM_PIC_1);
	CStatic*pDriverKO = (CStatic*)GetDlgItem(RCM_PIC_2);
	CStatic*pRcm_detected = (CStatic*)GetDlgItem(RCM_PIC_3);
	CStatic*pInitLogo = (CStatic*)GetDlgItem(RCM_PIC_4);
	CStatic*pLoading = (CStatic*)GetDlgItem(RCM_PIC_5);
	CStatic*pLoaded = (CStatic*)GetDlgItem(RCM_PIC_6);
	CStatic*pError = (CStatic*)GetDlgItem(RCM_PIC_7);

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
			UpdateWindow();
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
// Timer Handler.
void CTegraRcmGUIDlg::OnTimer(UINT nIDEvent)
{
	// Each second
	if (nIDEvent == ID_TIMER_SECONDS)
	{
		// Exit when PAUSE_LKP_DEVICE flag is TRUE
		if (PAUSE_LKP_DEVICE) return;
		
		// Get Auto inject checkbox value (checked, unchecked)
		CButton *m_ctlCheck = (CButton*)GetDlgItem(AUTO_INJECT);
		BOOL IsCheckChecked = (m_ctlCheck->GetCheck() == 1) ? true : false;
		
		if (AUTOINJECT_CURR != IsCheckChecked)
		{
			// Auto inject option enabled
			if (IsCheckChecked)
			{
				SetPreset("AUTO_INJECT", "TRUE");
				DELAY_AUTOINJECT = TRUE;
			}
			// Auto inject option disabled
			else
			{
				SetPreset("AUTO_INJECT", "FALSE");
				DELAY_AUTOINJECT = FALSE;
			}
			// Save current checkbox value
			AUTOINJECT_CURR = IsCheckChecked;
		}
		
		// Get RCM device Status
		// This feature has been developped by Rajkosto (copied from TegraRcmSmash)
		TegraRcmSmash device;
		int rc = device.RcmStatus();

		std::string s = "";

		// RCM Status = "RCM detected"
		if (rc >= 0)
		{
			this->GetDlgItem(IDC_INJECT)->EnableWindow(TRUE);
			this->GetDlgItem(IDC_SHOFEL2)->EnableWindow(TRUE);
			this->GetDlgItem(IDC_MOUNT_SD)->EnableWindow(TRUE);

		}
		// RCM Status = "USB Driver KO"
		else if (rc > -5)
		{
			this->GetDlgItem(IDC_INJECT)->EnableWindow(FALSE);
			this->GetDlgItem(IDC_SHOFEL2)->EnableWindow(FALSE);
			this->GetDlgItem(IDC_MOUNT_SD)->EnableWindow(FALSE);
			s = "lbusbK driver is needed !";
		}
		// RCM Status = "RCM not detected"
		else
		{
			this->GetDlgItem(IDC_INJECT)->EnableWindow(FALSE);
			this->GetDlgItem(IDC_SHOFEL2)->EnableWindow(FALSE);
			this->GetDlgItem(IDC_MOUNT_SD)->EnableWindow(FALSE);
			s = "Waiting for Switch in RCM mode.";

			// Delay Auto inject if needed
			if (AUTOINJECT_CURR) DELAY_AUTOINJECT = TRUE;
		}

		// On change RCM status
		if (rc != RCM_STATUS)
		{
			RCM_STATUS = rc;
			CStatic*pCtrl0 = (CStatic*)GetDlgItem(RCM_PIC_4);
			pCtrl0->ShowWindow(SW_HIDE);

			// Status changed to "RCM Detected"
			if (rc == 0)
			{
				BitmapDisplay(RCM_DETECTED);

				CString file;
				this->GetDlgItem(PAYLOAD_PATH)->GetWindowTextW(file);

				// Trigger auto inject if payload injection scheduled
				if (DELAY_AUTOINJECT && file.GetLength() > 0)
				{
					InjectPayload();
					DELAY_AUTOINJECT = FALSE;
				}
				else
				{
					SetDlgItemText(INFO_LABEL, TEXT("\nSelect a payload :"));
				}
			}
			// Status changed to "RCM not detected" or "USB driver KO"
			else
			{
				// Ask for driver install 
				if (rc > -5)
				{
					BitmapDisplay(DRIVER_KO);
					InstallDriver();
				} 
				else
				{
					BitmapDisplay(RCM_NOT_DETECTED);
				}
			}
			// Status changed to "RCM not detected" -> Disable WAITING_RECONNECT flag
			if (rc <= -5) WAITING_RECONNECT = FALSE;
		}
		RCM_STATUS = rc;
	}
}

//
// On change payload path
void CTegraRcmGUIDlg::OnEnChangePath()
{
	CString file;
	this->GetDlgItem(PAYLOAD_PATH)->GetWindowTextW(file);
	PAYLOAD_FILE = _tcsdup(file);

	if (!PREVENT_AUTOINJECT)
	{
		// Save payload path
		CT2CA pszConvertedAnsiString(file);
		std::string file_c(pszConvertedAnsiString);
		SetPreset("PAYLOAD_FILE", file_c);
	}

	std::string s = "\nSelect a payload :";

	CButton *m_ctlCheck = (CButton*)GetDlgItem(AUTO_INJECT);
	BOOL IsCheckChecked = (m_ctlCheck->GetCheck() == 1) ? true : false;
	// If Auto inject option enabled
	if (IsCheckChecked && !PREVENT_AUTOINJECT)
	{
		// Delay auto inject if RCM not detected
		if (RCM_STATUS != 0)
		{
			DELAY_AUTOINJECT = TRUE;
			s = "Payload injection scheduled.\nWaiting for RCM mode.";
		}
		// Inject payload if RCM detected
		else InjectPayload();
	}
	PREVENT_AUTOINJECT = FALSE;
	CA2T wt(s.c_str());
	SetDlgItemText(INFO_LABEL, wt);
}


//
// User payload injection
void CTegraRcmGUIDlg::InjectPayload()
{
	string s;
	if (PAYLOAD_FILE == nullptr) {
		BitmapDisplay(LOAD_ERROR);
		SetDlgItemText(INFO_LABEL, TEXT("\nNo file selected !"));
		return;
	}
	BitmapDisplay(LOADING);
	int rc = Smasher(PAYLOAD_FILE);
	if (rc >= 0)
	{
		BitmapDisplay(LOADED);
		s = "\nPayload injected !";
		WAITING_RECONNECT = TRUE;
	}
	else
	{
		BitmapDisplay(LOAD_ERROR);
		s = "Error while injecting payload (RC=" + std::to_string(rc) + ")";
	}
	CA2T wt(s.c_str());
	CTegraRcmGUIDlg::SetDlgItemText(INFO_LABEL, wt);
}

void CTegraRcmGUIDlg::OnBnClickedShofel2()
{
	TCHAR *exe_dir = GetAbsolutePath(TEXT(""), CSIDL_APPDATA);

	string s;
	TCHAR *COREBOOT_FILE = GetAbsolutePath(TEXT("shofel2\\coreboot\\coreboot.rom"), CSIDL_APPDATA);
	TCHAR *PAYLOAD = GetAbsolutePath(TEXT("shofel2\\coreboot\\cbfs.bin"), CSIDL_APPDATA);
	CString COREBOOT_FILE2 = COREBOOT_FILE;
	CString COREBOOT = _T("CBFS+") + COREBOOT_FILE2;

	std::ifstream infile(COREBOOT_FILE);
	BOOL coreboot_exists = infile.good();
	std::ifstream infile2(PAYLOAD);
	BOOL payload_exists = infile2.good();

	if (!coreboot_exists || !payload_exists) {
		SetDlgItemText(INFO_LABEL, TEXT("Linux coreboot not found in \\shofel2 dir"));
		CString message = _T("Kernel not found in shofel2 directory. Do you want to automatically download arch linux kernel from SoulCipher repo ?");
		const int result = MessageBox(message, _T("Kernel not found"), MB_YESNOCANCEL | MB_ICONQUESTION);
		if (result == IDYES)
		{
			PROCESS_INFORMATION pif;
			STARTUPINFO si;
			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			TCHAR *download_script = GetAbsolutePath(TEXT("shofel2\\download.bat"), CSIDL_APPDATA);
			BOOL bRet = CreateProcess(download_script, NULL, NULL, NULL, FALSE, 0, NULL, exe_dir, &si, &pif);
		}
		return; // TO-DO : Remove return for coreboot injection after download
	}
	BitmapDisplay(LOADING);
	SetDlgItemText(INFO_LABEL, TEXT("Loading coreboot. Please wait."));

	//int rc = device.SmashMain(5, args);
	TCHAR cmd[MAX_PATH] = TEXT("--relocator= \"");
	lstrcat(cmd, _tcsdup(PAYLOAD));
	lstrcat(cmd, TEXT("\" \"CBFS:"));
	lstrcat(cmd, _tcsdup(COREBOOT_FILE));
	lstrcat(cmd, TEXT("\""));
	int rc = Smasher(cmd);

	if (rc >= 0 || rc < -7)
	{
		SetDlgItemText(INFO_LABEL, TEXT("Coreboot injected. Waiting 5s for device..."));
		Sleep(5000);

		PROCESS_INFORMATION pif;
		STARTUPINFO si;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		TCHAR *imx_script = GetAbsolutePath(TEXT("shofel2\\imx_usb.bat"), CSIDL_APPDATA);
		SetDlgItemText(INFO_LABEL, TEXT("Loading coreboot... Please wait."));
		BOOL ret = CreateProcess(imx_script, NULL, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, exe_dir, &si, &pif);
		int rc = -50;
		if (NULL != ret)
		{
			WaitForSingleObject(pif.hProcess, INFINITE);
			DWORD exit_code;
			if (FALSE != GetExitCodeProcess(pif.hProcess, &exit_code))
			{
				if (STILL_ACTIVE != exit_code) rc = exit_code;
			}
			else rc = -51;
			CloseHandle(pif.hProcess);
			CloseHandle(pif.hThread);
		}

		if (rc == 0)
		{
			BitmapDisplay(LOADED);
			s = "\nCoreboot loaded !";
		}
		else
		{
			BitmapDisplay(LOAD_ERROR);
			s = "Error while loading imx_usb.exe";
		}
	}
	else
	{
		s = "Error while injecting payload. (RC=" + std::to_string(rc) + ")";
	}
	CA2T wt2(s.c_str());
	SetDlgItemText(INFO_LABEL, wt2);

}


string CTegraRcmGUIDlg::GetPreset(string param)
{
	TCHAR *rfile = GetAbsolutePath(TEXT("presets.conf"), CSIDL_APPDATA);
	CT2A rfile_c(rfile, CP_UTF8);
	TRACE(_T("UTF8: %S\n"), rfile_c.m_psz);

	ifstream readFile(rfile_c);
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

void CTegraRcmGUIDlg::SetPreset(string param, string value)
{
	TCHAR *rfile = GetAbsolutePath(TEXT("presets.conf"), CSIDL_APPDATA);
	TCHAR *wfile = GetAbsolutePath(TEXT("presets.conf.tmp"), CSIDL_APPDATA);
	CT2A rfile_c(rfile, CP_UTF8);
	TRACE(_T("UTF8: %S\n"), rfile_c.m_psz);
	CT2A wfile_c(wfile, CP_UTF8);
	TRACE(_T("UTF8: %S\n"), wfile_c.m_psz);

	// Replace or create preset in file
	ofstream outFile(wfile_c);
	ifstream readFile(rfile_c);
	string readout;
	string search = param + "=";
	string replace = "\n" + search + value + "\n";
	BOOL found = FALSE;
	while (getline(readFile, readout)) {
		if (readout.find(search) != std::string::npos) {
			outFile << replace;
			found = TRUE;
		}
		else {
			outFile << readout;
		}
	}
	if (!found) {
		outFile << replace;
	}
	outFile.close();
	readFile.close();
	remove(rfile_c);
	rename(wfile_c, rfile_c);
}


void CTegraRcmGUIDlg::InstallDriver()
{
	CString message = _T("APX device driver is missing. Do you want to install it now ?");
	const int result = MessageBox(message, _T("APX driver not found !"), MB_YESNOCANCEL | MB_ICONQUESTION);
	if (result == IDYES)
	{
		SHELLEXECUTEINFO shExInfo = { 0 };
		shExInfo.cbSize = sizeof(shExInfo);
		shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		shExInfo.hwnd = 0;
		shExInfo.lpVerb = _T("runas");
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


BOOL CTegraRcmGUIDlg::LookForDriver()
{
	TCHAR *system_dir = GetAbsolutePath(TEXT("libusbK.dll"), CSIDL_SYSTEM);
	std::ifstream infile(system_dir);
	BOOL file_exists = infile.good();
	return file_exists;
}


void CTegraRcmGUIDlg::OnBnClickedMountSd()
{
	BitmapDisplay(LOADING);
	string s;
	TCHAR args[] = TEXT("memloader\\memloader_usb.bin -r --dataini=memloader\\ums_sd.ini");
	int rc = Smasher(args);
	if (rc < 0)
	{
		
		BitmapDisplay(LOAD_ERROR);
		s = "Error while loading payload (RC=" + std::to_string(rc) + ")";
	}
	else
	{
		BitmapDisplay(LOADING);
		s = "SD tool (by rajkosto) injected";
	}
	CA2T wt(s.c_str());
	CTegraRcmGUIDlg::SetDlgItemText(INFO_LABEL, wt);
}

int CTegraRcmGUIDlg::Smasher(TCHAR args[])
{
	if (WAITING_RECONNECT)
	{
		CString message = _T("Payload already injected. Are you sure you want to overwrite the stack again ?");
		const int result = MessageBox(message, _T("WARNING !"), MB_YESNOCANCEL | MB_ICONQUESTION);
		if (result != IDYES)
		{
			DELAY_AUTOINJECT = FALSE;
			RCM_STATUS = -99;
			return -99;
		}
		WAITING_RECONNECT = FALSE;
	}

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	BOOL ret = FALSE;
	DWORD flags = CREATE_NO_WINDOW;
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags |= STARTF_USESTDHANDLES;
	si.hStdInput = NULL;
	TCHAR cmd[MAX_PATH] = TEXT(".\\TegraRcmSmash.exe ");
	lstrcat(cmd, args);
	ret = CreateProcess(NULL, cmd, NULL, NULL, TRUE, flags, NULL, NULL, &si, &pi);
	int rc = -50;
	if (NULL != ret)
	{
		WaitForSingleObject(pi.hProcess, INFINITE);
		DWORD exit_code;
		if (FALSE != GetExitCodeProcess(pi.hProcess, &exit_code))
		{
			if (STILL_ACTIVE != exit_code)
			{
				rc = exit_code;
			}
		}
		else
		{
			rc = -51;
		}
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	return rc;
}

TCHAR* CTegraRcmGUIDlg::GetAbsolutePath(TCHAR* relative_path, DWORD  dwFlags)
{
	TCHAR szPath[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, dwFlags, NULL, SHGFP_TYPE_CURRENT, szPath)))
	{
		if (dwFlags == CSIDL_APPDATA)   PathAppend(szPath, _T("\\TegraRcmGUI"));
		PathAppend(szPath, relative_path);
		return _tcsdup(szPath);
	}
	return _T("");
}

