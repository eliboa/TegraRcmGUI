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
	CleanRegestry();
	
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

	TCHAR szPath[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, szPath)))
	{
		PathAppend(szPath, _T("\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\TegraRcmGUI.lnk"));
		std::ifstream infile(szPath);
		if (infile.good()) {
			infile.close();
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
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, szPath)))
	{
		// Remove shortcut
		PathAppend(szPath, _T("\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\TegraRcmGUI.lnk"));
		remove(CW2A(szPath));
	}

	// Create new shortcut
	if (IsCheckChecked) CreateLink();
}


void DialogTab03::CreateLink()
{
	TCHAR szAppPath[_MAX_PATH];
	VERIFY(::GetModuleFileName(AfxGetApp()->m_hInstance, szAppPath, _MAX_PATH));

	TCHAR szPath[_MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, szPath)))
	{
		PathAppend(szPath, _T("\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\TegraRcmGUI.lnk"));
	}
	CoInitializeEx(NULL, 0);
	HRESULT hres = 0;
	IShellLink* psl;
	if (SUCCEEDED(hres)) {
		hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_ALL, IID_IShellLink, (LPVOID*)&psl);
		if (SUCCEEDED(hres)) {
			IPersistFile* ppf;

			// Set the path to the shortcut target and add the description. 
			psl->SetPath(szAppPath);
			psl->SetDescription(L"TegraRcmGUI");
			psl->SetIconLocation(szAppPath, 0);

			hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
			if (SUCCEEDED(hres)) {
				hres = ppf->Save(szPath, TRUE);
				ppf->Release();
			}
			psl->Release();
		}

	}
	CoUninitialize();
}

void DialogTab03::CleanRegestry() {
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
			//Remove regestry value
			lnRes = RegOpenKeyEx(HKEY_CURRENT_USER,
				_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"),
				0L, KEY_WRITE,
				&hKey);

			if (lnRes != ERROR_FILE_NOT_FOUND)
			{
				//Remove regestry value
				lnRes = RegDeleteValueA(hKey, subkey.c_str());

				// Create new shortcut
				CreateLink();
			}
		}
	}
}