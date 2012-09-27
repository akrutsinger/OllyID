#include "helper.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////// HELPER FUNCTIONS ////////////////////////////////

extern void remove_char(char *s, char c)
{
	char *p;
	for(p = s; *p = *s; p += (*s++ != c)){}
}

extern void unicode_remove_char(wchar_t *s, wchar_t c)
{
	wchar_t *p;
	for(p = s; *p = *s; p += (*s++ != c)){}
}

extern int get_line(FILE *fp, char *buffer, size_t buflen)
{
	char *end = buffer + buflen - 1;	/* Allow space for null terminator */
	char *dst = buffer;
	int c;
	while((c = fgetc(fp)) != EOF && c != '\n' && dst < end){
		*dst++ = c;
	}
	*dst = '\0';	/* Make sure string is null terminated */

	/* Return length of line */
	return ((c == EOF && dst == buffer) ? EOF : dst - buffer);
}