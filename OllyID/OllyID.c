/*******************************************************************************
 * OllyID - OllyID.c
 *
 * Copyright (c) 2012, Austyn Krutsinger
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
 *	WARNING!!!!
 *		CURRENTLY THIS PLUGIN ASSUMES THE USERDB.TXT (SIGNATURE DATABASE) IS
 *		FORMATTED CORRECTLY, AS IT WOULD BE FOR PEiD OR ANY OTHER PROGRAM THAT
 *		USES IT. THAT IS ALL...
 ******************************************************************************/

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
 * [ ] Scan any module currently in CPU instead of just main module
 * [ ] Add Show scan time setting
 * [ ] Option: Scan On Module Load
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
#include "OllyID.h"
#include "string.h"
#include "parser.h"
#include "settings.h"
#include "resource.h"

/* Globals Definitions - Module specific */
static HINSTANCE	plugin_instance;				/* Instance of plugin DLL */
static t_module		*main_module;					/* Pointer to main module struct */
static char			*module_code = NULL;				/* Global pointer for allocated module code */

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
	int n;
	wchar_t buf[SHORTNAME];

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
			DialogBox(plugin_instance,
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
#if DEVELOPMENT_MODE
		case MENU_TEST_CODE: /* Menu -> Test Code */

			break;
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

int find_signature_helper(void* signature_data, const char* signature_name,
							const char* property_name, const char* value)
{
	ulong	codebase_ep_offset;		/* Offset of EP relative to the start of the code base */
	ulong	sig_len = 0;			/* Length of signature text */
	ulong	code_len = 0;			/* Length of memory read */
	ulong	i, j;					/* Measly index variables */
	int		ret = 0;				/* Little 'ol return value */

    static char prev_signature_name[SHORTNAME] = "";
    static struct t_signature_data *p_signature;
	p_signature = (struct t_signature_data *)signature_data;


    if (strcmp(signature_name, prev_signature_name) != 0) {
		p_signature->name = _strdup(signature_name);
		StrcopyA(prev_signature_name, sizeof(prev_signature_name), signature_name);
        prev_signature_name[sizeof(prev_signature_name) - 1] = '\0';	/* Ensure NULL termination */
    }

	if (strcmp(property_name, "signature") == 0) {
		p_signature->data = _strdup(value);
	} else if (strcmp(property_name, "ep_only") == 0) {
		p_signature->ep_signature = (strcmp(value, "true") == 0) ? TRUE : FALSE;

		//:) I LOVE YOU
		/*
		 * Do comparison here
		 */

		/* Get length of signature so we know how much memory to read */
		remove_char(p_signature->data, ' ');
		sig_len = strlen(p_signature->data);

		/* We're always going to search the Entry Point part of the code whether we */
		/* check Scan EP only or not. If we scan the entire file, the entry point is still */
		/* part of that file... */
		if (p_signature->ep_signature == TRUE) {
			/* Only test the bytes at the module Entry Point */

			/* Calculate the EP offset relative to the begenning of the code base */
			/* Note: We have to multiply by 2 because each uchar in memory became two char's */
			codebase_ep_offset =  (main_module->entry - main_module->codebase) * 2;

			ret = SIG_FOUND;	/* Assume this signature will match until proven otherwise */
			/* Compare signature bytes to code starting at OEP */
			for (i = 0; i < sig_len; i++) {
				if ((module_code[codebase_ep_offset + i] != p_signature->data[i]) &&
						p_signature->data[i] != '?') {
					/* Mismatch */
					ret = SIG_NO_MATCH;
					break;		/* Exit for loop because we don't have a match */
				}
			}
		} else if ((scan_ep_only == 0) && (p_signature->ep_signature == FALSE)) {
			/*Start scanning from beginning of module */

			/*
			 * Brute force searching technique. Other methods might be better, but
			 * for this implementation it doesn't really matter much. There just isn't
			 * enough data to search through I don't think.
			 */
			for (i = 0; i < main_module->codesize * 2; i++) {
				/* Always assume we'll find a match, until we don't */
				ret = SIG_FOUND;

				/* Compare signature bytes to code starting at module loction i */
				for (j = 0; j < sig_len; j++) {
					if ((module_code[j + i] != p_signature->data[j]) &&
							p_signature->data[j] != '?') {
						/* Mismatch */
						ret = SIG_NO_MATCH;
						break;		/* Exit for loop because we don't have a match */
					}
				}

				/* Found a match. No need to search anymore */
				if (ret == SIG_FOUND)
					break;
			}
		}
	} else {
		ret = 0;	/* unknown section/name, error */
	}

	return ret;
}

int scan_module(void)
{
	FILE	*signature_file = NULL;		/* Pointer to database file */
	wchar_t	signature_name[TEXTLEN];	/* Name of signature from database file */
	BOOL	sigs_match = TRUE;			/* Flag FALSE if signatures don't match */
	int		ret;						/* Return values for certain functions */
#if DEVELOPMENT_MODE
	long	start_time, end_time;
#endif
    struct t_signature_data sig_data;

	Resumeallthreads();

	/* Get the main memory module so we can use the OEP */
	main_module = Findmainmodule();

	/* Make sure there is actually a module loaded into Olly */
	if (main_module != NULL) {
		if (scan_ep_only == 1) {
			Addtolist(0, DRAW_HILITE, L"[*] Scanning EP only");
		} else {
			Addtolist(0, DRAW_HILITE, L"[*] Scanning entire file");
		}

#if DEVELOPMENT_MODE
		start_time = clock();
#endif

		/* Allocate and read module memory into buffer */
		module_mem_create();

		/* Initiate the parsing! */
		ret = parse_database(database_path, find_signature_helper, &sig_data);

#if DEVELOPMENT_MODE
		end_time = clock();
#endif
		if (ret == SIG_FOUND) {
			Asciitounicode(sig_data.name , DATALEN, signature_name, DATALEN);
			Addtolist(0, DRAW_HILITE, L"[+] %s", signature_name);
#if DEVELOPMENT_MODE
			Addtolist(0, DRAW_HILITE, L"Total time: %ldms", end_time - start_time);
#endif
			MessageBox(hwollymain, signature_name, L"OllyID", MB_OK | MB_ICONINFORMATION);
			ret = 0;
		} else if (ret == SIG_NO_MATCH) {
			Addtolist(0, DRAW_HILITE, L"[!] Nothing Found");
#if DEVELOPMENT_MODE
			Addtolist(0, DRAW_HILITE, L"Total time: %ldms", end_time - start_time);
#endif
			MessageBox(hwollymain, L"Nothing found", L"OllyID", MB_OK | MB_ICONINFORMATION);
			ret = 1;
		} else if (ret == -1) {
        	Addtolist(0, DRAW_HILITE, L"[!] Could not open signature database: %s", database_path);
			ret = 2;
		} else {//if (ret > 0) {
			Addtolist(0, DRAW_HILITE, L"[!] Bad config file. First error on line %d", ret);
			return 3;
		}
	} else {		/* No module loaded */
		Addtolist(0, DRAW_HILITE, L"[!] No module loaded");
	}

	Suspendallthreads();

	/* Free resources */
	if (signature_file != NULL)
		fclose(signature_file);

	return ret;
}

int module_mem_create(void)
{
	uchar *code_buf = NULL;
	ulong code_len;

	if (module_code == NULL) {
		/* Allocate and read memory for the module code to be stored for faster searching */
		code_buf = (uchar *)Memalloc(main_module->codesize, REPORT|ZEROINIT);
		code_len = Readmemory((uchar *)code_buf, main_module->codebase, main_module->codesize, MM_SILENT);
		/* Convert byte codes to a UNICODE string */
		module_code = (char *)Memalloc((code_len * 2) + 1, REPORT|ZEROINIT);	/* Include +1 for \0 */
		HexdumpA(module_code, code_buf, code_len);

		/* Free the code buffer */
		if (code_buf != NULL) {
			Memfree(code_buf);
		}
	}
	return 1;
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
		plugin_instance = hinstDll;		/* Save plugin instance */
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
	/*
	 * Get initialization data
	 */

	load_settings(NULL);

	Addtolist(0, DRAW_NORMAL, L"");
	Addtolist(0, DRAW_NORMAL, L"[*] %s - v%s", PLUGIN_NAME, PLUGIN_VERS);
	Addtolist(0, DRAW_NORMAL, L"[*] Coded by: Austyn Krutsinger <akrutsinger@gmail.com>");

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
	if (scan_on_analysis == 1) {
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
	/* Free memory allocated for module code bytes */
	if (module_code != NULL) {
		Memfree(module_code);
	}
};

/*
 * Function is called when user opens new or restarts current application.
 * Plugin should reset internal variables and data structures to the initial
 * state.
 */
//extc void __cdecl ODBG2_Pluginreset(void)
//{
//	/* Possibly scan module here? */
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
//extc void __cdecl ODBG2_Pluginnotify(int code, void *data, ulong parm1, ulong parm2)
//{
//};

