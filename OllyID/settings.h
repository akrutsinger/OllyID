/*******************************************************************************
 * OllyID - settings.h
 *
 * Copyright (c) 2012, Austyn Krutsinger
 * All rights reserved.
 *
 * OllyID is released under the New BSD license (see LICENSE.txt).
 *
 ******************************************************************************/

#ifndef _OLLYID_SETTINGS_H_
#define _OLLYID_SETTINGS_H_

/* Global Declarations */
wchar_t database_path[MAX_PATH];
int scan_on_analysis;
int scan_on_mod_load;
int scan_ep_only;

/* Prototypes */
INT_PTR CALLBACK settings_dialog_procedure(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void save_settings(HWND hDlg);
void load_settings(HWND hDlg);

#endif	/*_OLLYID_SETTINGS_INCLUDED_ */