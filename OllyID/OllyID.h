/*******************************************************************************
 * OllyID - OllyID.h
 *
 * Copyright (c) 2012, Austyn Krutsinger
 * All rights reserved.
 *
 * OllyID is released under the New BSD license (see LICENSE.txt).
 *
 ******************************************************************************/

#ifndef _OLLYID_H_
#define _OLLYID_H_

#define PLUGIN_NAME		L"OllyID"		/* Unique plugin name */
#define PLUGIN_VERS		L"0.3.0"		/* Plugin version (stable . update . patch  - status) */

#define DEVELOPMENT_MODE	0			/* Enables / Disables development code /*

/* Menu items */
#define	MENU_LOG_WINDOW		1
#define MENU_SETTINGS		2
#define	MENU_ABOUT			3
#define	MENU_SCAN_MODULE	4
#define	MENU_CREATE_SIG		5
#define MENU_TEST_CODE		255

/**
 * Forward declarations
 */
 /* Menu functions */
int menu_handler(t_table* pTable, wchar_t* pName, ulong index, int nMode);
void display_about_message(void);
/* File functions */
int scan_module(void);
/* Helper functions */
int find_signature_helper(void* signature_block, const char* signature_name, const char* name, const char* value);
int module_mem_create(void);

/**
 * Global Declarations
 */


struct t_signature_data {
    char	*name;
    char	*data;
    int		ep_signature;
} t_signature_data;

/*
 * Plugin menu that will appear in the main OllyDbg menu
 * and in popup menu.
 */
static t_menu ollyid_menu[] =
{
#if DEVELOPMENT_MODE
	{ L"Test Code",
		L"Test Code",
		KK_DIRECT|KK_CTRL|KK_SHIFT|'T', menu_handler, NULL, MENU_TEST_CODE },
#endif
	{ L"Scan Module",
		L"Scan Module",
		KK_DIRECT|KK_CTRL|'S', menu_handler, NULL, MENU_SCAN_MODULE },
//	{ L"|Create Signature",
//		L"Create Signature",
//		KK_DIRECT|KK_CTRL|KK_SHIFT|'C', menu_handler, NULL, MENU_CREATE_SIG },
	{ L"|Settings",
		L"Open OllyID's Options Window",
		K_NONE, menu_handler, NULL, MENU_SETTINGS },
	{ L"|About",
		L"About OllyID",
		K_NONE, menu_handler, NULL, MENU_ABOUT },
	/* End of menu. */
	{ NULL, NULL, K_NONE, NULL, NULL, 0 }
};

#endif /* _OLLYID_H_ */