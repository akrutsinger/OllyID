#include <stdio.h>

extern void unicode_remove_char(wchar_t *s, wchar_t c);
extern void remove_char(char *s, char c);
extern int get_line(FILE *fp, char *buffer, size_t buflen);