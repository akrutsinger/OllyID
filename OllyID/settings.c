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
extern wchar_t	global_database_path[MAX_PATH];
extern int		global_scan_on_analysis		= FALSE;
extern int		global_scan_on_mod_load		= FALSE;
extern int		global_scan_ep_only			= TRUE;
extern int		global_must_read_database	= TRUE;
extern int		global_new_process_loaded	= FALSE;
extern int		global_log_level			= 1;
extern int		global_show_results_msgbox	= TRUE;

INT_PTR CALLBACK settings_dialog_procedure(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) { 
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
			case IDC_OK:
				/* Save the settings */
				save_settings(hDlg);
				EndDialog(hDlg, 0);
				return TRUE;
			case IDC_CANCEL:
				/* End dialog without saving anything */
				EndDialog(hDlg, 0);
				return TRUE;
			case IDC_BROWSE:
				Browsefilename(L"OllyID - Open database", global_database_path, L"*.txt", (wchar_t*)plugindir, L"txt", hwollymain, BRO_FILE);
				SetDlgItemText(hDlg, IDC_DATABASE_PATH, (LPCWSTR)global_database_path);
				return TRUE;
		}
		return TRUE;
	case WM_DROPFILES:
	{
		HDROP hdrop = (HDROP)wParam;
		DragQueryFile(hdrop, 0, global_database_path, sizeof(global_database_path));
		DragFinish(hdrop);
		SetDlgItemText(hDlg, IDC_DATABASE_PATH, (LPCWSTR)global_database_path);
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
	GetDlgItemText(hDlg, IDC_DATABASE_PATH, global_database_path, MAXPATH);
	Writetoini(NULL, PLUGIN_NAME, L"Database path", global_database_path);
	
	/* Global marker telling us the database file has changed */
	global_must_read_database = TRUE;

	/* Scan on Analysis */
	global_scan_on_analysis = SendMessage(GetDlgItem(hDlg, IDC_SCAN_ON_ANALYSIS), BM_GETCHECK, 0, 0);
	Writetoini(NULL,
				PLUGIN_NAME,
				L"Scan on analysis", L"%i",
				/* Writes 1 if checked, 0 if unchecked */
				global_scan_on_analysis);

	/* Scan on Module Load */
//	global_scan_on_mod_load = SendMessage(GetDlgItem(hDlg, IDC_SCAN_ON_MODULE_LOAD), BM_GETCHECK, 0, 0);
//	Writetoini(NULL,
//				PLUGIN_NAME,
//				L"Scan on module load", L"%i",
//				/* Writes 1 if checked, 0 if unchecked */
//				global_scan_on_mod_load);

	/* Scan EP only */
	global_scan_ep_only = SendMessage(GetDlgItem(hDlg, IDC_SCAN_EP_ONLY), BM_GETCHECK, 0, 0);
	Writetoini(NULL,
				PLUGIN_NAME,
				L"Scan EP only", L"%i",
				/* Writes 1 if checked, 0 if unchecked */
				global_scan_ep_only);

	/* Logging level */
	if (SendMessage(GetDlgItem(hDlg, IDC_LOG_NOTHING), BM_GETCHECK, 0, 0) == TRUE) {
		global_log_level = LOG_NOTHING;
	} else if (SendMessage(GetDlgItem(hDlg, IDC_LOG_INFO), BM_GETCHECK, 0, 0) == TRUE) {
		global_log_level = LOG_INFO;
	} else {
		global_log_level = LOG_ERROR;
	}
	Writetoini(NULL,
				PLUGIN_NAME,
				L"Log level", L"%i",
				global_log_level);

	/* Show results message box */
	global_show_results_msgbox = SendMessage(GetDlgItem(hDlg, IDC_SHOW_RESULTS_MSGBOX), BM_GETCHECK, 0, 0);
	Writetoini(NULL,
				PLUGIN_NAME,
				L"Show results msgbox", L"%i",
				/* Writes 1 if checked, 0 if unchecked */
				global_show_results_msgbox);
}


void load_settings(HWND hDlg)
{
	/* Local variables */
	int ret;
	int n;			/* Used for string concatination */

	/* Database Path */
	ret = Stringfromini(PLUGIN_NAME, L"Database path", global_database_path, MAXPATH);
	if (ret == 0) {
		/* Set default database name to userdb.txt in plugin directory */
		n = StrcopyW(global_database_path, MAXPATH, plugindir);
		n += StrcopyW(global_database_path + n, MAXPATH - n, L"\\userdb.txt");
	}
	SetDlgItemText(hDlg, IDC_DATABASE_PATH, (LPCWSTR)global_database_path);

	/* Scan on Analysis */
	global_scan_on_analysis = 0;
	ret = Getfromini( NULL, PLUGIN_NAME, L"Scan on analysis", L"%i", &global_scan_on_analysis);
	if (global_scan_on_analysis == 1)
		SendDlgItemMessage(hDlg, IDC_SCAN_ON_ANALYSIS, BM_SETCHECK, BST_CHECKED, 0);

	/* Scan on Module Load */
//	global_scan_on_mod_load = 0;
//	ret = Getfromini( NULL, PLUGIN_NAME, L"Scan on module load", L"%i", &global_scan_on_mod_load);
//	if (global_scan_on_mod_load == 1)
//		SendDlgItemMessage(hDlg, IDC_SCAN_ON_MODULE_LOAD, BM_SETCHECK, BST_CHECKED, 0);

	/* Scan EP only */
	global_scan_ep_only = 0;
	ret = Getfromini( NULL, PLUGIN_NAME, L"Scan EP only", L"%i", &global_scan_ep_only);
	if (global_scan_ep_only == 1)
		SendDlgItemMessage(hDlg, IDC_SCAN_EP_ONLY, BM_SETCHECK, BST_CHECKED, 0);

	/* Verbosity level */
	global_log_level = LOG_ERROR;
	ret = Getfromini( NULL, PLUGIN_NAME, L"Log level", L"%i", &global_log_level);
	if (global_log_level == LOG_NOTHING) {
		SendDlgItemMessage(hDlg, IDC_LOG_NOTHING, BM_SETCHECK, BST_CHECKED, 0);
	} else if (global_log_level == LOG_INFO) {
		SendDlgItemMessage(hDlg, IDC_LOG_INFO, BM_SETCHECK, BST_CHECKED, 0);
	} else {	/* LOG_ERROR */
		SendDlgItemMessage(hDlg, IDC_LOG_ERROR, BM_SETCHECK, BST_CHECKED, 0);
	}

	/* Show results message box */
	global_show_results_msgbox = 0;
	ret = Getfromini( NULL, PLUGIN_NAME, L"Show results msgbox", L"%i", &global_show_results_msgbox);
	if (global_show_results_msgbox == 1)
		SendDlgItemMessage(hDlg, IDC_SHOW_RESULTS_MSGBOX, BM_SETCHECK, BST_CHECKED, 0);
}