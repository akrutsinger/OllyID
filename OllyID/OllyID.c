////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                  SAMPLE BOOKMARK PLUGIN FOR OLLYDBG v2.01                  //
//                                                                            //
// This plugin allows you to scan a debugged application with the ID          //
// database (userdb.txt). It will compare the signature of the loaded module  //
// and print the results of the scan for you to quickly identify the PE you   //
// are debugging.                                                             //
//                                                                            //
// This code is distributed "as is", without warranty of any kind, expressed  //
// or implied, including, but not limited to warranty of fitness for any      //
// particular purpose. In no event will Austyn Krutsiinger be liable to you   //
// for any special, incidental, indirect, consequential or any other damages  //
// caused by the use, misuse, or the inability to use of this code, including //
// any lost profits or lost savings, even if Oleh Yuschuk has been advised of //
// the possibility of such damages. Or, translated into English: use at your  //
// own risk!                                                                  //
//                                                                            //
// This code is free. You can modify it, include parts of it into your own    //
// programs and redistribute modified code provided that you remove all       //
// copyright messages or substitute them with your own copyright.             //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// VERY IMPORTANT NOTICE: PLUGINS ARE UNICODE LIBRARIES! COMPILE THEM WITH BYTE
// ALIGNMENT OF STRUCTURES AND DEFAULT UNSIGNED CHAR!

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
 *
 * Version 0.0.3-alpha
 * [+] Improved scan functionality
 *
 *
 * Version 0.0.2-alpha
 * [+] Added ScanFile routine to search database
 *
 * Version 0.0.1-alpha
 * [+] Base code
 *
 *
 * -----------------------------------------------------------------------------
 * TODO
 * -----------------------------------------------------------------------------
 * [ ] Better analysis on signature database in case formatting is a little off
 * [ ] Standardize naming conventions
 * [?] Add OllyID Log window
 * [ ] Improve error checking for most everything
 * [ ] Add Options window
 * [ ] Assumes ep_only = true. Automatically determine from Options menu
 * [ ] Currently only scans from EP only. Add ability to scan entire module
 * [?] Option: Scan On Analyse
 * [ ] Option: Scan On Module Load
 * [ ] Option: specify max_sig_len
 * [ ] Restructure project files layout and build locations et cetera
 * [ ] Download datase from: http://www.abysssec/AbysssecDb/Database.TXT
 *
 ******************************************************************************/


#include "OllyID.h"


////////////////////////////////////////////////////////////////////////////////
////////////////// PLUGIN MENUS EMBEDDED INTO OLLYDBG WINDOWS //////////////////

/**
 * @ShowAboutMessage
 *
 *		Displays "About" message box
 */
void DisplayAboutMessage(void)
{
	wchar_t wszAboutMessage[TEXTLEN] = { 0 };
	int n;
    // Debuggee should continue execution while message box is displayed.
	Resumeallthreads();
	// In this case, swprintf() would be as good as a sequence of StrcopyW(),
	// but secure copy makes buffer overflow impossible.
	n = StrcopyW(wszAboutMessage, TEXTLEN, L"OllyID plugin v");
	n += StrcopyW(wszAboutMessage + n, TEXTLEN - n, PLUGIN_VERS);
	n += StrcopyW(wszAboutMessage + n, TEXTLEN - n, L"\n\ncoded by: Austyn Krutsinger");
	n += StrcopyW(wszAboutMessage + n, TEXTLEN - n, L"\ncontact me: akrutsinger@gmail.com");
	// The conditionals below are here to verify that this plugin can be
	// compiled with all supported compilers. They are not necessary in the
	// final code.
	#if defined(__BORLANDC__)
		n += StrcopyW(wszAboutMessage + n, TEXTLEN - n, L"\n\nCompiled with Borland (R) ");
	#elif defined(_MSC_VER)
		n += StrcopyW(wszAboutMessage + n, TEXTLEN - n, L"\n\nCompiled with Microsoft (R) ");
	#elif defined(__MINGW32__)
		n += StrcopyW(wszAboutMessage + n, TEXTLEN - n, L"\n\nCompiled with MinGW32 ");
	#else
		n += StrcopyW(wszAboutMessage + n, TEXTLEN - n, L"\n\nCompiled with ");
	#endif
	#ifdef __cplusplus
		StrcopyW(wszAboutMessage + n, TEXTLEN - n, L"C++ compiler");
	#else
		StrcopyW(wszAboutMessage + n, TEXTLEN - n, L"C compiler");
	#endif
	MessageBox(hwollymain, wszAboutMessage, L"OllyID plugin", MB_OK|MB_ICONINFORMATION);
	// Suspendallthreads() and Resumeallthreads() must be paired, even if they
	// are called in inverse order!
	Suspendallthreads();
}

/**
 * @menu_handler
 * 
 *      Menu callback for our plugin to process our menu commands.
 */
static int menu_handler(t_table* pTable, wchar_t* pName, ulong index, int nMode)
{
    UNREFERENCED_PARAMETER(pTable);
    UNREFERENCED_PARAMETER(pName);

    switch(nMode)
    {
    case MENU_VERIFY:
        return MENU_NORMAL;

    case MENU_EXECUTE:
        {
            switch(index)
            {
			case MENU_TEST:
				{
					

					break;
				}
			case MENU_LOG:	// Menu -> OllyPID Log
				{
					if(log_window.hw == NULL)
						// Create table window. Third parameter (ncolumn) is the number of
						// visible columns in the newly created window (ignored if appearance is
						// restored from the initialization file). If it's lower than the total
						// number of columns, remaining columns are initially invisible. Fourth
						// parameter is the name of icon - as OllyDbg resource.
						Createtablewindow(&log_window, 0, log_window.bar.nbar, g_plugin_instance, L"ICO_P", PLUGIN_NAME);
					else
						Activatetablewindow(&log_window);

					break;
				}
            case MENU_OPTIONS: // Menu -> OllyID Options
				{
				break;
				}
            case MENU_SCAN_FILE: // Disasm Menu -> Scan File
				{
					ScanFile();
					break;
				}
            case MENU_CREATE_SIG: // Disasm Menu -> Create Signature
				{
					break;
				}
            case MENU_ABOUT: // Menu -> About
                {
					DisplayAboutMessage();
                }
            }
            return MENU_NOREDRAW;
        }
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
		// Main menu.
		return g_vMainMenu;
	else if(wcscmp(type, PWM_DISASM) == 0)
		// Disassembler pane of CPU window.
		return g_vDisasmMenu;
	return NULL;                         // No menu
};

////////////////////////////////////////////////////////////////////////////////
///////////////////////////// FILE ROUTINES ////////////////////////////////////

int find_signature_helper(void* signature_block, const char* signature_name,
							const char* name, const char* value)
{
	uchar	code_buf[DATALEN];			/* Stores memory bytes from main module */
	uchar	hex_buf[DATALEN];			/* Buffer to store bytes converted from module in memory */
	ulong	sig_len;					/* Length of signature text */
	ulong	code_len;					/* Length of memory read */
	ulong	i, j;						/* Measly index variables */
	int		ret = TRUE;					/* Default value: means function did what it's supposed to */

    static char prev_signature_name[SHORTNAME] = "";
    t_signature_block *p_signature = (t_signature_block *)signature_block;


    if(strcmp(signature_name, prev_signature_name) != 0) {
		p_signature->name = _strdup(signature_name);
        strncpy(prev_signature_name, signature_name, sizeof(prev_signature_name));
        prev_signature_name[sizeof(prev_signature_name) - 1] = '\0';	/* Ensure NULL termination */
    }

	if(strcmp(name, "signature") == 0) {
		p_signature->data = _strdup(value);
	}else if(strcmp(name, "ep_only") == 0){
		p_signature->ep_only = (strcmp(value, "true") == 0) ? TRUE : FALSE;

		/*
		 * Do comparison here
		 */

		/* Get length of signature so we know how much memory to read */
		remove_char(p_signature->data, ' ');
		sig_len = strlen(p_signature->data);

		if(p_signature->ep_only == TRUE){
			/* Only test the bytes at the module Entry Point */

			/**
			 * TODO: Optimize this out of the handler so we only have to read the memory once
			 */
			 /* Note: code_len should always equal sig_len */
			code_len = Readmemory((uchar *)code_buf, main_module->entry, sig_len, MM_SILENT);
			/* Convert byte codes to a UNICODE string */
			HexdumpA(hex_buf, code_buf, code_len);

			ret = SIG_FOUND;	/* Assume this signature will match until proven otherwise */
			/* Compare signature bytes to code starting at OEP */
			for(i = 0; i < sig_len; i++) {
				if(p_signature->data[i] == '?'){
					continue;
				}else if(hex_buf[i] != p_signature->data[i]){
					/* Mismatch */
					ret = SIG_NOTHING;
					break;		/* Exit for loop because we don't have a match */
				}
			}



		}else if(p_signature->ep_only == FALSE){
			/*Start scanning from beginning of module */

			/**
			 * Make a special case for searching the entire module so we read the
			 * memory sig_len length chunks at a time
			 */
			//code_len = 0;
			//for(i = 0; i < main_module->size; i++){
			//	/* The very last read code_len may be less than sig_len */
			//	code_len = Readmemory((uchar *)code_buf, main_module->base + code_len, sig_len, MM_SILENT);
			//	/* Convert byte codes to a UNICODE string */
			//	HexdumpA(hex_buf, code_buf, code_len);

			//	/* Compare signature bytes to code starting at OEP */
			//	for(j = 0; j < sig_len; j++) {
			//		if(p_signature->data[j] == '?'){
			//			continue;
			//		}else if(hex_buf[j] != p_signature->data[j]){
			//			/* Mismatch */
			//			ret = SIG_NOTHING;
			//			break;		/* Exit for loop because we don't have a match */
			//		}
			//	}

			//	/* Found a match. No need to search anymore */
			//	if(ret == SIG_FOUND)
			//		break;
			//}

		}

//		if(ret == SIG_FOUND){
//			MessageBoxA(hwollymain, p_signature->name , "match found!", MB_OK);
//		}

	} else{
		ret = 0;	/* unknown section/name, error */
	}

	return ret;
}

int ScanFile(void)
{
	FILE	*fpSignatureFile = NULL;	// Pointer to database file
	wchar_t	wSignatureName[TEXTLEN];	// Name of signature from database file
//	wchar_t wbuf[SIGNATURE_MAX];		// Mainly for storing the signature after converted to UNICODE
//	wchar_t whex_buf[SIGNATURE_MAX];	// Buffer to store bytes converted from module in memory
//	wchar_t *wfile_signature;			// Pointer used to iterate during our comparison
	char	buf[DATALEN];				// ASCII text from the file before conversion
//	ulong	sig_len;					// Length of signature text
//	ulong	i;							// Index variable for loops
	BOOL	sigs_match = TRUE;			// Flag FALSE if signatures don't match
//	BOOL	ep_only;					// Flag for searching for signature from OEP or entire module
	int		ret;						// Return values for certain functions

//	uchar	code_buf[SIGNATURE_MAX];	// Stores memory bytes from main module
//	ulong	code_len;					// Length of memory read

    t_signature_block sig_data;

	Resumeallthreads();

	/* Get the main memory module so we can use the OEP */
	main_module = Findmainmodule();

	/* Make sure there is actually a module loaded into Olly */
	if(main_module != NULL){
		/* Convert UNICODE filename to char so we can open the file */
		Unicodetoascii(g_signature_filename, MAXPATH, buf, MAXPATH);
		
		/* Initiate the parsing! */
		ret = database_parse(buf, find_signature_helper, &sig_data);
	
		if(ret == SIG_FOUND){
			Asciitounicode(sig_data.name , DATALEN, wSignatureName, DATALEN);
			Addtolist(0, DRAW_HILITE, L"[+] %s", wSignatureName);
			MessageBox(hwollymain, wSignatureName, L"OllyID", MB_OK | MB_ICONINFORMATION);
		}else if(ret == SIG_NOTHING){
			Addtolist(0, DRAW_HILITE, L"[!] Nothing Found");
			return SIG_NOTHING;
		}else if(ret == -1) {
        	Addtolist(0, DRAW_HILITE, L"[!] Could not open signature database: %s", g_signature_filename);
			return 2;
		}else{//if(ret > 0){
			Addtolist(0, DRAW_HILITE, L"[!] Bad config file. First error on line %d", ret);
			return 3;
		}
	}else{		// No module loaded
		//add_to_log(0, DRAW_HILITE, L"[!] No module loaded");
		Addtolist(0, DRAW_HILITE, L"[!] No module loaded");
	}

	Suspendallthreads();












//	/* Convert UNICODE filename from .ini to char so we can open the file */
//	Unicodetoascii(g_signature_filename, TEXTLEN, buf, TEXTLEN);	// Convert filename so we can open it
//	fpSignatureFile = fopen(buf, "r");	/* Open the signature database */
//
//	if(fpSignatureFile != NULL) {
//		/* Get the main memory module so we can use the OEP */
//		pMainModule = Findmainmodule();
//
//		/* Make sure there is actually a module loaded into Olly */
//		if(pMainModule != NULL){
//			/**
//			 * Parse database 4 lines at a time
//			 * We make 4 calls during the loop for Signature name, signature bytes,
//			 * ep_only, and the blank line inbetween signature sections
//			 */
//
//			/* Read module memory for code to compare with signature */
//			/* Do this once outside the loop so we don't waste cycles */
//			code_len = Readmemory((uchar *)code_buf, pMainModule->entry, SIGNATURE_MAX, MM_SILENT);
//			/* Convert byte codes to a UNICODE string */
//			HexdumpW(whex_buf, code_buf, code_len);
//
//			do {
//				/**
//				 * Name of compiler / packer / protector
//				 */		
//				ret = get_line(fpSignatureFile, buf, SIGNATURE_MAX);
//				/* Convert the input file buffer (char) to unicode buffer */
//				Asciitounicode(buf, SIGNATURE_MAX, wbuf, SIGNATURE_MAX);
//				sig_len = StrlenW(wbuf, TEXTLEN);
//
//				/* Program / packer / encryptor name */
//				if(wbuf[0] == L'[' && wbuf[sig_len - 1] == L']') {
//					/* Remove leading '[' and trailing ']' and copy name to Signatures struct */
//					StrcopyW(wSignatureName, sig_len - 1, wbuf + 1);
//				}
//							
//				/**
//				 * Signature bytes
//				 */
//				ret = get_line(fpSignatureFile, buf, SIGNATURE_MAX);
//				/* Convert the input file buffer (char) to unicode buffer */
//				Asciitounicode(buf, SIGNATURE_MAX, wbuf, SIGNATURE_MAX);
//				if(strcmp(buf, "signature") == TRUE) {
//					/* Remove spaces from signatre from file */
//					unicode_remove_char(wbuf, L' ');
//					sig_len = StrlenW(wbuf + 10, SIGNATURE_MAX);
//
//					/* Compare signature bytes to code starting at OEP */
//					wfile_signature = wbuf + 10;		// Set the start of our file sig buffer after "signature="
//					sigs_match = TRUE;					// Reset to TRUE for each signature test
//					for(i = 0; i < sig_len; i++) {
//						if(wfile_signature[i] == L'?'){
//							continue;
//						}else if(whex_buf[i] != wfile_signature[i]){
//							/* Mismatch */
//							sigs_match = FALSE;
//							break;		// Exit for loop because we don't have a match
//						}
////						*wfile_signature++;		// Increment file signature point to stay aligned with whex_buf
//					}
//
//					/* Check if we've found a match */
//					if(sigs_match == TRUE) {
//						break;	// Exit while loop so we don't waste CPU cycles
//					}
//				}
//
//				/**
//				 * Determine if we are search from Entry Point or entire file
//				 */
//				ret = get_line(fpSignatureFile, buf, SIGNATURE_MAX);
//				if(strcmp(buf, "ep_only") == TRUE) {
//					if(strcmp(buf + 8, "true")){
//						ep_only = TRUE;
//					} else if(strcmp(buf + 8, "false")){
//						ep_only = FALSE;
//					}
//				}
//
//				/**
//				 * Get empty line between signature sets
//				 */
//				ret = get_line(fpSignatureFile, buf, SIGNATURE_MAX);
//
//			} while(ret != EOF);
//
//			/* Report if we found a match */
//			if(sigs_match == TRUE){
//				//add_to_log(0, DRAW_HILITE, L"[+] %s", wSignatureName);
//				Addtolist(0, DRAW_HILITE, L"[+] %s", wSignatureName);
//				MessageBox(hwollymain, wSignatureName, L"OllyID", MB_OK | MB_ICONINFORMATION);
//
//			}else{	// No matches found
//				//add_to_log(0, DRAW_HILITE, L"[!] Nothing Found", i);
//				Addtolist(0, DRAW_HILITE, L"[!] Nothing Found", i);
//			}
//		}else{		// No module loaded
//			//add_to_log(0, DRAW_HILITE, L"[!] No module loaded");
//			Addtolist(0, DRAW_HILITE, L"[!] No module loaded");
//		}
//
//	}else {
//		//add_to_log(0, DRAW_HILITE, L"[!] Could not open signature database: %s", g_vSignatureFilename);
//		Addtolist(0, DRAW_HILITE, L"[!] Could not open signature database: %s", g_signature_filename);
//	}
//	
//	Suspendallthreads();			

	/* Free resources */
	if(fpSignatureFile != NULL)
		fclose(fpSignatureFile);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
///////////////////////////// WINDOW STUFF /////////////////////////////////////

void add_to_log(ulong addr, int color, wchar_t *format, ...)
{
	va_list args;

	va_start(args, format);
	add_to_logw(addr, color, format, args);
	va_end(args);
}

void add_to_logw(ulong addr, int color, wchar_t *format, va_list args)
{
	wchar_t buf[TEXTLEN];
	t_log_data log_data;

    log_data.addr = addr;		/* Address to display */
    log_data.size = 1;			/* Only adding 1 entry */
    log_data.type = TY_NEW;		/* Adding a new item */

	vswprintf(buf, TEXTLEN, format, args);
	StrcopyW(log_data.msg, TEXTLEN, buf);	/* Copy buffer to log_data */

    Addsorteddata(&(log_window.sorted), &log_data);	/* Add the data to our log window */
	invalidate_log_window();
}

void invalidate_log_window(void)
{
	if(log_window.hw != NULL)
		InvalidateRect(log_window.hw, NULL, FALSE);
}

/* Sorting function for Log window. Not needed, just pro forma */
int cdecl log_window_sort_func(const t_sorthdr *sh1, const t_sorthdr *sh2, const int n)
{
	return 0;	/* Items are equal */
}

/* Destructor for Log window. Not needed, just pro forma */
void cdecl log_window_dest_func(t_sorthdr *p)
{
}

/* Drawing function for Log window */
int log_window_draw_func(wchar_t *s, uchar *mask, int *select, struct t_table *pt, t_sorthdr *ph, int column, void *cache)
{
	int ret;
	t_log_data *p_log_data;
	p_log_data = (t_log_data *)ph;

	switch (column) {
		case DF_CACHESIZE:                 // Request for draw cache size
			// Columns 3 and 4 (disassembly and comment) both require calls to
			// Disasm(). To accelerate processing, I call disassembler once per line
			// and cache data between the calls. Here I inform the drawing routine
			// how large the cache must be.

			return sizeof(t_log_data);
		case DF_FILLCACHE:                 // Request to fill draw cache
			// We don't need to initialize cache when drawing begins. Note that cache
			// is initially zeroed.
			return 0;
		case DF_FREECACHE:                 // Request to free cached resources
			// We don't need to free cached resources when drawing ends.
			return 0;
		case DF_NEWROW:                    // Request to start new row in window
			// New row starts. Let us disassemble the command at the pointed address.
			// I assume that bookmarks can't be set on data. First of all, we need to
			// read the contents of memory. Length of 80x86 commands is limited to
			// MAXCMDSIZE bytes.
			return 0;
		case 0:		/* Address column */
		{
			ret = Simpleaddress(s, p_log_data->addr, mask, select);
			break;
		}
		case 1:		/* Message column */
		{
			ret = wsprintf(s, L"%s", p_log_data->msg);
			break;
		}
	}
	return ret;
}

// Custom table function of plugin window. Here it is used only to process
// doubleclicks (custom message WM_USER_DBLCLK). This function is also called
// on WM_DESTROY, WM_CLOSE (by returning -1, you can prevent window from
// closing), WM_SIZE (custom tables only), WM_CHAR (only if TABLE_WANTCHAR is
// set) and different custom messages WM_USER_xxx (depending on table type).
// See documentation for details.

long cdecl log_window_table_func(t_table *pt, HWND hw, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
		case WM_USER_DBLCLK:               // Doubleclick
		{
			return 1;
		}
		default: break;
	};
	return 0;
};

////////////////////////////////////////////////////////////////////////////////
//////////////////////////// PLUGIN INITIALIZATION /////////////////////////////

/**
 * @InitializeLogWindow
 *
 *		Initialize log windows for OllyID to print information to
 *
 *		Returns: Handle to table window
 */ 
HWND initialize_log_window(void)
{
	HWND ret;	/* Handle returned */

	// Initialize bookmark table. OllyDbg uses table name to save the appearance
	// to the main initialization file. Keep this name unique, or else.
	StrcopyW(log_window.name, SHORTNAME, PLUGIN_NAME);

	log_window.bar.visible = 1;              // By default, bar is visible

	log_window.bar.name[0] = L"Address";
	log_window.bar.expl[0] = L"OEP or start of signature address";
	log_window.bar.mode[0] = BAR_FLAT;
	log_window.bar.defdx[0] = 9;

	log_window.bar.name[1] = L"Message";
	log_window.bar.expl[1] = L"OllyID Log Messages";
	log_window.bar.mode[1] = BAR_FLAT;
	log_window.bar.defdx[1] = 50;
 
	log_window.bar.nbar = 2;
	log_window.mode = TABLE_SAVEALL;		// Save complete appearance; TABLE_COPYMENU|TABLE_APPMENU|TABLE_SAVEPOS|TABLE_ONTOP|TABLE_HILMENU;
	log_window.drawfunc = (DRAWFUNC *)log_window_draw_func;
	log_window.tabfunc = log_window_table_func;
	log_window.custommode = 0;
	log_window.customdata = NULL;
	log_window.updatefunc = NULL;
	log_window.tableselfunc = NULL;

	log_window.menu = g_vLogMenu;

	ret = Createtablewindow(&log_window, 0, log_window.bar.nbar, g_plugin_instance, L"ICO_P", PLUGIN_NAME);

	if(log_window.hw != NULL) {
    	Activatetablewindow(&log_window);
		//HINSTANCE hinst = (HINSTANCE)GetModuleHandleW(g_vPluginName, L".dll");
		//HICON ico = LoadIcon(hinst, MAKEINTRESOURCE(IDI_ICON_LOG)); 
		//SendMessage(log_window.hw, WM_SETICON, false, (long)ico);
	}
	return ret;
}

/**
 * @DllMain
 * 
 *      Dll entrypoint - mainly unused.
 */
BOOL WINAPI DllEntryPoint(HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpReserved)
{
	if(fdwReason == DLL_PROCESS_ATTACH)
		g_plugin_instance = hinstDll;	// Mark plugin instance
	return 1;							// Report success
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
	int restore, n;

	/* Initialize log window storage. */

	/*****
	 * USE Createsimpledata
	 ****/
	if(Createsorteddata(&(log_window.sorted),				/* Descriptor of sorted data */
						sizeof(t_log_data),					/* Size of single data item */
						10,									/* Initial number of allocated items */
						(SORTFUNC *)log_window_sort_func,	/* Sorting function */
						(DESTFUNC *)log_window_dest_func,	/* Data destructor */
						0) != 0){							/* Simple data, no special options */
		Addtolist(0, DRAW_HILITE, L"[!] Failed to initialize log window storage");
		return -1;	/* Low memory? */
	}

	/* Get initialization data. */
	n = StrcopyW(g_signature_filename, MAXPATH, plugindir);
	n += StrcopyW(g_signature_filename + n, MAXPATH - n, L"\\userdb.txt"); /* Default value */
	//Getfromini(NULL, PLUGIN_NAME, L"SignatureFilename",L"%ls", g_vSignatureFilename);

	/*
	 * OllyDbg saves positions of plugin windows with attribute TABLE_SAVEPOS to
	 * the .ini file but does not automatically restore them. Let us add this
	 * functionality here. To conform to OllyDbg norms, window is restored only
	 * if corresponding option is enabled.
	 */
	if(restorewinpos != 0){
		restore = 0;                         /* Default */
		Getfromini(NULL,PLUGIN_NAME,L"Restore window", L"%i", &restore);
		if(restore)
			Createtablewindow(&log_window, 0, log_window.bar.nbar, NULL, L"ICO_P", PLUGIN_NAME);
	};


	Addtolist(0, DRAW_NORMAL, L"");
	Addtolist(0, DRAW_NORMAL, L"[*] %s - v%s", PLUGIN_NAME, PLUGIN_VERS);
	Addtolist(0, DRAW_NORMAL, L"[*] coded by: Austyn Krutsinger");
	Addtolist(0, DRAW_NORMAL, L"[*] contact me: akrutsinger@gmail.com");

	if(initialize_log_window() != NULL){	/* Create the Log window */
		Addtolist(0, DRAW_NORMAL, L"");
		Addtolist(0, DRAW_NORMAL, L"[+] Library initialized");
	}else{
		Addtolist(0, DRAW_HILITE, L"[!] Could not create log window");
	}
	/* Report success. */
	return 0;
};

// Function is called when user opens new or restarts current application.
// Plugin should reset internal variables and data structures to the initial
// state.
extc void __cdecl ODBG2_Pluginreset(void)
{
	Deletesorteddatarange(&(log_window.sorted), 0,0xFFFFFFFF);
};

// OllyDbg calls this optional function when user wants to terminate OllyDbg.
// All MDI windows created by plugins still exist. Function must return 0 if
// it is safe to terminate. Any non-zero return will stop closing sequence. Do
// not misuse this possibility! Always inform user about the reasons why
// termination is not good and ask for his decision! Attention, don't make any
// unrecoverable actions for the case that some other plugin will decide that
// OllyDbg should continue running.
extc int __cdecl ODBG2_Pluginclose(void)
{
	// For automatical restoring of open windows, mark in .ini file whether
	// Bookmarks window is still open.
	Writetoini(NULL, PLUGIN_NAME, L"Restore window", L"%i", log_window.hw != NULL);
	/* Save database path */
	//Writetoini(NULL, PLUGIN_NAME, L"SignatureFilename", L"%ls", g_vSignatureFilename);
	return 0;
};

// OllyDbg calls this optional function once on exit. At this moment, all MDI
// windows created by plugin are already destroyed (and received WM_DESTROY
// messages). Function must free all internally allocated resources, like
// window classes, files, memory etc.
extc void __cdecl ODBG2_Plugindestroy(void)
{
	Destroysorteddata(&(log_window.sorted));
};


////////////////////////////////////////////////////////////////////////////////
/////////////////////////// EVENTS AND NOTIFICATIONS ///////////////////////////

// If you define ODBG2_Pluginmainloop(), this function will be called each time
// from the main Windows loop in OllyDbg. If there is some (real) debug event
// from the debugged application, debugevent points to it, otherwise it's NULL.
// If fast command emulation is active, it does not receive all (emulated)
// exceptions, use ODBG2_Pluginexception() instead. Do not declare these two
// functions unnecessarily, as this may negatively influence the overall speed!
//
// extc void __cdecl ODBG2_Pluginmainloop(DEBUG_EVENT *debugevent) {
// };
//
// extc void __cdecl ODBG2_Pluginexception(t_run *prun,t_thread *pthr,
//   t_reg *preg) {
// };
//
// Optional entry, called each time OllyDbg analyses some module and analysis
// is finished. Plugin can make additional analysis steps. Debugged application
// is paused for the time of processing. Bookmark plugin, of course, does not
// analyse code. If you don't need this feature, remove ODBG2_Pluginanalyse()
// from the plugin code.
//extc void __cdecl ODBG2_Pluginanalyse(t_module *pmod)
//{
//	ScanFile();
//};

// Optional entry, notifies plugin on relatively infrequent events.
//extc void __cdecl ODBG2_Pluginnotify(int code, void *data, ulong parm1, ulong parm2)
//{
//};

