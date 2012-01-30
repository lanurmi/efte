/*    unichar.h
 *
 *    Copyright (c) 2012, eFTE SF Group (see AUTHORS file)
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef UNICHAR_H_
#define UNICHAR_H_

#define UNICODE_ENABLED

#ifdef UNICODE_ENABLED
#include <wchar.h>
typedef wchar_t unichar_t;
#define uni_strcmp wcscmp
#define uni_strncmp wcsncmp
#define uni_strrchr wcsrchr
#define uni_strlen wcslen
int uni_stricmp(const unichar_t *str1, const unichar_t *str2);
int uni_strnicmp(const unichar_t *str1, const unichar_t *str2, unsigned int len);
unichar_t *uni_strdup(const unichar_t *str);
#else
typedef char unichar_t;
#define uni_strcmp strcmp
#define uni_strncmp strncmp
#define uni_strrchr strrchr
#define uni_stricmp stricmp
#define uni_strnicmp strnicmp
#define uni_strdup strdup
#define uni_strlen strlen
#endif

/* Convert unichar buffer to UTF-8. */
char *uni_to_utf8_n(const unichar_t *data, unsigned int size);
/* Convert UTF-8 data to unichar */
unichar_t *uni_utf8_to_unichar(const char *data, unsigned int size);
/* Convert ASCII data to unichar */
unichar_t *uni_ascii_to_unichar(const char *data, unsigned int size);

/* strncmp() for mixed unichar/char strings. */
int uni_strncmp_ascii(const unichar_t *unistr, const char *str,
                      unsigned int len);
int uni_strnicmp_ascii(const unichar_t *unistr, const char *str,
                       unsigned int len);

unichar_t *uni_strdup_ascii(const char *str);

#endif
