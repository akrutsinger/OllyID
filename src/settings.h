/*******************************************************************************
 * OllyID - settings.h
 *
 * Copyright (c) 2012-2013, Austyn Krutsinger
 * All rights reserved.
 *
 * OllyID is released under the New BSD license (see LICENSE.txt).
 *
 ******************************************************************************/

#ifndef _OLLYID_SETTINGS_H_
#define _OLLYID_SETTINGS_H_

/* Global Declarations */
wchar_t global_database_path[MAX_PATH];
int global_scan_on_analysis;
int global_scan_on_mod_load;
int global_scan_ep_only;
int global_must_read_database;
int global_new_process_loaded;
int global_log_level;
int global_show_results_msgbox;

/* Prototypes */
INT_PTR CALLBACK settings_dialog_procedure(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void save_settings(HWND hDlg);
void load_settings(HWND hDlg);

#endif	/*_OLLYID_SETTINGS_INCLUDED_ */