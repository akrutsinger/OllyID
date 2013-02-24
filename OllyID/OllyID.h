/*******************************************************************************
 * OllyID - OllyID.h
 *
 * Copyright (c) 2012, Austyn Krutsinger
 * All rights reserved.
 *
 * OllyID is released under the New BSD license (see LICENSE.txt).
 *
 ******************************************************************************/

#include "dictionary.h"

#ifndef _OLLYID_H_
#define _OLLYID_H_

#define PLUGIN_NAME		L"OllyID"		/* Unique plugin name */
#define PLUGIN_VERS		L"0.4.0"		/* Plugin version (stable . update . patch  - status) */

/* Menu items */
#define	MENU_LOG_WINDOW		1
#define MENU_SETTINGS		2
#define	MENU_ABOUT			3
#define	MENU_SCAN_MODULE	4
#define	MENU_CREATE_SIG		5
#define MENU_TEST_CODE		255

#define SIG_FOUND		1
#define SIG_NO_MATCH	-1

/**
 * Forward declarations
 */
 /* Menu functions */
int menu_handler(t_table *pTable, wchar_t *pName, ulong index, int nMode);
void display_about_message(void);
/* File functions */
int scan_module(void);
/* Helper functions */
int parser_helper_func(const char *signature_name, const char *property_name, const char *value);
char *module_mem_alloc(void);
int compare_from_entry_point(const char *module, const char *data, const int signature_len);
int compare_from_module_start(const char *module, const char *data, const int signature_len);
int find_signature(struct sig_entry_s *entry);
void free_signatures(struct sig_entry_s *data);
void module_mem_free(char *module);
struct signature_list_s *signature_list_alloc(void);
void signature_list_free(struct signature_list_s *list);
struct sig_entry_s *signature_find_pos(const struct signature_list_s *list, int posstart);
int append_signature(struct signature_list_s *list, const struct sig_entry_s *entry);
struct signature_list_s *build_database(dictionary *dict);
int signature_list_realloc(struct signature_list_s *list);

/**
 * Global Declarations
 */

struct sig_entry_s {
	char	*name;
	char	*data;
	int		sig_len;
	int		ep_signature;
	struct sig_entry_s	*prev;
	struct sig_entry_s	*next;
};

struct signature_list_s {
	struct sig_entry_s	*head_sentinel;
	struct sig_entry_s	*tail_sentinel;

	uint num_elements;
};

/*
 * Plugin menu that will appear in the main OllyDbg menu
 * and in popup menu.
 */
static t_menu ollyid_menu[] =
{
#ifdef DEVELOPMENT_MODE
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