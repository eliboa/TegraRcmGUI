
// TegraRcmGUIDlg.cpp : implementation file
//

#include "stdafx.h"
#include "afxdialogex.h"
#include "TegraRcmGUI.h"
#include "TegraRcmGUIDlg.h"
#include "TegraRcmSmash.h"
#include "res/BitmapPicture.h"
#include <windows.h>
#include <string> 
#include <thread>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>

std::string exec(const char* cmd) {
	std::array<char, 128> buffer;
	std::string result;
	std::shared_ptr<FILE> pipe(_popen(cmd, "r"), _pclose);
	if (!pipe) throw std::runtime_error("_popen() failed!");
	while (!feof(pipe.get())) {
		if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
			result += buffer.data();
	}
	return result;
}

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

TCHAR* PAYLOAD_FILE;
int RCM_STATUS = -10;
int LOOP_WAIT = 0;

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
	DDX_Control(pDX, PAYLOAD_PATH, m_EditBrowse);
}

BEGIN_MESSAGE_MAP(CTegraRcmGUIDlg, CDialog)
	ON_WM_TIMER()
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(PAYLOAD_PATH, &CTegraRcmGUIDlg::OnEnChangePath)
	ON_BN_CLICKED(IDC_INJECT, &CTegraRcmGUIDlg::OnBnClickedButton)
	ON_BN_CLICKED(IDC_SHOFEL2, &CTegraRcmGUIDlg::OnBnClickedShofel2)
END_MESSAGE_MAP()


// CTegraRcmGUIDlg message handlers

BOOL CTegraRcmGUIDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	RCM_BITMAP0.SetBitmap(INIT_LOGO);
	RCM_BITMAP1.SetBitmap(RCM_NOT_DETECTED);
	RCM_BITMAP2.SetBitmap(DRIVER_KO);
	RCM_BITMAP3.SetBitmap(RCM_DETECTED);
	SendMessage(PAYLOAD_PATH, BM_CLICK, 0);

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	CTegraRcmGUIDlg::StartTimer();
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}




void CTegraRcmGUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

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
	SetTimer(ID_TIMER_MINUTE, 60 * 1000, 0);

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

// Timer Handler.
void CTegraRcmGUIDlg::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == ID_TIMER_SECONDS)
	{
		
		TegraRcmSmash device;
		int rc = device.RcmStatus();
		
		CStatic*pCtrl1 = (CStatic*)GetDlgItem(RCM_PIC_1);
		CStatic*pCtrl2 = (CStatic*)GetDlgItem(RCM_PIC_2);
		CStatic*pCtrl3 = (CStatic*)GetDlgItem(RCM_PIC_3);
		
		std::string s = "";
		if (rc >= 0)
		{
			pCtrl1->ShowWindow(SW_HIDE);
			pCtrl2->ShowWindow(SW_HIDE);
			pCtrl3->ShowWindow(SW_SHOW);
			this->GetDlgItem(IDC_INJECT)->EnableWindow(TRUE);
			this->GetDlgItem(IDC_SHOFEL2)->EnableWindow(TRUE);
		}
		else if (rc > -5)
		{
			pCtrl1->ShowWindow(SW_HIDE);
			pCtrl2->ShowWindow(SW_SHOW);
			pCtrl3->ShowWindow(SW_HIDE);
			this->GetDlgItem(IDC_INJECT)->EnableWindow(FALSE);
			this->GetDlgItem(IDC_SHOFEL2)->EnableWindow(FALSE);
			s = "Please Install the lbusbK driver (download Zadig)";

		}
		else
		{
			pCtrl1->ShowWindow(SW_SHOW);
			pCtrl2->ShowWindow(SW_HIDE);
			pCtrl3->ShowWindow(SW_HIDE);
			this->GetDlgItem(IDC_INJECT)->EnableWindow(FALSE);
			this->GetDlgItem(IDC_SHOFEL2)->EnableWindow(FALSE);
			s = "Waiting for Switch in RCM mode.";
		}	

		if (rc != RCM_STATUS)
		{
			CStatic*pCtrl0 = (CStatic*)GetDlgItem(RCM_PIC_4);
			pCtrl0->ShowWindow(SW_HIDE);
			CA2T wt(s.c_str());
			SetDlgItemText(INFO_LABEL, wt);
		}
		RCM_STATUS = rc;
	}
}

void CTegraRcmGUIDlg::OnEnChangePath()
{
	CString file;
	this->GetDlgItem(PAYLOAD_PATH)->GetWindowTextW(file);
	PAYLOAD_FILE = _tcsdup(file);
	
	std::string s = "";
	CA2T wt(s.c_str());
	SetDlgItemText(INFO_LABEL, wt);
}


void CTegraRcmGUIDlg::OnBnClickedButton()
{
	LOOP_WAIT = 1;
	TCHAR* args[2];
	args[0] = TEXT("");
	args[1] = PAYLOAD_FILE;
	string s;

	if (PAYLOAD_FILE == nullptr) {
		s = "No file selected !";
		CA2T wt(s.c_str());
		CTegraRcmGUIDlg::SetDlgItemText(INFO_LABEL, wt);
		LOOP_WAIT = 0;
		return;
	}


	TegraRcmSmash device;
	int rc = device.SmashMain(2, args);
	

	if (rc >= 0)
	{
		s = "Payload injected !";
	}
	else
	{		
		s = "Error while injecting payload (RC=" + std::to_string(rc) + ")";
	}
	CA2T wt(s.c_str());
	CTegraRcmGUIDlg::SetDlgItemText(INFO_LABEL, wt);
	LOOP_WAIT = 0;
}

void CTegraRcmGUIDlg::OnBnClickedShofel2()
{
	LOOP_WAIT = 1;

	TCHAR szPath[_MAX_PATH];
	VERIFY(::GetModuleFileName(AfxGetApp()->m_hInstance, szPath, _MAX_PATH));
	CString csPath(szPath);
	int nIndex = csPath.ReverseFind(_T('\\'));
	if (nIndex > 0) {
		csPath = csPath.Left(nIndex);
	}
	else {
		csPath.Empty();
	}

	string s;
	CString COREBOOT_FILE = csPath + _T("\\shofel2\\coreboot\\coreboot.rom");
	CString COREBOOT = _T("CBFS+") + COREBOOT_FILE;
	CString PAYLOAD = csPath + _T("\\shofel2\\coreboot\\cbfs.bin");
	std::ifstream infile(COREBOOT_FILE);
	BOOL coreboot_exists = infile.good();
	std::ifstream infile2(PAYLOAD);
	BOOL payload_exists = infile2.good();

	if (!coreboot_exists || !payload_exists) {
		s = "Linux kernel found not found in \\shofel2 dir";
		CA2T wt(s.c_str());
		CTegraRcmGUIDlg::SetDlgItemText(INFO_LABEL, wt);

		
		CString message = _T("Kernel not found in shofel2 directory. Do you want to automatically download arch linux kernel from SoulCipher repo ?");
		const int result = MessageBox(message, _T("Kernel not found"), MB_YESNOCANCEL | MB_ICONQUESTION);
		if (result == IDYES) 
		{
			PROCESS_INFORMATION pif;
			STARTUPINFO si;
			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			CString download_script = csPath + _T("\\shofel2\\download.bat");
			BOOL bRet = CreateProcess(download_script, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pif);
		} 

		LOOP_WAIT = 0;
		return;
	}

	TCHAR* payload_f = _tcsdup(PAYLOAD);
	TCHAR* coreboot_f = _tcsdup(COREBOOT);
	TCHAR* args[5];
	args[0] = TEXT("");
	args[1] = TEXT("-w");
	args[2] = TEXT("--relocator=");
	args[3] = payload_f;
	args[4] = coreboot_f;

	TegraRcmSmash device;

	s = "Loading coreboot. Please wait.";
	CA2T wt(s.c_str());
	SetDlgItemText(INFO_LABEL, wt);

	int rc = device.SmashMain(5, args);
	if (rc >= 0 || rc < -7)
	{
		s = "Coreboot loaded. Waiting for device...";
		CA2T wt(s.c_str());
		SetDlgItemText(INFO_LABEL, wt);
		Sleep(5000);

		CString usb_loader = csPath + _T("\\shofel2\\imx_usb.bat");
		PROCESS_INFORMATION pif;
		STARTUPINFO si;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		BOOL bRet = CreateProcess(usb_loader, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pif);
		if (bRet == 0)
		{
			s = "Error while loading shofel2\\imx_usb.bat.";
		}
		else
		{
			s = "Payload injected !";
		}
	}
	else
	{
		s = "Error while injecting payload. (RC=" + std::to_string(rc) + ")";
	}
	CA2T wt2(s.c_str());
	SetDlgItemText(INFO_LABEL, wt2);

	LOOP_WAIT = 0;
}
