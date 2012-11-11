/*******************************************************************************
 * OllyID - settings.c 
 *
 * Copyright (c) 2012, Austyn Krutsinger
 * All rights reserved.
 *
 * OllyID is released under the New BSD license (see LICENSE.txt).
 *
 ******************************************************************************/

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <ShellAPI.h>

#include "plugin.h"
#include "OllyID.h"
#include "settings.h"
#include "resource.h"

/* Globals Definitions - Program specific */
extern wchar_t database_path[MAX_PATH];
extern int scan_on_analysis;
extern int scan_on_mod_load;
extern int scan_ep_only;

INT_PTR CALLBACK settings_dialog_procedure(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) { 
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
			case IDC_OK:
				/*
				 * Save the settings
				 */
				save_settings(hDlg);
				EndDialog(hDlg, 0);
				return TRUE;
			case IDC_CANCEL:
				/* End dialog without saving anything */
				EndDialog(hDlg, 0);
				return TRUE;
			case IDC_BROWSE:
				Browsefilename(L"OllyID - Open database", database_path, L"*.txt", (wchar_t*)plugindir, L"txt", hwollymain, BRO_FILE);
				SetDlgItemText(hDlg, IDC_DATABASE_PATH, (LPCWSTR)database_path);
				return TRUE;
			case IDC_SCAN_EP_ONLY:
				if (SendMessage(GetDlgItem(hDlg, IDC_SCAN_EP_ONLY), BM_GETCHECK, 0, 0) == BST_UNCHECKED) {
					MessageBoxW(hDlg, L"Not scanning \"EP only\" signatures can be very inaccurate depending on what database you are using. It can also be very slow depending on the size of the module you are debuging. Until I get this fixed uncheck this with caution.", L"OllyID", MB_OK|MB_ICONEXCLAMATION);
				}
		}
		return TRUE;
	case WM_DROPFILES:
	{
		HDROP hdrop = (HDROP)wParam;
		DragQueryFile(hdrop, 0, database_path, sizeof(database_path));
		DragFinish(hdrop);
		SetDlgItemText(hDlg, IDC_DATABASE_PATH, (LPCWSTR)database_path);
		SetFocus(GetDlgItem(hDlg, IDC_DATABASE_PATH));
		return TRUE;
	}
	case WM_CLOSE:
		DragAcceptFiles(hDlg, FALSE);
		EndDialog(hDlg, 0);
		return TRUE;
	case WM_INITDIALOG:
		/* Load settings from ollydbg.ini. If there is no
		 * setting already in the ollydbg.ini, set the default
		 * values so we can save them if we want
		 */
		DragAcceptFiles(hDlg, TRUE);
		load_settings(hDlg);
		SetFocus(GetDlgItem(hDlg, IDC_CANCEL));
		return TRUE;

	}
	return FALSE; 
}

void save_settings(HWND hDlg)
{
	/* Database Path */
	GetDlgItemText(hDlg, IDC_DATABASE_PATH, database_path, MAXPATH);
	Writetoini(NULL, PLUGIN_NAME, L"Database path", database_path);

	/* Scan on Analysis */
	scan_on_analysis = SendMessage(GetDlgItem(hDlg, IDC_SCAN_ON_ANALYSIS), BM_GETCHECK, 0, 0);
	Writetoini(NULL,
				PLUGIN_NAME,
				L"Scan on analysis", L"%i",
				/* Writes 1 if checked, 0 if unchecked */
				scan_on_analysis);

	/* Scan on Module Load */
	scan_on_mod_load = SendMessage(GetDlgItem(hDlg, IDC_SCAN_ON_MODULE_LOAD), BM_GETCHECK, 0, 0);
	Writetoini(NULL,
				PLUGIN_NAME,
				L"Scan on module load", L"%i",
				/* Writes 1 if checked, 0 if unchecked */
				scan_on_mod_load);

	/* Scan EP only */
	scan_ep_only = SendMessage(GetDlgItem(hDlg, IDC_SCAN_EP_ONLY), BM_GETCHECK, 0, 0);
	Writetoini(NULL,
				PLUGIN_NAME,
				L"Scan EP only", L"%i",
				/* Writes 1 if checked, 0 if unchecked */
				scan_ep_only);
}


void load_settings(HWND hDlg)
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