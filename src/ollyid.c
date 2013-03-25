/*******************************************************************************
 * OllyID - ollyid.c
 *
 * Copyright (c) 2012-2013, Austyn Krutsinger
 * All rights reserved.
 *
 * OllyID is released under the New BSD license (see LICENSE.txt).
 *
 ******************************************************************************/

/*
 * VERY IMPORTANT NOTICE: PLUGINS ARE UNICODE LIBRARIES! COMPILE THEM WITH BYTE
 * ALIGNMENT OF STRUCTURES AND DEFAULT UNSIGNED CHAR!
 */

/*******************************************************************************
 * Things to change as I think of them...
 * [ ] = To do
 * [?] = Might be a good idea?
 * [!] = Implemented
 * [+] = Added
 * [-] = Removed
 * [*] = Changed
 * [~] = Almost there...
 *
 * Version 0.5.1 (11MAR2013)
 * [*] Fixed bug where OllyID would crash if no module was ever loaded and database was switched
 * [*] Load database file only if path actually changed
 *
 * Version 0.5.0 (25FEB2013)
 * [+] Added support for OllyDbg 201h
 * [+] Option: Message box for search result
 * [+] Option: Verbose program output
 * [*] Standardized naming of global variables
 *
 * Version 0.4.0 (24FEB2013)
 * [+] Signatures are stored in link list dramatically increasing search speed
 * [+] Added internal cstrndup function to replace _strdup
 * [+] Added progress bar during scanning
 * [*] Parses database file more efficiently
 * [*] Changed strlen to use Olly's StrlenA function
 * [*] Major code cleanup
 *
 * Version 0.3.0 (12NOV2012)
 * [*] Changed the way signatures are searched for another huge speed increase
 * [*] Minor code cleanup
 *
 * Version 0.2.0 (09NOV2012)
 * [+] Add "Browse" button for database
 * [+] Enable drag-n-drop for database into Settings
 * [+] Major code optimization: about 1000% increase (literally) in non ep_only scanning (whoops!)
 *
 * Version 0.1.1 (02NOV2012)
 * [+] Implemented Scan on analysis
 * [+] Added notification to log window for scanning
 * [*] Fix bug where settings were not always updated in memory
 *
 * Version 0.1.0 (01NOV2012)
 * [+] Scan entire module or only from EP
 * [+] Settings Dialog
 * [-] Removed TEST menu...finally
 * [-] Removed Log Window remnants
 * [*] Main menu and Popup menus now match
 * [*] Significant code cleanup
 *
 * Version 0.0.4-alpha (08OCT2012)
 * [+] Updated for OllyDbg 2.01g PDK
 *
 * Version 0.0.3-alpha (24SEP2012)
 * [+] Improved scan functionality
 *
 * Version 0.0.2-alpha
 * [+] Added ScanModule routine to search database
 *
 * Version 0.0.1-alpha
 * [+] Base code
 *
 *
 * -----------------------------------------------------------------------------
 * TODO
 * -----------------------------------------------------------------------------
 *
 * [ ] Option: Scan On Module Load
 * [ ] Make parsing and dictionary solely unicode
 * [ ] Adjust location of Suspendallthreads() and Resumeallthreads()
 * [ ] Check for memory leaks
 * [ ] Add menu setting for show info in Log window or MessageBox or both
 * [ ] Store module and signature database information as uchar instead of char's
 * [ ] Scan any module currently in CPU instead of just main module
 * [ ] Add Show scan time setting
 * [ ] Fix the way the parser handles double brackets. Id est [[MSLRH]] displays as [[MSLRH
 * [ ] Implement "Create Signature"
 * [ ] Implement string routines in code instead of stdlib.h (i.e. strdup, strcpy, etc)
 * [ ] Improve error checking for most everything
 * [ ] Restructure project files layout and build locations et cetera
 * [ ] Download datase from: http://abysssec.com/AbyssDB/Database.TXT
 *
 ******************************************************************************/

/* Defines */
/*
 * Microsoft compilers hate (and maybe justifiably) old-school functions like
 * wcscpy() that may cause buffer overflow and all related dangers. Still, I
 * don't want to see all these bloody warnings.
 */
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN		/* Remove extra windows.h information */

/* Includes */
#include <Windows.h>
#include <time.h>

#include "plugin.h"
#include "ollyid.h"
#include "string.h"
#include "parser.h"
#include "settings.h"
#include "resource.h"

/* Globals Definitions - Module specific */
static HINSTANCE				main_plugin_instance = NULL;	/* Instance of plugin DLL */
static t_module					*main_module = NULL;			/* Pointer to main module struct */
static char						*main_module_code = NULL;		/* Global pointer for allocated module code */
static struct signature_list_s	*main_signature_list = NULL;	/* Global pointer for list of signature entries */
static dictionary				*main_dictionary = NULL;		/* Dictionary structure that stores database information */
static long						start_time, end_time;			/* Used for calculating search time */
////////////////////////////////////////////////////////////////////////////////
////////////////// PLUGIN MENUS EMBEDDED INTO OLLYDBG WINDOWS //////////////////

/**
 * @display_about_message
 *
 *		Displays "About" message box
 */
void display_about_message(void)
{
	wchar_t about_message[TEXTLEN] = { 0 };
	wchar_t buf[SHORTNAME];
	int n;

	/* Debuggee should continue execution while message box is displayed. */
	Resumeallthreads();
	/* In this case, swprintf() would be as good as a sequence of StrcopyW(), */
	/* but secure copy makes buffer overflow impossible. */
	n = StrcopyW(about_message, TEXTLEN, L"OllyID v");
	n += StrcopyW(about_message + n, TEXTLEN - n, PLUGIN_VERS);
	n += StrcopyW(about_message + n, TEXTLEN - n, L"\n\nCoded by Austyn Krutsinger <akrutsinger@gmail.com>");
	/* The conditionals below are here to verify that this plugin can be */
	/* compiled with all supported compilers. They are not necessary in the */
	/* final code. */
		n += StrcopyW(about_message + n, TEXTLEN - n, L"\n\nCompiled on ");
		Asciitounicode(__DATE__, SHORTNAME, buf, SHORTNAME);
		n += StrcopyW(about_message + n, TEXTLEN - n, buf);
		n += StrcopyW(about_message + n, TEXTLEN - n, L" ");
		Asciitounicode(__TIME__, SHORTNAME, buf, SHORTNAME);
		n += StrcopyW(about_message + n, TEXTLEN - n, buf);
		n += StrcopyW(about_message + n, TEXTLEN - n, L" with ");
	#if defined(__BORLANDC__)
		n += StrcopyW(about_message + n, TEXTLEN - n, L"Borland (R) ");
	#elif defined(_MSC_VER)
		n += StrcopyW(about_message + n, TEXTLEN - n, L"Microsoft (R) ");
	#elif defined(__MINGW32__)
		n += StrcopyW(about_message + n, TEXTLEN - n, L"MinGW32 ");
	#else
		n += StrcopyW(about_message + n, TEXTLEN - n, L"\n\nCompiled with ");
	#endif
	#ifdef __cplusplus
		StrcopyW(about_message + n, TEXTLEN - n, L"C++ compiler");
	#else
		StrcopyW(about_message + n, TEXTLEN - n, L"C compiler");
	#endif
	MessageBox(hwollymain, about_message, L"About OllyID", MB_OK|MB_ICONINFORMATION);
	/* Suspendallthreads() and Resumeallthreads() must be paired, even if they */
	/* are called in inverse order! */
	Suspendallthreads();
}

/**
 * @menu_handler
 * 
 *      Menu callback for our plugin to process our menu commands.
 */
int menu_handler(t_table* pTable, wchar_t* pName, ulong index, int nMode)
{
	UNREFERENCED_PARAMETER(pTable);
	UNREFERENCED_PARAMETER(pName);

	switch (nMode) {
	case MENU_VERIFY:
		return MENU_NORMAL;

	case MENU_EXECUTE:
		switch (index) {
		case MENU_SETTINGS: /* Menu -> Settings */
			DialogBox(main_plugin_instance,
						MAKEINTRESOURCE(IDD_SETTINGS),
						hwollymain,
						(DLGPROC)settings_dialog_procedure);
			break;
		case MENU_SCAN_MODULE: /* Disasm Menu -> Scan Module */
			scan_module();
			break;
		case MENU_CREATE_SIG: /* Disasm Menu -> Create Signature */
			break;
		case MENU_ABOUT: /* Menu -> About */
			display_about_message();
			break;
#ifdef DEVELOPMENT_MODE
		case MENU_TEST_CODE: /* Menu -> Test Code */
		{
			Addtolist(0, DRAW_HILITE, L"database changed: %i", global_must_read_database);
			break;
		}
#endif
		}
		return MENU_NOREDRAW;
	}

	return MENU_ABSENT;
}

/**
 * @ODBG2_Pluginmenu
 *
 *      Adds items to OllyDbgs menu system.
 */
extc t_menu * __cdecl ODBG2_Pluginmenu(wchar_t *type)
{
	if (wcscmp(type, PWM_MAIN) == 0)
		/* Main menu. */
		return ollyid_menu;
	else if (wcscmp(type, PWM_DISASM) == 0)
		/* Disassembler pane of CPU window. */
		return ollyid_menu;
	return NULL;		/* No menu */
};

////////////////////////////////////////////////////////////////////////////////
///////////////////////////// FILE ROUTINES ////////////////////////////////////

struct sig_entry_s *signature_find_pos(const struct signature_list_s *list, int posstart)
{
	struct sig_entry_s *ptr = NULL;
	float x;
	int i;
	
	/* accept 1 slot overflow for fetching head and tail sentinels */
	if (posstart < -1 || posstart > (int)list->num_elements)
		return NULL;

	if (list->num_elements <= 0) {
		x = 0;
	} else {
		x = (float)(posstart + 1) / list->num_elements;
	}
	if (x <= 0.5) {
		/* first half: get to posstart from head */
		for (i = -1, ptr = list->head_sentinel; i < posstart; ptr = ptr->next, i++);
	} else {
		/* last half: get to posstart from tail */
		for (i = list->num_elements, ptr = list->tail_sentinel; i > posstart; ptr = ptr->prev, i--);
	}

	return ptr;
}

int append_signature(struct signature_list_s *list, const struct sig_entry_s *entry)
{
	struct sig_entry_s *new_entry, *succ, *prec;

	new_entry = (struct sig_entry_s *)Memalloc(sizeof(struct sig_entry_s), SILENT|ZEROINIT);

	new_entry->data = cstrndup(entry->data, DATALEN);
	new_entry->name = cstrndup(entry->name, TEXTLEN);
	new_entry->ep_signature = entry->ep_signature;
	new_entry->sig_len = entry->sig_len;

	/* actually append element */
	prec = signature_find_pos(list, list->num_elements - 1);
	succ = prec->next;

	prec->next = new_entry;
	new_entry->prev = prec;
	new_entry->next = succ;
	succ->prev = new_entry;

	list->num_elements++;

	return 0;
}

struct signature_list_s *build_database(dictionary *dict)
{
	char sec_buf[TEXTLEN];
	struct signature_list_s *list = NULL;
	struct sig_entry_s *entry = NULL;
	struct sig_entry_s *prev_sig = NULL;
	int i, n, ep_only;
	char *section_name;
	char *key;

	list = signature_list_alloc();
	if (list == NULL)
		return NULL;

	for (i = 0; i < iniparser_getnsec(dict); i++) {
		prev_sig = entry;	/* saves previous entry before the new entry is created */

		/* get name, section data, and ep_only */
		section_name = iniparser_getsecname(dict, i);

		n = StrcopyA(sec_buf, TEXTLEN, section_name);
		n += StrcopyA(sec_buf + n, TEXTLEN - n, ":signature");
		key = iniparser_getstring(dict, sec_buf, "00");
		
		n = StrcopyA(sec_buf, TEXTLEN, section_name);
		n += StrcopyA(sec_buf + n, TEXTLEN - n, ":ep_only");
		ep_only = (strcmp(iniparser_getstring(dict, sec_buf, "false"), "true") == 0) ? TRUE : FALSE;

		/* Allocate entry */
		entry = (struct sig_entry_s *)Memalloc(sizeof(struct sig_entry_s), SILENT|ZEROINIT);
		entry->name = cstrndup(section_name, TEXTLEN);
		/* Remove spaces from key (data) to save space make comparing faster */
		remove_char(key, ' ');
		entry->data = cstrndup(key, DATALEN);
		entry->sig_len = StrlenA(entry->data, DATALEN);
		entry->ep_signature = ep_only;
		entry->prev = prev_sig;
		if (entry->prev != NULL)
			entry->prev->next = entry;

		/* add it to our list */
		append_signature(list, entry);
	}
	
	return list;
}

int scan_module(void)
{
	FILE	*signature_file = NULL;		/* Pointer to database file */
	struct sig_entry_s *cur_sig = NULL;
	int		ret;						/* Return values for certain functions */

	Suspendallthreads();

	/* Get the main memory module so we can use the OEP */
	main_module = Findmainmodule();

	/* Make sure there is actually a module loaded into Olly */
	if (main_module != NULL) {

		start_timer();

		/* Allocate and read module memory into buffer */
		if (global_new_process_loaded == TRUE) {
			module_mem_free(main_module_code);
			main_module_code = module_mem_alloc();
			global_new_process_loaded = FALSE;
		}

		/* Initialize the global list used to store the signatures */
		if (main_signature_list == NULL)
			main_signature_list = signature_list_alloc();

		/* If the settings have changed we need to free the old signature database */
		if(global_must_read_database == TRUE) {
			dictionary_del(main_dictionary);
			main_dictionary = iniparser_load(global_database_path);
			if (main_dictionary == NULL) {
				if (global_log_level >= LOG_ERROR)
					Addtolist(0, DRAW_HILITE, L"[!] Could not initialize database");
				return 0;
			} else {
				if (global_log_level >= LOG_INFO)
					Addtolist(0, DRAW_NORMAL, L"[*] Total signatures: %i", iniparser_getnsec(main_dictionary));
				signature_list_free(main_signature_list);
				main_signature_list = build_database(main_dictionary);
				if (main_signature_list == NULL) {
					if (global_log_level >= LOG_ERROR)
						Addtolist(0, DRAW_HILITE, L"[!] Could not build database");
					return 0;
				}
				global_must_read_database = FALSE;	/* Unset until a new signature is loaded */
			}
		}

		cur_sig = main_signature_list->head_sentinel;	/* Set our current signature to the root node */
		
		if (global_scan_ep_only == 1) {
			if (global_log_level >= LOG_INFO)
				Addtolist(0, DRAW_NORMAL, L"[+] Searching EP only for signatures");
		} else {
			if (global_log_level >= LOG_INFO)
				Addtolist(0, DRAW_NORMAL, L"[*] Searching entire module for signatures");
		}

		/* Scan the module for a signature */
		ret = find_signature(cur_sig);

		if (ret == SIG_NO_MATCH) {
			stop_timer();
			if (global_log_level > LOG_NOTHING)
				Addtolist(0, DRAW_HILITE, L"[!] Nothing Found");
			if (global_show_results_msgbox == TRUE )
				MessageBox(hwollymain, L"Nothing found", L"OllyID", MB_OK | MB_ICONINFORMATION);
			ret = 1;
		}

		if (global_log_level >= LOG_INFO)
			Addtolist(0, DRAW_NORMAL, L"[*] Total time: %ldms", end_time - start_time);
	} else {		/* No module loaded */
		if (global_log_level >= LOG_ERROR)
			Addtolist(0, DRAW_HILITE, L"[!] No module loaded");
		if (global_show_results_msgbox == TRUE )
			MessageBox(hwollymain, L"No module loaded", L"OllyID", MB_OK | MB_ICONINFORMATION);
	}


	/* Free resources */
	if (signature_file != NULL)
		fclose(signature_file);
		
	Resumeallthreads();

	return ret;
}

int compare_from_entry_point(const char *module, const char *data, const int signature_len)
{
	ulong	codebase_ep_offset;	/* Offset of EP relative to the start of the code base */
	int i;						/* Indexing variable */

	/* Calculate the EP offset relative to the begenning of the code base */
	/* We have to multiply by 2 because each uchar in memory became two char's */
	codebase_ep_offset =  (main_module->entry - main_module->codebase) * 2;

	/* Compare signature bytes to code starting at OEP */
	for (i = 0; i < signature_len; i++) {
		if ((module[codebase_ep_offset + i] != data[i]) && (data[i] != '?'))
			return FALSE;	/* Mismatch */
	}

	return TRUE;
}

int compare_from_module_start(const char *module, const char *data, const int signature_len)
{
	ulong i;	/* Indexing variable */
	int j;		/* Indexing variable */

	/*Start scanning from beginning of module */
	for (i = 0; i < main_module->codesize * 2; i++) {
		/* Compare signature bytes to code starting at module loction i */
		for (j = 0; j < signature_len; j++) {
			if ((module[j + i] != data[j]) && (data[j] != '?'))
				return FALSE;	/* Strings don't match */
		}
	}

	return TRUE;
}

int find_signature(struct sig_entry_s *entry)
{
	ulong	code_len = 0;			/* Length of memory read */
	ulong	sig_counter = 1;		/* counter for current signature number scanned */
	int		ret = SIG_NO_MATCH;		/* Little 'ol return value */
	wchar_t	wbuf[TEXTLEN];			/* Buffer for name of current signature */


	//:) I LOVE YOU

	/* Search loop */
	while (entry->next != NULL) {
		/* Update progress bar */
		Asciitounicode(entry->name, TEXTLEN, wbuf, TEXTLEN);
		Progress((int)(((double)sig_counter / (double)main_signature_list->num_elements) * 1000.0), L"Scanning: %s  ", wbuf);
		sig_counter++;

		if (global_scan_ep_only == TRUE) {
			if (entry->ep_signature == TRUE) {
				ret = compare_from_entry_point(main_module_code, entry->data, entry->sig_len);
				if (ret == TRUE) {
					stop_timer();
					Asciitounicode(entry->name , TEXTLEN, wbuf, TEXTLEN);
					if (global_log_level > LOG_NOTHING)
						Addtolist(0, DRAW_HILITE, L"[+] %s", wbuf);
					if (global_show_results_msgbox == TRUE )
						MessageBox(hwollymain, wbuf, L"OllyID", MB_OK);
					return SIG_FOUND;
				}
			} else {
				// skip because signature could be anywhere in file
			}
		} else if (global_scan_ep_only == FALSE) {
			if (entry->ep_signature == TRUE) {
				/* Don't scan entire file since this signature is only located at entry point */
				ret = compare_from_entry_point(main_module_code, entry->data, entry->sig_len);
				if (ret == TRUE) {
					stop_timer();
					Asciitounicode(entry->name , TEXTLEN, wbuf, TEXTLEN);
					if (global_log_level > LOG_NOTHING)
						Addtolist(0, DRAW_HILITE, L"[+] %s", wbuf);
					if (global_show_results_msgbox == TRUE )
						MessageBox(hwollymain, wbuf, L"OllyID", MB_OK);
					return SIG_FOUND;
				}
			} else if ((entry->ep_signature == FALSE) && (entry->data != NULL)) {	/* sometimes data is null */
				// scan from begenning of file
				ret = compare_from_module_start(main_module_code, entry->data, entry->sig_len);
				if (ret == TRUE) {
					stop_timer();
					Asciitounicode(entry->name , TEXTLEN, wbuf, TEXTLEN);
					if (global_log_level > LOG_NOTHING)
						Addtolist(0, DRAW_HILITE, L"[+] %s", wbuf);
					if (global_show_results_msgbox == TRUE )
						MessageBox(hwollymain, wbuf, L"OllyID", MB_OK);
					return SIG_FOUND;
				}
			}
		}

		/* Go to the next signature and continue scanning */
		entry = entry->next;
	};	/* while */

	return SIG_NO_MATCH;	/* If we make it here, no signature was found */
}

/* Return pointer to allocated memory */
char *module_mem_alloc(void)
{
	char *ptr = NULL;
	uchar *code_buf = NULL;
	ulong code_len;
	int n;

	/* Allocate and read memory for the module code to be stored for faster searching */
	code_buf = (uchar *)Memalloc(main_module->codesize, REPORT|ZEROINIT);
	code_len = Readmemory((uchar *)code_buf, main_module->codebase, main_module->codesize, MM_SILENT);
	/* Convert byte codes to a UNICODE string */
	ptr = (char *)Memalloc((code_len * 2) + 1, REPORT|ZEROINIT);	/* Include +1 for \0 */
	n = HexdumpA(ptr, code_buf, code_len);

	/* Free the code buffer */
	if (code_buf != NULL)
		Memfree(code_buf);

	return ptr;
}

void module_mem_free(char *module)
{
	/* Free memory allocated for module code bytes */
	if (module != NULL)
		Memfree((char *)module);
}

void free_signatures(struct sig_entry_s *data)
{
	if (data != NULL) {
		struct sig_entry_s *tmp;
		struct sig_entry_s *cur_sig = data;
		while (cur_sig->next != NULL) {
			tmp = cur_sig->next;
			Memfree((struct sig_entry_s *)cur_sig->data);
			Memfree((struct sig_entry_s *)cur_sig->name);
			Memfree((struct sig_entry_s *)cur_sig);
			cur_sig = tmp;
		}
	}
}

struct signature_list_s *signature_list_alloc(void)
{
	struct signature_list_s *list;

	list = (struct signature_list_s *)Memalloc(sizeof(struct signature_list_s), SILENT|ZEROINIT);
	if (list == NULL)
		return NULL;

	list->num_elements = 0;

	/* head/tail sentinels */
	list->head_sentinel = (struct sig_entry_s *)Memalloc(sizeof(struct sig_entry_s), SILENT|ZEROINIT);
	list->tail_sentinel = (struct sig_entry_s *)Memalloc(sizeof(struct sig_entry_s), SILENT|ZEROINIT);
	list->head_sentinel->next = list->tail_sentinel;
	list->tail_sentinel->prev = list->head_sentinel;
	list->head_sentinel->prev = list->tail_sentinel->next = NULL;
	list->head_sentinel->name = list->tail_sentinel->name = NULL;
	list->head_sentinel->data = list->tail_sentinel->data = NULL;

	return list;
}

void signature_list_free(struct signature_list_s *list)
{
	struct sig_entry_s *entry = NULL;
	
	if (list == NULL)
		return;
		
	entry = list->head_sentinel;
	while (entry != list->tail_sentinel) {
		if (entry->data != NULL)
			Memfree((struct sig_entry_s *)entry->data);
		if (entry->name != NULL)
			Memfree((struct sig_entry_s *)entry->name);
		entry = entry->next;
		if (entry->prev != NULL)
			Memfree((struct sig_entry_s *)entry->prev);
	}
	if (list->head_sentinel != NULL)
		Memfree((struct signature_list_s *)list->head_sentinel);
	if (list->tail_sentinel != NULL)
		Memfree((struct signature_list_s *)list->tail_sentinel);
	Memfree((struct signature_list_s *)list);
}

int signature_list_realloc(struct signature_list_s *list)
{
	signature_list_free(list);
	list = signature_list_alloc();

	return 0;
}

void start_timer(void)
{
	start_time = clock();
}

void stop_timer(void)
{
	end_time = clock();
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////// PLUGIN INITIALIZATION /////////////////////////////

/**
 * @DllMain
 * 
 *      Dll entrypoint - mainly unused.
 */
BOOL WINAPI DllEntryPoint(HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
		main_plugin_instance = hinstDll;		/* Save plugin instance */
	return 1;							/* Report success */
};

/**
 * @ODBG2_Pluginquery - required!
 * 
 *      Handles initializing the plugin.
 */
extc int __cdecl ODBG2_Pluginquery(int ollydbgversion, ulong *features, wchar_t pluginname[SHORTNAME], wchar_t pluginversion[SHORTNAME])
{
	/*
	 * Check whether OllyDbg has compatible version. This plugin uses only the
	 * most basic functions, so this check is done pro forma, just to remind of
	 * this option.
	 */
	if (ollydbgversion < 201)
		return 0;
	/* Report name and version to OllyDbg */
	StrcopyW(pluginname, SHORTNAME, PLUGIN_NAME);
	StrcopyW(pluginversion, SHORTNAME, PLUGIN_VERS);
	return PLUGIN_VERSION;			/* Expected API version */
};

/**
 * @ODBG2_Plugininit - optional
 * 
 *      Handles one-time initializations and allocate resources.
 */
extc int __cdecl ODBG2_Plugininit(void)
{
	int ret = 0;

	load_settings(NULL);

	Addtolist(0, DRAW_NORMAL, L"");
	Addtolist(0, DRAW_NORMAL, L"[*] %s v%s", PLUGIN_NAME, PLUGIN_VERS);
	Addtolist(0, DRAW_NORMAL, L"[*] Coded by: Austyn Krutsinger <akrutsinger@gmail.com>");
	Addtolist(0, DRAW_NORMAL, L"");

	/* Set this to true so we only parse the database when needed */
	global_must_read_database = TRUE;

	/* Report success. */
	return 0;
};

/*
 * Optional entry, called each time OllyDbg analyses some module and analysis
 * is finished. Plugin can make additional analysis steps. Debugged application
 * is paused for the time of processing. Bookmark plugin, of course, does not
 * analyse code. If you don't need this feature, remove ODBG2_Pluginanalyse()
 * from the plugin code.
 */
extc void __cdecl ODBG2_Pluginanalyse(t_module *pmod)
{
	if (global_scan_on_analysis == 1) {
		scan_module();
	}
};

/*
 * OllyDbg calls this optional function once on exit. At this moment, all MDI
 * windows created by plugin are already destroyed (and received WM_DESTROY
 * messages). Function must free all internally allocated resources, like
 * window classes, files, memory etc.
 */
extc void __cdecl ODBG2_Plugindestroy(void)
{
	if (main_dictionary != NULL)
		dictionary_del(main_dictionary);

	/* Free all signatures stored in memory */
	signature_list_free(main_signature_list);

	/* Free memory allocated for module code bytes */
	module_mem_free(main_module_code);
};

/*
 * Function is called when user opens new or restarts current application.
 * Plugin should reset internal variables and data structures to the initial
 * state.
 */
//extc void __cdecl ODBG2_Pluginreset(void)
//{
//};

/*
 * OllyDbg calls this optional function when user wants to terminate OllyDbg.
 * All MDI windows created by plugins still exist. Function must return 0 if
 * it is safe to terminate. Any non-zero return will stop closing sequence. Do
 * not misuse this possibility! Always inform user about the reasons why
 * termination is not good and ask for his decision! Attention, don't make any
 * unrecoverable actions for the case that some other plugin will decide that
 * OllyDbg should continue running.
 */
//extc int __cdecl ODBG2_Pluginclose(void)
//{
//	return 0;
//};


////////////////////////////////////////////////////////////////////////////////
/////////////////////////// EVENTS AND NOTIFICATIONS ///////////////////////////
/*
 * If you define ODBG2_Pluginmainloop(), this function will be called each time
 * from the main Windows loop in OllyDbg. If there is some (real) debug event
 * from the debugged application, debugevent points to it, otherwise it's NULL.
 * If fast command emulation is active, it does not receive all (emulated)
 * exceptions, use ODBG2_Pluginexception() instead. Do not declare these two
 * functions unnecessarily, as this may negatively influence the overall speed!
 */
// extc void __cdecl ODBG2_Pluginmainloop(DEBUG_EVENT *debugevent) {
// };
// extc void __cdecl ODBG2_Pluginexception(t_run *prun, t_thread *pthr, t_reg *preg) {
// };

/*
 * Optional entry, notifies plugin on relatively infrequent events.
 */
extc void __cdecl ODBG2_Pluginnotify(int code, void *data, ulong parm1, ulong parm2)
{
	if (code == PN_NEWPROC) {
		global_new_process_loaded = TRUE;
//		if (global_scan_on_mod_load == TRUE)
//			scan_module();
	}
};