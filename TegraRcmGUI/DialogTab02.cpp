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
#include <stdlib.h>
#include <codecvt>

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
ON_BN_CLICKED(IDC_DUMP_BISKEY, &DialogTab02::OnBnClickedDumpBiskey)
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

	pBtn = (CButton*)GetDlgItem(IDC_DUMP_BISKEY);
	pBtn->ModifyStyle(0, BS_ICON);
	hIcn = (HICON)LoadImage(
		AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE(ID_KEYS_ICON),
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

	CComboBox* pmyComboBox = (CComboBox*)GetDlgItem(ID_UMS_COMBO);
	pmyComboBox->AddString(TEXT("eMMC BOOT0 (DANGEROUS)"));
	pmyComboBox->AddString(TEXT("eMMC BOOT1 (DANGEROUS)"));
	pmyComboBox->AddString(TEXT("eMMC rawNAND (DANGEROUS)"));
	pmyComboBox->AddString(TEXT("MMC - SD Card"));
	pmyComboBox->SetCurSel(3);


	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void DialogTab02::OnBnClickedMountSd()
{
	CComboBox* pmyComboBox = (CComboBox*)GetDlgItem(ID_UMS_COMBO);
	if (pmyComboBox->GetCurSel() < 3) {
		CString message = _T("-----> WARNING <-----\nYou are about to mount internal storage of your Nintendo Switch\nBE VERY CAREFUL ! Do not format or write to your NAND partitions if you don't know what you're doing.\nTHIS COULD BRICK YOUR CONSOLE !!!\n\nAre you really sure you want to continue ?");
		const int result = MessageBox(message, _T("BEWARE & WARNING"), MB_YESNO | MB_ICONWARNING);
		if (result != IDYES) {
			m_TegraRcm->AppendLogBox(TEXT("Mount NAND partition ABORTED\r\n"));
			return;
		}
	}

	m_TegraRcm->BitmapDisplay(LOADING);
	GetParent()->UpdateWindow();
	string s;
	
	
	TCHAR args[256];
	switch (pmyComboBox->GetCurSel())
	{
	case 0:
		_tcscpy(args, TEXT(".\\tools\\memloader\\memloader_usb.bin -r --dataini=.\\tools\\memloader\\ums_boot0.ini"));
		break;
	case 1:
		_tcscpy(args, TEXT(".\\tools\\memloader\\memloader_usb.bin -r --dataini=.\\tools\\memloader\\ums_boot1.ini"));
		break;
	case 2:
		_tcscpy(args, TEXT(".\\tools\\memloader\\memloader_usb.bin -r --dataini=.\\tools\\memloader\\ums_emmc.ini"));
		break;
	default:
		_tcscpy(args, TEXT(".\\tools\\memloader\\memloader_usb.bin -r --dataini=.\\tools\\memloader\\ums_sd.ini"));
		break;
	}
	
	int rc = m_TegraRcm->Smasher(args, FALSE);
	if (rc < -10)
	{
		m_TegraRcm->BitmapDisplay(LOAD_ERROR);
		s = "Error while injecting UMS Tool (RC=" + std::to_string(rc) + ")";
		if (!m_TegraRcm->CmdShow) m_TegraRcm->ShowTrayIconBalloon(TEXT("Error"), TEXT("Error while injecting payload"), 1000, NIIF_ERROR);
	}
	else
	{
		m_TegraRcm->BitmapDisplay(LOADED);
		s = "UMS Tool injected";
		if (!m_TegraRcm->CmdShow) m_TegraRcm->ShowTrayIconBalloon(TEXT("UMS Tool injected"), TEXT(" "), 1000, NIIF_INFO);
	}
	//CA2T wt(s.c_str());
	//GetParent()->SetDlgItemText(INFO_LABEL, wt);
	CString ss(s.c_str());
	m_TegraRcm->AppendLogBox(ss + TEXT("\r\n"));
}


void DialogTab02::OnBnClickedShofel2()
{
	TCHAR *exe_dir = m_TegraRcm->GetAbsolutePath(TEXT(""), CSIDL_APPDATA);

	string s;
	TCHAR *COREBOOT_FILE = m_TegraRcm->GetAbsolutePath(TEXT("tools\\shofel2\\coreboot\\coreboot.rom"), CSIDL_APPDATA);
	TCHAR *PAYLOAD = m_TegraRcm->GetAbsolutePath(TEXT("tools\\shofel2\\coreboot\\cbfs.bin"), CSIDL_APPDATA);
	CString COREBOOT_FILE2 = COREBOOT_FILE;
	CString COREBOOT = _T("CBFS+") + COREBOOT_FILE2;

	std::ifstream infile(COREBOOT_FILE);
	BOOL coreboot_exists = infile.good();
	std::ifstream infile2(PAYLOAD);
	BOOL payload_exists = infile2.good();

	if (!coreboot_exists || !payload_exists) {
		//GetParent()->SetDlgItemText(INFO_LABEL, TEXT("Linux coreboot not found in \\shofel2 dir"));
		m_TegraRcm->AppendLogBox(TEXT("Linux coreboot not found in \\shofel2 di\r\n"));

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
	//GetParent()->SetDlgItemText(INFO_LABEL, TEXT("Loading coreboot. Please wait."));
	m_TegraRcm->AppendLogBox(TEXT("Linux coreboot not found in \\shofel2 di\r\n"));


	//int rc = device.SmashMain(5, args);
	TCHAR cmd[4096] = TEXT("--relocator= \"");
	lstrcat(cmd, _tcsdup(PAYLOAD));
	lstrcat(cmd, TEXT("\" \"CBFS:"));
	lstrcat(cmd, _tcsdup(COREBOOT_FILE));
	lstrcat(cmd, TEXT("\""));
	int rc = m_TegraRcm->Smasher(cmd, FALSE);
	int test = 1;
	if (rc >= 0 || rc < -7)
	{
		//GetParent()->SetDlgItemText(INFO_LABEL, TEXT("Coreboot injected. Waiting 5s for device..."));
		m_TegraRcm->AppendLogBox(TEXT("Coreboot injected. Waiting 5s for device...\r\n"));
		Sleep(5000);

		PROCESS_INFORMATION pif;
		STARTUPINFO si;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		TCHAR *imx_script = m_TegraRcm->GetAbsolutePath(TEXT("tools\\shofel2\\imx_usb.bat"), CSIDL_APPDATA);
		//GetParent()->SetDlgItemText(INFO_LABEL, TEXT("Loading coreboot... Please wait."));
		m_TegraRcm->AppendLogBox(TEXT("Loading coreboot... Please wait\r\n"));

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
	//CA2T wt2(s.c_str());
	//GetParent()->SetDlgItemText(INFO_LABEL, wt2);
	CString ss(s.c_str());
	m_TegraRcm->AppendLogBox(ss + TEXT("\r\n"));

}

HBRUSH DialogTab02::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (pWnd->GetDlgCtrlID())
	{
		case ID_UMSTOOL_TITLE:
		case ID_LINUX_TITLE:
		case ID_BISKEY_TITLE:
			pDC->SelectObject(&m_BoldFont); 
			break;
	}
	return hbr;
}


void DialogTab02::OnBnClickedDumpBiskey()
{


	m_TegraRcm->BitmapDisplay(LOADING);
	GetParent()->UpdateWindow();
	TCHAR args[] = TEXT("-w tools\\biskeydump_usb.bin BOOT:0x0");
	int rc = m_TegraRcm->Smasher(args);

	BOOL keyFound = FALSE;
	TCHAR *rfile = m_TegraRcm->GetAbsolutePath(TEXT("out.log"), CSIDL_APPDATA);
	CString Cline;
	std::wifstream fin(rfile, std::ios::binary);
	fin.imbue(std::locale(fin.getloc(), new std::codecvt_utf8_utf16<wchar_t>));
	CString Filename;
	for (wchar_t c; fin.get(c); ) {
		CString Cchar(c);
		if (Cchar == TEXT("\n")) {
			
			if (Cline.Find(TEXT("HWI")) != -1 ||
				Cline.Find(TEXT("SBK")) != -1 ||
				Cline.Find(TEXT("TSEC KEY")) != -1 ||
				Cline.Find(TEXT("BIS KEY")) != -1) {

				if (!keyFound)
				{
					keyFound = TRUE;

					CString szFilter;
					szFilter = "TXT files (*.txt)|*.txt|All files (*.*)|*.*||";

					CFileDialog FileOpenDialog(
						FALSE,
						NULL,
						TEXT("BIS_keys.txt"),
						OFN_HIDEREADONLY,
						szFilter,                       
						AfxGetMainWnd());               

					if (FileOpenDialog.DoModal() == IDOK)
					{
						CFile File;
						Filename = FileOpenDialog.GetPathName();
						remove(CT2A(Filename));
					}
					else {
						return;
					}

				}
				CT2CA pszConvertedAnsiString(Cline + _T('\n'));
				std::string outLine = pszConvertedAnsiString;
				fstream outFile;
				outFile.open(Filename, fstream::in | fstream::out | fstream::app);
				outFile << outLine;
				outFile.close();

			}
			Cline.Empty();
		}
		else if (Cchar != TEXT("\r") && Cchar != TEXT("")) {
			Cline.Append(Cchar);
		}
	}
	fin.close();


	CString s;
	if (!keyFound)
	{
		m_TegraRcm->BitmapDisplay(LOAD_ERROR);		
		s.Append(TEXT("Error while retrieving BIS keys"));
		if (!m_TegraRcm->CmdShow) m_TegraRcm->ShowTrayIconBalloon(TEXT("Error"), s, 1000, NIIF_ERROR);
		s.Append(TEXT("\r\n"));
	}
	else
	{
		m_TegraRcm->BitmapDisplay(LOADED);
		CString loc(Filename);
		s.Append(TEXT("BIS keys saved to : "));
		s.Append(loc);
		if (!m_TegraRcm->CmdShow) m_TegraRcm->ShowTrayIconBalloon(s, TEXT(" "), 1000, NIIF_INFO);
		s.Append(TEXT("\r\n"));

	}
	m_TegraRcm->AppendLogBox(s);
}
