/*******************************************************************************
 * OllyID - string.c
 *
 * Copyright (c) 2012, Austyn Krutsinger
 * All rights reserved.
 *
 * OllyID is released under the New BSD license (see LICENSE.txt).
 *
 ******************************************************************************/

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <stdio.h>		/* isspace() */

#include "plugin.h"
#include "string.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////// HELPER FUNCTIONS ////////////////////////////////


char *cstrndup(const char *src, int max_len)
{
	/* Allocate memory */
	char *dst = (char *)Memalloc(StrlenA(src, max_len) * sizeof(char) + 1, SILENT|ZEROINIT);
	if (dst != NULL)
		StrcopyA(dst, max_len, src);	/* Copy string if allocated successfully */
	return dst;		/* Return new memory */
}

/**
 * strnicmp - Case insensitive, length-limited string comparison
 * @s1: One string
 * @s2: The other string
 * @len: the maximum number of characters to compare
 */
extern int strncmp2(const char *s1, const char *s2, size_t len)
{
	unsigned char c1, c2;

	if (!len)
		return 0;

	do {
		c1 = *s1++;
		c2 = *s2++;
		if (!c1 || !c2)
			break;
		if (c1 == c2)
			continue;
		c1 = tolower(c1);
		c2 = tolower(c2);
		if (c1 != c2)
			break;
	} while (--len);
	return (int)c1 - (int)c2;
}

extern void remove_char(char *s, char c)
{
	char *p;
	for (p = s; *p = *s; p += (*s++ != c)) {}
}

extern void unicode_remove_char(wchar_t *s, wchar_t c)
{
	wchar_t *p;
	for (p = s; *p = *s; p += (*s++ != c)) {}
}

extern int get_line(FILE *fp, char *buffer, size_t buflen)
{
	char *end = buffer + buflen - 1;	/* Allow space for null terminator */
	char *dst = buffer;
	int c;
	while((c = fgetc(fp)) != EOF && c != '\n' && dst < end) {
		*dst++ = c;
	}
	*dst = '\0';	/* Make sure string is null terminated */

	/* Return length of line */
	return ((c == EOF && dst == buffer) ? EOF : dst - buffer);
}

/* Strip whitespace chars off end of given string, in place. Return s. */
extern char* rstrip(char* s)
{
	char* p = s + StrlenA(s, DATALEN);
	while (p > s && isspace(*--p))
		*p = '\0';
	return s;
}

/* Return pointer to first non-whitespace char in given string. */
extern char* lskip(const char* s)
{
	while (*s && isspace(*s))
		s++;
	return (char*)s;
}

/* Return pointer to first char c or ';' comment in given string, or pointer to
   null at end of string if neither found. ';' must be prefixed by a whitespace
   character to register as a comment. */
extern char* find_char_or_comment(const char* s, char c)
{
	int was_whitespace = 0;
	while (*s && *s != c && !(was_whitespace && *s == ';')) {
		was_whitespace = isspace(*s);
		s++;
	}
	return (char*)s;
}