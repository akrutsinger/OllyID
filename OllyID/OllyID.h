#ifndef _OLLYID_H_
#define _OLLYID_H_

#define PLUGIN_NAME		L"OllyID"		/* Unique plugin name */
#define PLUGIN_VERS		L"0.1.0"		/* Plugin version (stable . update . patch  - status) */

/* Menu items */
#define	MENU_LOG_WINDOW		1
#define MENU_SETTINGS		2
#define	MENU_ABOUT			3
#define	MENU_SCAN_MODULE	4
#define	MENU_CREATE_SIG		5

/**
 * Forward declarations
 */
 /* Menu functions */
int menu_handler(t_table* pTable, wchar_t* pName, ulong index, int nMode);
void display_about_message(void);
/* File functions */
int scan_module(void);
/* Window functions */
HWND initialize_log_window(void);
int cdecl log_window_sort_func(const t_sorthdr *sh1, const t_sorthdr *sh2, const int n);
void cdecl log_window_dest_func(t_sorthdr *p);
int log_window_draw_func(wchar_t *s, uchar *mask, int *select, struct t_table *pt, t_sorthdr *ph, int column, void *cache);
long cdecl log_window_table_func(t_table *pt, HWND hw, UINT msg, WPARAM wp, LPARAM lp);
void invalidate_log_window(void);
/* Helper functions */
void add_to_log(ulong addr, int color, wchar_t *format, ...);
void add_to_logw(ulong addr, int color, wchar_t *format, va_list args);
int find_signature_helper(void* signature_block, const char* signature_name, const char* name, const char* value);

/**
 * Globals
 */
HINSTANCE		plugin_instance;				/* Instance of plugin DLL */
static t_table	log_window;						/* OllyID table */
ulong			*entry_point;					/* Address of modules entry point */
ulong			*module_base;					/* Address for modules lowest memory */
t_module		*main_module;					/* Pointer to main module struct */


// Most of OllyDbg windows are the so called tables. A table consists of table
// descriptor (t_table) with embedded sorted data (t_table.sorted, unused in
// custom tables). If data is present, all data elements have the same size and
// begin with a 3-dword t_sorthdr: address, size, type. Data is kept sorted by
// address (in our case this is the index of the bookmark), overlapping is not
// allowed. Our bookmark table consists of elements of type t_bookmark.
typedef struct t_log_data {
	// Obligatory header, its layout _must_ coincide with t_sorthdr!
	ulong		addr;				// Base address of the entry
	ulong		size;				// Size of index, always 1 in our case
	ulong		type;				// Type of entry, TY_xxx
	// Custom data follows header.
	wchar_t		msg[TEXTLEN];		// Log Message
} t_log_data;

typedef struct t_signature_block {
    char	*name;
    char	*data;
    int		ep_signature;
} t_signature_block;

/*
 * Plugin menu that will appear in the main OllyDbg menu
 * and in popup menu.
 */
static t_menu ollyid_menu[] =
{
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
	// End of menu.
	{ NULL, NULL, K_NONE, NULL, NULL, 0 }
};

#endif /* _OLLYID_H_ */