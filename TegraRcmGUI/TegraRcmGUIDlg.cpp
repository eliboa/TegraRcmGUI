/*
TegraRcmGUIDlg.cpp

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

#include "stdafx.h"
#include "afxdialogex.h"
#include "TegraRcmGUI.h"
#include "TegraRcmGUIDlg.h"
#include <windows.h>
#include <shellapi.h>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ARRAY_SIZE 1024

CString csPath;
CString csPath2;
ULONGLONG GetDllVersion(LPCTSTR lpszDllName);

//
// dialog
//
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
	DDX_Control(pDX, IDC_TAB_CONTROL, m_tbCtrl);
}
BEGIN_MESSAGE_MAP(CTegraRcmGUIDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SHOWWINDOW()
	ON_WM_CLOSE()
	ON_MESSAGE(WM_SYSICON, OnTrayIconEvent)
	ON_COMMAND(SWM_EXIT, OnClose)
	ON_COMMAND(SWM_SHOW, ShowWindowCommand)
	ON_COMMAND(SWM_HIDE, HideWindowCommand)
	ON_COMMAND(SWM_INJECT, InjectCommand)
	ON_COMMAND(SWM_BROWSE, BrowseCommand)
	ON_COMMAND(SWM_LINUX, LinuxCommand)
	ON_COMMAND(SWM_MOUNT, MountCommand)
	ON_COMMAND(SWM_FAV01, InjectFav01Command)
	ON_COMMAND(SWM_FAV02, InjectFav02Command)
	ON_COMMAND(SWM_FAV03, InjectFav03Command)
	ON_COMMAND(SWM_FAV04, InjectFav04Command)
	ON_COMMAND(SWM_FAV05, InjectFav05Command)
	ON_COMMAND(SWM_FAV06, InjectFav06Command)
	ON_COMMAND(SWM_FAV07, InjectFav07Command)
	ON_COMMAND(SWM_FAV08, InjectFav08Command)
	ON_COMMAND(SWM_FAV09, InjectFav09Command)
	ON_COMMAND(SWM_FAV10, InjectFav10Command)
	ON_COMMAND(SWM_AUTOINJECT, AutoInjectCommand)
	ON_MESSAGE(WM_QUERYENDSESSION, OnQueryEndSession)
END_MESSAGE_MAP()

//
// message handlers
//
BOOL CTegraRcmGUIDlg::OnInitDialog()
{

	m_TegraRcm = new TegraRcm(this);
	m_TegraRcm->AppendLog("new TegraRcm()");

	CDialog::OnInitDialog();

	// Accessibility
	EnableActiveAccessibility();

	// Get current directory
	TCHAR szPath[_MAX_PATH];
	VERIFY(::GetModuleFileName(AfxGetApp()->m_hInstance, szPath, _MAX_PATH));
	CString csPathf(szPath);
	CT2CA pszConvertedAnsiString(csPathf);
	std::string strStd(pszConvertedAnsiString);
	m_TegraRcm->AppendLog("Module filename is : ");
	m_TegraRcm->AppendLog(strStd);

	int nIndex = csPathf.ReverseFind(_T('\\'));
	if (nIndex > 0) csPath = csPathf.Left(nIndex);
	else csPath.Empty();

	// Initialize bitmap
	CRect rc;
	AfxGetMainWnd()->GetWindowRect(rc);
	int width = rc.Width();
	int fontSize = width * 0.031;
	if (width < 400)
	{
		RCM_BITMAP0.SetBitmap(INIT_LOGO_2);
		RCM_BITMAP1.SetBitmap(RCM_NOT_DETECTED_2);
		RCM_BITMAP2.SetBitmap(DRIVER_KO_2);
		RCM_BITMAP3.SetBitmap(RCM_DETECTED_2);
		RCM_BITMAP4.SetBitmap(LOADING_2);
		RCM_BITMAP5.SetBitmap(LOADED_2);
		RCM_BITMAP6.SetBitmap(LOAD_ERROR_2);
	}
	else
	{
		RCM_BITMAP0.SetBitmap(INIT_LOGO);
		RCM_BITMAP1.SetBitmap(RCM_NOT_DETECTED);
		RCM_BITMAP2.SetBitmap(DRIVER_KO);
		RCM_BITMAP3.SetBitmap(RCM_DETECTED);
		RCM_BITMAP4.SetBitmap(LOADING);
		RCM_BITMAP5.SetBitmap(LOADED);
		RCM_BITMAP6.SetBitmap(LOAD_ERROR);
	}

	// Log Box
	LOGFONT lf;
	/*
	CEdit* pBox = (CEdit*)AfxGetMainWnd()->GetDlgItem(IDC_LOG_BOX);	
	CFont* old = pBox->GetFont();
	old->GetLogFont(&lf);
	CFont newfont;
	newfont.CreateFont(lf.lfHeight + 30, 0, lf.lfEscapement, lf.lfOrientation, lf.lfWeight, lf.lfItalic, lf.lfUnderline, lf.lfStrikeOut, lf.lfCharSet, lf.lfOutPrecision, lf.lfClipPrecision, lf.lfQuality, lf.lfPitchAndFamily, lf.lfFaceName);
	pBox->SetFont(&newfont);
	*/

	CEdit* pBox = (CEdit*)AfxGetMainWnd()->GetDlgItem(IDC_LOG_BOX);

	CFont *myFont = new CFont();
	myFont->CreateFont(fontSize, 0, 0, 0, FW_NORMAL, false, false,
		0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FIXED_PITCH | FF_MODERN, _T("Verdana"));

	pBox->SetFont(myFont);
	

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

	// Kill other running process of app
	m_TegraRcm->KillRunningProcess(TEXT("TegraRcmGUI.exe"));
	
	m_tbCtrl.InitDialogs(m_TegraRcm);
	
	TCITEM tcItem1;
	tcItem1.mask = TCIF_TEXT;
	tcItem1.pszText = _T("Payload");
	m_tbCtrl.InsertItem(0, &tcItem1);

	TCITEM tcItem2;
	tcItem2.mask = TCIF_TEXT;
	tcItem2.pszText = _T("Tools");
	m_tbCtrl.InsertItem(1, &tcItem2);

	TCITEM tcItem3;
	tcItem3.mask = TCIF_TEXT;
	tcItem3.pszText = _T("Settings");

	m_tbCtrl.InsertItem(2, &tcItem3);
	m_tbCtrl.ActivateTabDialogs();

	m_TegraRcm->InitCtrltbDlgs(m_tbCtrl.m_Dialog[0], m_tbCtrl.m_Dialog[1], m_tbCtrl.m_Dialog[2]);
	
	m_TegraRcm->BitmapDisplay(INIT_LOGO);

	// Tray icon
	m_TegraRcm->CreateTrayIcon();
	m_TegraRcm->SetTrayIconTipText(TEXT("TegraRcmGUI"));

	// Start timer to check RCM status every second
	CTegraRcmGUIDlg::StartTimer();	



	return TRUE;
}
void CTegraRcmGUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);
}
void CTegraRcmGUIDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	//CmdShow = bShow;
	//CDialog::OnShowWindow(bShow, nStatus);
}
HBRUSH CTegraRcmGUIDlg::OnCtlColor(CDC* pDC, CWnd *pWnd, UINT nCtlColor)
{
	switch (nCtlColor)
	{
	case CTLCOLOR_STATIC:
		/*
		if (GetDlgItem(IDC_RAJKOSTO)->GetSafeHwnd() == pWnd->GetSafeHwnd() || GetDlgItem(SEPARATOR)->GetSafeHwnd() == pWnd->GetSafeHwnd())
		{
			pDC->SetTextColor(RGB(192, 192, 192));
			pDC->SetBkMode(TRANSPARENT);
			return (HBRUSH)GetStockObject(NULL_BRUSH);
		}
		*/
		if (GetDlgItem(INFO_LABEL)->GetSafeHwnd() == pWnd->GetSafeHwnd())
		{
			pDC->SetBkMode(TRANSPARENT);
			pDC->SetTextColor(m_TegraRcm->LabelColor);
			return (HBRUSH) CreateSolidBrush( WhiteRGB );
		}
		if (GetDlgItem(IDC_LOG_BOX)->GetSafeHwnd() == pWnd->GetSafeHwnd())
		{
			pDC->SetBkMode(TRANSPARENT);
			return (HBRUSH)CreateSolidBrush(WhiteRGB);
		}
		if (GetDlgItem(IDC_STATUS_BG)->GetSafeHwnd() == pWnd->GetSafeHwnd())
		{
			return (HBRUSH)CreateSolidBrush(WhiteRGB);
		}
		if (GetDlgItem(RCM_BITMAP)->GetSafeHwnd() == pWnd->GetSafeHwnd())
		{
			return (HBRUSH)CreateSolidBrush(WhiteRGB);
		}
	default:
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
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
HCURSOR CTegraRcmGUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
void CTegraRcmGUIDlg::OnSize(UINT nType, int cx, int cy)
{
	if (nType == SIZE_MINIMIZED)
	{
		if (m_TegraRcm->MIN_TO_TRAY_CURR)
		{
			ShowWindow(SW_HIDE);
		}
		if (m_TegraRcm != NULL)
		{
			m_TegraRcm->CmdShow = FALSE;
		}
	}
	else
	{
		if (m_TegraRcm != NULL) m_TegraRcm->CmdShow = TRUE;
		//ShowWindow(SW_RESTORE);
	}
}
void CTegraRcmGUIDlg::OnClose()
{
	m_TegraRcm->DestroyTrayIcon();
	PostQuitMessage(0);
}

LRESULT CTegraRcmGUIDlg::OnTrayIconEvent(WPARAM wParam, LPARAM lParam)
{
	return m_TegraRcm->OnTrayIconEvent(wParam, lParam);
}
void CTegraRcmGUIDlg::ShowWindowCommand()
{
	ShowWindow(SW_RESTORE);
	SetForegroundWindow();
	SetFocus();
	SetActiveWindow();
	if (m_TegraRcm != NULL)
	{
		m_TegraRcm->CmdShow = TRUE;
	}
}
void CTegraRcmGUIDlg::HideWindowCommand()
{
	ShowWindow(SW_HIDE);
	if (m_TegraRcm != NULL)
	{
		m_TegraRcm->CmdShow = FALSE;
	}
}
void CTegraRcmGUIDlg::InjectCommand()
{
	if (m_TegraRcm != NULL)
	{
		DialogTab01 *pt = (DialogTab01*) m_TegraRcm->m_Ctrltb1;
		pt->InjectPayload();
	}
}
void CTegraRcmGUIDlg::BrowseCommand()
{
	if (m_TegraRcm != NULL)
	{
		DialogTab01 *pt = (DialogTab01*)m_TegraRcm->m_Ctrltb1;
		pt->OnBnClickedBrowse();
	}
}
void CTegraRcmGUIDlg::LinuxCommand()
{
	if (m_TegraRcm != NULL)
	{
		DialogTab02 *pt = (DialogTab02*)m_TegraRcm->m_Ctrltb2;
		pt->OnBnClickedShofel2();
	}
}
void CTegraRcmGUIDlg::MountCommand()
{
	if (m_TegraRcm != NULL)
	{
		DialogTab02 *pt = (DialogTab02*)m_TegraRcm->m_Ctrltb2;
		CComboBox* pmyComboBox = (CComboBox*)pt->GetDlgItem(ID_UMS_COMBO);
		pmyComboBox->SetCurSel(3);
		pt->OnBnClickedMountSd();
	}
}
void CTegraRcmGUIDlg::InjectFav01Command() { InjectFavCommand(0); }
void CTegraRcmGUIDlg::InjectFav02Command() { InjectFavCommand(1); }
void CTegraRcmGUIDlg::InjectFav03Command() { InjectFavCommand(2); }
void CTegraRcmGUIDlg::InjectFav04Command() { InjectFavCommand(3); }
void CTegraRcmGUIDlg::InjectFav05Command() { InjectFavCommand(4); }
void CTegraRcmGUIDlg::InjectFav06Command() { InjectFavCommand(5); }
void CTegraRcmGUIDlg::InjectFav07Command() { InjectFavCommand(6); }
void CTegraRcmGUIDlg::InjectFav08Command() { InjectFavCommand(7); }
void CTegraRcmGUIDlg::InjectFav09Command() { InjectFavCommand(8); }
void CTegraRcmGUIDlg::InjectFav10Command() { InjectFavCommand(9); }
void CTegraRcmGUIDlg::InjectFavCommand(int i)
{
	if (m_TegraRcm != NULL)
	{
		DialogTab01 *pt = (DialogTab01*)m_TegraRcm->m_Ctrltb1;
		CString fav = m_TegraRcm->Favorites.GetAt(i);
		pt->GetDlgItem(PAYLOAD_PATH)->SetWindowTextW(fav);
		pt->GetDlgItem(PAYLOAD_PATH)->GetFocus();
		pt->InjectPayload();
	}
}

void CTegraRcmGUIDlg::AutoInjectCommand()
{
	if (m_TegraRcm != NULL)
	{
		DialogTab03 *pt = (DialogTab03*)m_TegraRcm->m_Ctrltb3;
		if (m_TegraRcm->AUTOINJECT_CURR)
		{
			m_TegraRcm->AUTOINJECT_CURR = FALSE;
			m_TegraRcm->SetPreset("AUTO_INJECT", "FALSE");
			m_TegraRcm->DELAY_AUTOINJECT = FALSE;
			CButton *m_ctlCheck = (CButton*)pt->GetDlgItem(AUTO_INJECT);
			m_ctlCheck->SetCheck(BST_UNCHECKED);
		}
		else
		{
			m_TegraRcm->AUTOINJECT_CURR = TRUE;
			m_TegraRcm->SetPreset("AUTO_INJECT", "TRUE");
			m_TegraRcm->DELAY_AUTOINJECT = TRUE;
			CButton *m_ctlCheck = (CButton*)pt->GetDlgItem(AUTO_INJECT);
			m_ctlCheck->SetCheck(BST_CHECKED);
		}
		AfxGetMainWnd()->UpdateWindow();
		pt->OnClickedAutoInject();
	}
}


void CTegraRcmGUIDlg::StartTimer()
{
	// Set timer for Minutes.
	//SetTimer(ID_TIMER_MINUTE, 60 * 1000, 0);
	// Set timer for Seconds.
	SetTimer(ID_TIMER_SECONDS, 1000, 0);
}
void CTegraRcmGUIDlg::StopTimer()
{
	// Stop both timers.
	KillTimer(ID_TIMER_MINUTE);
	KillTimer(ID_TIMER_SECONDS);
}
void CTegraRcmGUIDlg::OnTimer(UINT_PTR nIDEvent)
{
	// Each second
	if (nIDEvent == ID_TIMER_SECONDS)
	{
		m_TegraRcm->LookUp();		
	}

	// After first initDialog, hide window if min2tray option is enabled
	if (AUTOSTART)
	{
		if (m_TegraRcm->MIN_TO_TRAY_CURR) HideWindowCommand();
		AUTOSTART = FALSE;
	}
}

LRESULT CTegraRcmGUIDlg::OnQueryEndSession(WPARAM wParm, LPARAM lParm)
{
	// This is not useful, exit is not needed in that case
	m_TegraRcm->AppendLog("OnEndSession");
	PostMessage(WM_QUIT);
	return TRUE;
}
