/*******************************************************************************
 * OllyID - string.h
 *
 * Copyright (c) 2012-2013, Austyn Krutsinger
 * All rights reserved.
 *
 * OllyID is released under the New BSD license (see LICENSE.txt).
 *
 ******************************************************************************/

#include <stdio.h>

#ifndef _OLLYID_HELPER_H_
#define _OLLYID_HELPER_H_

extern void unicode_remove_char(wchar_t *s, wchar_t c);
extern void remove_char(char *s, char c);
extern int get_line(FILE *fp, char *buffer, size_t buflen);
extern char* rstrip(char* s);
extern char* lskip(const char* s);
extern char* find_char_or_comment(const char* s, char c);
extern char *cstrndup(const char *src, int max_len);

#endif /* _OLLYID_HELPER_H_ */