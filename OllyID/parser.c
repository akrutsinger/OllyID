/* inih -- simple .INI file parser

inih is released under the New BSD license (see LICENSE.txt). Go to the project
home page for more info:

http://code.google.com/p/inih/

*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>		/* For plugin.h */
//#include <stdio.h>
//#include <ctype.h>
//#include <string.h>

#include "parser.h"
#include "string.h"
#include "plugin.h"		/* StrcopyA */

#define MAX_SECTION 50
#define MAX_NAME 50

/* See documentation in header file. */
int parse_database_file(FILE* file,
                   int (*handler)(void*, const char*, const char*, const char*),
                   void* user)
{
    /* Uses a fair bit of stack (use heap instead if you need to) */
#if USE_STACK
    char line[MAX_LINE_LEN];
#else
    char* line;
#endif
    char section[MAX_SECTION] = "";
    char prev_name[MAX_NAME] = "";

    char* start;
    char* end;
    char* name;
    char* value;
    int lineno = 0;
    int error = 0;
	int ret = 0;

#if !USE_STACK
	line = (char*)malloc(MAX_LINE_LEN);
	if (!line) {
		return -2;
	}
#endif

    /* Scan through file line by line */
    while (fgets(line, MAX_LINE_LEN, file) != NULL) {
        lineno++;

        start = line;
#if ALLOW_BOM
        if (lineno == 1 && (unsigned char)start[0] == 0xEF &&
                           (unsigned char)start[1] == 0xBB &&
                           (unsigned char)start[2] == 0xBF) {
            start += 3;
        }
#endif
        start = lskip(rstrip(start));

        if (*start == ';' || *start == '#') {
            /* Per Python ConfigParser, allow '#' comments at start of line */
        }
#if ALLOW_MULTILINE
        else if (*prev_name && *start && start > line) {
            /* Non-black line with leading whitespace, treat as continuation
               of previous name's value (as per Python ConfigParser). */
//			if (handler(user, section, prev_name, start) == 0 && !error)
//				error = lineno;




        }
#endif
        else if (*start == '[') {
            /* A "[section]" line */
            end = find_char_or_comment(start + 1, ']');
            if (*end == ']') {
                *end = '\0';
				StrcopyA(section, sizeof(section), start + 1);
                *prev_name = '\0';
            }
            else if (!error) {
                /* No ']' found on section line */
                error = lineno;
            }
        }
        else if (*start && *start != ';') {
            /* Not a comment, must be a name[=:]value pair */
            end = find_char_or_comment(start, '=');
            if (*end != '=') {
                end = find_char_or_comment(start, ':');
            }
            if (*end == '=' || *end == ':') {
                *end = '\0';
                name = rstrip(start);
                value = lskip(end + 1);
                end = find_char_or_comment(value, '\0');
                if (*end == ';')
                    *end = '\0';
                rstrip(value);

                /* Valid name[=:]value pair found, call handler */
				StrcopyA(prev_name, sizeof(prev_name), name);
				ret = handler(user, section, name, value);
                if (ret == SIG_FOUND) {
					error = SIG_FOUND;
					break;
				} else if (ret == SIG_NO_MATCH) {
					error = SIG_NO_MATCH;
				} else if (ret > 0 && !error) {
                    error = lineno;
				}
            }
            else if (error == 0) {
                /* No '=' or ':' found on name[=:]value line */
                error = lineno;
            }
        }
    }

#if !USE_STACK
	free(line);
#endif

    return error;
}

/* See documentation in header file. */
int parse_database(const char* filename,
              int (*handler)(void*, const char*, const char*, const char*),
              void* user)
{
	FILE* file;
	int ret;

	file = fopen(filename, "r");
	if (!file)
		return -1;
	ret = parse_database_file(file, handler, user);
	fclose(file);
	return ret;
}
