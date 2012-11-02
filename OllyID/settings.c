#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include "plugin.h"
#include "OllyID.h"
#include "settings.h"
#include "resource.h"

/* Globals Definitions */
extern wchar_t database_path[MAX_PATH];
extern int scan_on_analysis;
extern int scan_on_mod_load;
extern int scan_ep_only;

INT_PTR CALLBACK settings_dialog_procedure(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) 
	{ 
	case WM_COMMAND:
	{
		switch (LOWORD(wParam)) 
		{
			case IDC_OK:
			{
				/*
				 * Save the settings
				 */

				write_settings_to_ini(hDlg);

				EndDialog(hDlg, 0);
				return TRUE;
			}

			case IDC_CANCEL:
			{
				/* End dialog without saving anything */
				EndDialog(hDlg, 0);
				return TRUE;
			}

		}
		return TRUE;
	}

	case WM_CLOSE:
	{
		EndDialog(hDlg, 0);
		return TRUE;
	}

	case WM_INITDIALOG:
	{
		/* Load settings from ollydbg.ini. If there is no
		 * setting already in the ollydbg.ini, set the default
		 * values so we can save them if we want
		 */

		read_settings_from_ini(hDlg);

		SetFocus(GetDlgItem(hDlg, IDC_CANCEL));
		return TRUE;
	}

	}
	return FALSE; 
}

void write_settings_to_ini(HWND hDlg)
{
	/* Database Path */
	GetDlgItemText(hDlg, IDC_DATABASE_PATH, database_path, MAXPATH);
	Writetoini(NULL, PLUGIN_NAME, L"Database path", database_path);

	/* Scan on Analysis */
	Writetoini(NULL,
				PLUGIN_NAME,
				L"Scan on analysis", L"%i",
				/* Writes 1 if checked, 0 if unchecked */
				SendMessage(GetDlgItem(hDlg, IDC_SCAN_ON_ANALYSIS), BM_GETCHECK, 0, 0));

	/* Scan on Module Load */
	Writetoini(NULL,
				PLUGIN_NAME,
				L"Scan on module load", L"%i",
				/* Writes 1 if checked, 0 if unchecked */
				SendMessage(GetDlgItem(hDlg, IDC_SCAN_ON_MODULE_LOAD), BM_GETCHECK, 0, 0));

	/* Scan EP only */
	Writetoini(NULL,
				PLUGIN_NAME,
				L"Scan EP only", L"%i",
				/* Writes 1 if checked, 0 if unchecked */
				SendMessage(GetDlgItem(hDlg, IDC_SCAN_EP_ONLY), BM_GETCHECK, 0, 0));
}


void read_settings_from_ini(HWND hDlg)
{
	/* Local variables */
	int ret;
	int n;						/* Used for string concatination */

	/* Database Path */
	ret = Stringfromini(PLUGIN_NAME, L"Database path", database_path, MAXPATH);
	if (ret == 0) {
		/* Set default database name to userdb.txt in plugin directory */
		n = StrcopyW(database_path, MAXPATH, plugindir);
		n += StrcopyW(database_path + n, MAXPATH - n, L"\\userdb.txt");
	}
	SetDlgItemText(hDlg, IDC_DATABASE_PATH, (LPCWSTR)database_path);

	/* Scan on Analysis */
	scan_on_analysis = 0;
	ret = Getfromini( NULL, PLUGIN_NAME, L"Scan on analysis", L"%i", &scan_on_analysis);
	if (scan_on_analysis == 1) {
		SendDlgItemMessage(hDlg, IDC_SCAN_ON_ANALYSIS, BM_SETCHECK, BST_CHECKED, 0);
	}

	/* Scan on Module Load */
	scan_on_mod_load = 0;
	ret = Getfromini( NULL, PLUGIN_NAME, L"Scan on module load", L"%i", &scan_on_mod_load);
	if (scan_on_mod_load == 1) {
		SendDlgItemMessage(hDlg, IDC_SCAN_ON_MODULE_LOAD, BM_SETCHECK, BST_CHECKED, 0);
	}

	/* Scan EP only */
	scan_ep_only = 0;
	ret = Getfromini( NULL, PLUGIN_NAME, L"Scan EP only", L"%i", &scan_ep_only);
	if (scan_ep_only == 1) {
		SendDlgItemMessage(hDlg, IDC_SCAN_EP_ONLY, BM_SETCHECK, BST_CHECKED, 0);
	}
}