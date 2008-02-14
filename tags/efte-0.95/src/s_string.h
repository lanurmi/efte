/*
 * string.h
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 */

#ifndef __S_STRING_H__
#define __S_STRING_H__

int UnTabStr(char *dest, int maxlen, const char *source, int slen);

#if !defined(HAVE_STRLCPY)
size_t strlcpy(char *dst, const char *src, size_t size);
#endif // !HAVE_STRLCPY

#if !defined(HAVE_STRLCAT)
size_t strlcat(char *dst, const char *src, size_t size);
#endif // !HAVE_STRLCAT

#endif
