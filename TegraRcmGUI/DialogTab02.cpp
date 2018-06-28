/*
DialogTab02.cpp

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
#include "afxdialogex.h"
#include "DialogTab02.h"


using namespace std;

// DialogTab02 dialog

IMPLEMENT_DYNAMIC(DialogTab02, CDialogEx)

DialogTab02::DialogTab02(TegraRcm *pTegraRcm, CWnd* Parent /*=NULL*/)
	: CDialogEx(ID_DIALOGTAB_02, Parent)
{
	m_TegraRcm = pTegraRcm;
}

DialogTab02::~DialogTab02()
{
}

void DialogTab02::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(DialogTab02, CDialogEx)
	ON_BN_CLICKED(IDC_MOUNT_SD, &DialogTab02::OnBnClickedMountSd)
	ON_BN_CLICKED(IDC_SHOFEL2, &DialogTab02::OnBnClickedShofel2)
//	ON_WM_CTLCOLOR()
ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// DialogTab02 message handlers

BOOL DialogTab02::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CRect rc;

	CButton* pBtn = (CButton*)GetDlgItem(IDC_MOUNT_SD);
	pBtn->GetWindowRect(rc);
	int height = rc.Height() * 0.8;
	pBtn->ModifyStyle(0, BS_ICON);
	HICON hIcn = (HICON)LoadImage(
		AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE(ID_UMSTOOL_ICON),
		IMAGE_ICON,
		height, height, // use actual size
		LR_DEFAULTCOLOR
		);
	pBtn->SetIcon(hIcn);

	pBtn = (CButton*)GetDlgItem(IDC_SHOFEL2);
	pBtn->ModifyStyle(0, BS_ICON);
	hIcn = (HICON)LoadImage(
		AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE(ID_LINUX_ICON),
		IMAGE_ICON,
		height, height, // use actual size
		LR_DEFAULTCOLOR
		);
	pBtn->SetIcon(hIcn);


	CFont* pFont = GetFont();
	LOGFONT lf;
	pFont->GetLogFont(&lf);
	lf.lfWeight = FW_BOLD;
	m_BoldFont.CreateFontIndirect(&lf);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void DialogTab02::OnBnClickedMountSd()
{
	m_TegraRcm->BitmapDisplay(LOADING);
	GetParent()->UpdateWindow();
	string s;
	TCHAR args[] = TEXT("memloader\\memloader_usb.bin -r --dataini=memloader\\ums_sd.ini");
	int rc = m_TegraRcm->Smasher(args);
	if (rc < 0)
	{
		m_TegraRcm->BitmapDisplay(LOAD_ERROR);
		s = "Error while injecting payload (RC=" + std::to_string(rc) + ")";
		if (!m_TegraRcm->CmdShow) m_TegraRcm->ShowTrayIconBalloon(TEXT("Error"), TEXT("Error while injecting payload"), 1000, NIIF_ERROR);
	}
	else
	{
		m_TegraRcm->BitmapDisplay(LOADED);
		s = "UMS Tool injected";
		if (!m_TegraRcm->CmdShow) m_TegraRcm->ShowTrayIconBalloon(TEXT("UMS Tool injected"), TEXT(" "), 1000, NIIF_INFO);
	}
	CA2T wt(s.c_str());
	GetParent()->SetDlgItemText(INFO_LABEL, wt);
}


void DialogTab02::OnBnClickedShofel2()
{
	TCHAR *exe_dir = m_TegraRcm->GetAbsolutePath(TEXT(""), CSIDL_APPDATA);

	string s;
	TCHAR *COREBOOT_FILE = m_TegraRcm->GetAbsolutePath(TEXT("shofel2\\coreboot\\coreboot.rom"), CSIDL_APPDATA);
	TCHAR *PAYLOAD = m_TegraRcm->GetAbsolutePath(TEXT("shofel2\\coreboot\\cbfs.bin"), CSIDL_APPDATA);
	CString COREBOOT_FILE2 = COREBOOT_FILE;
	CString COREBOOT = _T("CBFS+") + COREBOOT_FILE2;

	std::ifstream infile(COREBOOT_FILE);
	BOOL coreboot_exists = infile.good();
	std::ifstream infile2(PAYLOAD);
	BOOL payload_exists = infile2.good();

	if (!coreboot_exists || !payload_exists) {
		GetParent()->SetDlgItemText(INFO_LABEL, TEXT("Linux coreboot not found in \\shofel2 dir"));
		CString message = _T("Kernel not found in shofel2 directory. Do you want to automatically download arch linux kernel from SoulCipher repo ?");
		const int result = MessageBox(message, _T("Kernel not found"), MB_YESNOCANCEL | MB_ICONQUESTION);
		if (result == IDYES)
		{
			PROCESS_INFORMATION pif;
			STARTUPINFO si;
			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			TCHAR *download_script = m_TegraRcm->GetAbsolutePath(TEXT("shofel2\\download.bat"), CSIDL_APPDATA);
			BOOL bRet = CreateProcess(download_script, NULL, NULL, NULL, FALSE, 0, NULL, exe_dir, &si, &pif);
		}
		return; // TO-DO : Remove return for coreboot injection after download
	}
	m_TegraRcm->BitmapDisplay(LOADING);
	GetParent()->UpdateWindow();
	GetParent()->SetDlgItemText(INFO_LABEL, TEXT("Loading coreboot. Please wait."));

	//int rc = device.SmashMain(5, args);
	TCHAR cmd[MAX_PATH] = TEXT("--relocator= \"");
	lstrcat(cmd, _tcsdup(PAYLOAD));
	lstrcat(cmd, TEXT("\" \"CBFS:"));
	lstrcat(cmd, _tcsdup(COREBOOT_FILE));
	lstrcat(cmd, TEXT("\""));
	int rc = m_TegraRcm->Smasher(cmd);

	if (rc >= 0 || rc < -7)
	{
		GetParent()->SetDlgItemText(INFO_LABEL, TEXT("Coreboot injected. Waiting 5s for device..."));
		Sleep(5000);

		PROCESS_INFORMATION pif;
		STARTUPINFO si;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		TCHAR *imx_script = m_TegraRcm->GetAbsolutePath(TEXT("shofel2\\imx_usb.bat"), CSIDL_APPDATA);
		GetParent()->SetDlgItemText(INFO_LABEL, TEXT("Loading coreboot... Please wait."));
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
			m_TegraRcm->BitmapDisplay(LOADED);
			s = "\nCoreboot loaded !";
			if (!m_TegraRcm->CmdShow) m_TegraRcm->ShowTrayIconBalloon(TEXT("Coreboot loaded "), TEXT(" "), 1000, NIIF_INFO);
		}
		else
		{
			m_TegraRcm->BitmapDisplay(LOAD_ERROR);
			s = "Error while loading imx_usb.exe";
			if (!m_TegraRcm->CmdShow) m_TegraRcm->ShowTrayIconBalloon(TEXT("Error"), TEXT("Error while loading imx_usb.exe"), 1000, NIIF_ERROR);
		}
	}
	else
	{
		s = "Error while injecting payload. (RC=" + std::to_string(rc) + ")";
		if (!m_TegraRcm->CmdShow) m_TegraRcm->ShowTrayIconBalloon(TEXT("Error"), TEXT("Error while injecting payload"), 1000, NIIF_ERROR);
	}
	CA2T wt2(s.c_str());
	GetParent()->SetDlgItemText(INFO_LABEL, wt2);
}

HBRUSH DialogTab02::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (pWnd->GetDlgCtrlID())
	{
		case ID_UMSTOOL_TITLE:
		case ID_LINUX_TITLE:
			pDC->SelectObject(&m_BoldFont); 
			break;
	}
	return hbr;
}
