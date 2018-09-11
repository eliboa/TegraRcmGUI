/*
DialogTab01.cpp

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
#include "DialogTab01.h"
#include "afxdialogex.h"

using namespace std;


// DialogTab01 dialog

IMPLEMENT_DYNAMIC(DialogTab01, CDialog)

DialogTab01::DialogTab01(TegraRcm *pTegraRcm, CWnd* Parent /*=NULL*/)
	: CDialog(ID_DIALOGTAB_01, Parent)
{
	m_TegraRcm = pTegraRcm;
}

DialogTab01::~DialogTab01()
{
}


BOOL DialogTab01::OnInitDialog()
{
	CDialog::OnInitDialog();
	CRect rc;

	string pfile = m_TegraRcm->GetPreset("PAYLOAD_FILE");
	CString file(pfile.c_str());
	this->GetDlgItem(PAYLOAD_PATH)->SetWindowTextW(file);

	CButton* pBtn = (CButton*)GetDlgItem(IDC_BROWSE);
	pBtn->GetWindowRect(rc);
	int height = rc.Height() * 0.8;
	pBtn->ModifyStyle(0, BS_ICON);
	HICON hIcn = (HICON)LoadImage(
		AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE(ID_BROWSE_ICON),
		IMAGE_ICON,
		height, height, // use actual size
		LR_DEFAULTCOLOR
		);

	pBtn->SetIcon(hIcn);

	pBtn = (CButton*)GetDlgItem(ID_ADD_FAV);


	pBtn->GetWindowRect(rc);
	height = rc.Height() * 0.8;

	pBtn->ModifyStyle(0, BS_ICON);
	hIcn = (HICON)LoadImage(
		AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE(ID_ADD_ICON),
		IMAGE_ICON,
		height, height, // use actual size
		LR_DEFAULTCOLOR
		);
	pBtn->SetIcon(hIcn);

	pBtn = (CButton*)GetDlgItem(ID_DEL_FAV);
	pBtn->ModifyStyle(0, BS_ICON);
	hIcn = (HICON)LoadImage(
		AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE(ID_DELETE_ICON),
		IMAGE_ICON,
		height, height, // use actual size
		LR_DEFAULTCOLOR
		);
	pBtn->SetIcon(hIcn);

	PREVENT_AUTOINJECT = FALSE;

	m_TegraRcm->AppendLog("Add favorites to listbox");
	
	for (int i = 0; i < m_TegraRcm->Favorites.GetCount(); i++)
	{
		CListBox*pListBox = (CListBox*)GetDlgItem(IDC_LIST1);		
		CString fav = m_TegraRcm->Favorites[i];
		int nIndex = fav.ReverseFind(_T('\\'));
		CString csFilename, csPath, Item;
		if (nIndex > 0)
		{
			csFilename = fav.Right(fav.GetLength() - nIndex - 1);
			csPath = fav.Left(nIndex);
			Item = csFilename + _T(" (") + csPath + _T(")");
		}
		else
		{
			Item = fav;
		}
		pListBox->AddString(_tcsdup(Item));
			
		wstring wcsPath(csPath);
		string scsPath(wcsPath.begin(), wcsPath.end());
		m_TegraRcm->AppendLog("Add favorites to listbox");
		m_TegraRcm->AppendLog(scsPath);		
	}

	CFont* pFont = GetFont();
	LOGFONT lf;
	pFont->GetLogFont(&lf);
	lf.lfWeight = FW_BOLD;
	m_BoldFont.CreateFontIndirect(&lf);

	GetDlgItem(IDC_INJECT)->SendMessage(WM_SETFONT, WPARAM(HFONT(m_BoldFont)), 0);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(DialogTab01, CDialog)
	ON_BN_CLICKED(IDC_BROWSE, &DialogTab01::OnBnClickedBrowse)
	ON_BN_CLICKED(IDC_INJECT, &DialogTab01::InjectPayload)
	ON_BN_CLICKED(ID_ADD_FAV, &DialogTab01::OnBnClickedAddFav)
	ON_BN_CLICKED(ID_DEL_FAV, &DialogTab01::OnBnClickedDelFav)
	ON_LBN_DBLCLK(IDC_LIST1, &DialogTab01::OnDblclkList1)
	ON_LBN_SELCHANGE(IDC_LIST1, &DialogTab01::OnLbnSelchangeList1)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

//
// Auto-inject on change path
//
void DialogTab01::OnEnChangePath()
{
	CString file;
	GetDlgItem(PAYLOAD_PATH)->GetWindowTextW(file);
	PAYLOAD_FILE = _tcsdup(file);

	if (!PREVENT_AUTOINJECT)
	{
		// Save payload path
		CT2CA pszConvertedAnsiString(file);
		std::string file_c(pszConvertedAnsiString);
		m_TegraRcm->SetPreset("PAYLOAD_FILE", file_c);
	}

	string value = m_TegraRcm->GetPreset("AUTO_INJECT");
	TegraRcmSmash device;
	int RCM_STATUS = device.RcmStatus();
	// If Auto inject option enabled
	if (value == "TRUE" && !PREVENT_AUTOINJECT)
	{
		// Delay auto inject if RCM not detected
		if (RCM_STATUS != 0)
		{
			DELAY_AUTOINJECT = TRUE;
			m_TegraRcm->SendUserMessage("Injection scheduled. Waiting for device", VALID);
		}
		// Inject payload if RCM detected
		else {	
			InjectPayload();
		}
	}
	PREVENT_AUTOINJECT = FALSE;
}
void DialogTab01::OnBnClickedBrowse()
{
	CString szFilter;
	szFilter = "Bin files (*.bin)|*.bin|All files (*.*)|*.*||";

	CFileDialog FileOpenDialog(
		TRUE,
		NULL,
		NULL,
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
		szFilter,                       // filter 
		AfxGetMainWnd());               // the parent window  

	CString file;
	this->GetDlgItem(PAYLOAD_PATH)->GetWindowTextW(file);
	FileOpenDialog.m_ofn.lpstrInitialDir = file;

	if (FileOpenDialog.DoModal() == IDOK)
	{
		CFile File;
		VERIFY(File.Open(FileOpenDialog.GetPathName(), CFile::modeRead));
		CString Filename;
		Filename = File.GetFilePath();
		this->GetDlgItem(PAYLOAD_PATH)->SetWindowTextW(Filename);
		OnEnChangePath();
	}
}
void DialogTab01::InjectPayload()
{
	CString file;
	this->GetDlgItem(PAYLOAD_PATH)->GetWindowTextW(file);
	PAYLOAD_FILE = _tcsdup(file);

	if (PAYLOAD_FILE == nullptr) {
		m_TegraRcm->BitmapDisplay(LOAD_ERROR);
		m_TegraRcm->SendUserMessage("No file selected", INVALID);
		return;
	}

	std::ifstream infile(file);
	if (!infile.good())
	{
		m_TegraRcm->SendUserMessage("File doesn't exist", INVALID);
		return;
	}

	m_TegraRcm->BitmapDisplay(LOADING);
	GetParent()->UpdateWindow();

	TCHAR cmd[MAX_PATH] = TEXT("\"");
	lstrcat(cmd, PAYLOAD_FILE);
	lstrcat(cmd, TEXT("\""));
	
	m_TegraRcm->SendUserMessage("Injecting payload...");
	int rc = m_TegraRcm->Smasher(cmd);
	if (rc >= 0)
	{
		m_TegraRcm->BitmapDisplay(LOADED);
		m_TegraRcm->SendUserMessage("Payload injected !", VALID);
		m_TegraRcm->WAITING_RECONNECT = TRUE;
		if (!m_TegraRcm->CmdShow) m_TegraRcm->ShowTrayIconBalloon(TEXT("Payload injected"), TEXT(" "), 1000, NIIF_INFO);
	}
	else
	{
		m_TegraRcm->BitmapDisplay(LOAD_ERROR);
		string s = "Error while injecting payload (RC=" + std::to_string(rc) + ")";
		if (!m_TegraRcm->CmdShow) m_TegraRcm->ShowTrayIconBalloon(TEXT("Error"), TEXT("Error while injecting payload"), 1000, NIIF_ERROR);
		m_TegraRcm->SendUserMessage(s.c_str(), INVALID);
		
	}
}


void DialogTab01::OnBnClickedAddFav()
{

	CString csPath;
	GetDlgItem(PAYLOAD_PATH)->GetWindowTextW(csPath);
	std::ifstream infile(csPath);
	if (!infile.good())
	{
		m_TegraRcm->SendUserMessage("File doesn't exist", INVALID);
		return;
	}

	CListBox*pListBox = (CListBox*) GetDlgItem(IDC_LIST1);
	CString  csPathf(csPath), csFilename, Item;
	int nIndex = csPathf.ReverseFind(_T('\\'));
	if (nIndex > 0) csFilename = csPathf.Right(csPathf.GetLength() - nIndex - 1);
	else return;

	for (int i = 0; i < m_TegraRcm->Favorites.GetCount(); i++)
	{
		if (m_TegraRcm->Favorites[i] == csPathf)
		{
			m_TegraRcm->SendUserMessage("Favorite already exists", INVALID);
			return;
		}
	}

	csPath = csPathf.Left(nIndex);
	Item = csFilename + _T(" (") + csPath + _T(")");
	pListBox->AddString(_tcsdup(Item));
	m_ListBox.Add(csPathf);
	m_TegraRcm->Favorites.Add(csPathf);
	m_TegraRcm->SaveFavorites();

	m_TegraRcm->SendUserMessage("Favorite added", VALID);
	return;
}


void DialogTab01::OnBnClickedDelFav()
{

	CListBox*pListBox = (CListBox*)GetDlgItem(IDC_LIST1);
	int i = pListBox->GetCurSel();
	if (i >= 0)
	{
		pListBox->DeleteString(i);
		m_TegraRcm->Favorites.RemoveAt(i);
		m_TegraRcm->SaveFavorites();
		m_TegraRcm->SendUserMessage("Favorite removed", VALID);
	}
	return;
}


void DialogTab01::OnDblclkList1()
{
	if (m_SelectedItem >= 0)
	{
		CString fav = m_TegraRcm->Favorites.GetAt(m_SelectedItem);
		GetDlgItem(PAYLOAD_PATH)->SetWindowTextW(fav);
		GetDlgItem(PAYLOAD_PATH)->GetFocus();
		if (m_TegraRcm->GetPreset("AUTO_INJECT") != "TRUE" && m_TegraRcm->GetRcmStatus() == 0)
		{
			InjectPayload();
		}
		else
		{
			OnEnChangePath();
		}
	}
}


void DialogTab01::OnLbnSelchangeList1()
{
	CListBox*pListBox = (CListBox*)GetDlgItem(IDC_LIST1);
	int i = pListBox->GetCurSel();
	if (i >= 0)
	{
		m_SelectedItem = i;
	}
}

HBRUSH DialogTab01::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (pWnd->GetDlgCtrlID())
	{
	case ID_SELECT_PAYLOAD:
	case ID_FAVORITES_TITLE:
		pDC->SelectObject(&m_BoldFont);
		break;
	}
	return hbr;
}
