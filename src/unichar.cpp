/*    unichar.cpp
 *
 *    Copyright (c) 2012, eFTE SF Group (see AUTHORS file)
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "unichar.h"

#ifdef UNICODE_ENABLED
static const unsigned char uni_utf8_non1_bytes[256 - 192 - 2] = {
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1
};

static inline unsigned int
uni_utf8_char_bytes(char chr)
{
	/* 0x00 .. 0x7f are ASCII. 0x80 .. 0xC1 are invalid. */
	if ((unsigned char)chr < (192 + 2))
		return 1;
	return uni_utf8_non1_bytes[(unsigned char)chr - (192 + 2)];
}

static int uni_utf8_get_char_n(const void *_input, size_t max_len, unichar_t *chr_r)
{
	const unsigned char *input = (const unsigned char *)_input;
	unichar_t chr;
	unsigned int i, len;
	int ret;

	if (*input < 0x80) {
		*chr_r = *input;
		return 1;
	}

	/* first byte has len highest bits set, followed by zero bit.
	   the rest of the bits are used as the highest bits of the value. */
	chr = *input;
	len = uni_utf8_char_bytes(*input);
	switch (len) {
	case 2:
		chr &= 0x1f;
		break;
	case 3:
		chr &= 0x0f;
		break;
	case 4:
		chr &= 0x07;
		break;
	case 5:
		chr &= 0x03;
		break;
	case 6:
		chr &= 0x01;
		break;
	default:
		/* only 7bit chars should have len==1 */
		return -1;
	}

	if (len <= max_len)
		ret = 1;
	else {
		/* check first if the input is invalid before returning 0 */
		ret = 0;
		len = max_len;
	}

	/* the following bytes must all be 10xxxxxx */
	for (i = 1; i < len; i++) {
		if ((input[i] & 0xc0) != 0x80)
			return input[i] == '\0' ? 0 : -1;

		chr <<= 6;
		chr |= input[i] & 0x3f;
	}

	*chr_r = chr;
	return ret;
}

void uni_ucs4_to_utf8_c(unichar_t chr, char **output)
{
	unsigned char first;
	int bitpos;

	if (chr < 0x80) {
		**output = chr;
		*output += 1;
		return;
	}

	if (chr < (1 << (6 + 5))) {
		/* 110xxxxx */
		bitpos = 6;
		first = 0x80 | 0x40;
	} else if (chr < (1 << ((2*6) + 4))) {
		/* 1110xxxx */
		bitpos = 2*6;
		first = 0x80 | 0x40 | 0x20;
	} else if (chr < (1 << ((3*6) + 3))) {
		/* 11110xxx */
		bitpos = 3*6;
		first = 0x80 | 0x40 | 0x20 | 0x10;
	} else if (chr < (1 << ((4*6) + 2))) {
		/* 111110xx */
		bitpos = 4*6;
		first = 0x80 | 0x40 | 0x20 | 0x10 | 0x08;
	} else {
		/* 1111110x */
		bitpos = 5*6;
		first = 0x80 | 0x40 | 0x20 | 0x10 | 0x08 | 0x04;
	}
	**output = first | (chr >> bitpos);
	*output += 1;

	do {
		bitpos -= 6;
		**output = 0x80 | ((chr >> bitpos) & 0x3f);
		*output += 1;
	} while (bitpos > 0);
}

char *uni_to_utf8_n(const unichar_t *data, unsigned int size)
{
    char *dest = (char *)malloc(size*5 + 1); // FIXME: wasteful
    char *p = dest;
    for (; size > 0 && *data != '\0'; data++, size--)
        uni_ucs4_to_utf8_c(*data, &p);
    return dest;
}

unichar_t *uni_utf8_to_unichar(const char *data, unsigned int size)
{
    unichar_t *uniBuffer, chr;
    unsigned int i, dest;

    uniBuffer = (unichar_t *)malloc(sizeof(unichar_t) * (size + 1));
    if (uniBuffer == NULL)
        exit(1);

    for (i = dest = 0; i < size; ) {
        if (uni_utf8_get_char_n(data+i, size-i, &chr) <= 0) {
            /* invalid input */
            break;
        }
        i += uni_utf8_char_bytes(data[i]);
        uniBuffer[dest++] = chr;
    }
    uniBuffer[dest] = '\0';
    return uniBuffer;
}

unichar_t *uni_ascii_to_unichar(const char *data, unsigned int size)
{
    unichar_t *uniBuffer;
    unsigned int i;

    uniBuffer = (unichar_t *)malloc(sizeof(unichar_t) * (size + 1));
    if (uniBuffer == NULL)
        exit(1);
    for (i = 0; i < size; i++)
        uniBuffer[i] = data[i];
    uniBuffer[i] = 0;
    return uniBuffer;
}

int uni_strncmp_ascii(const unichar_t *unistr, const char *str,
                      unsigned int len)
{
    unsigned int i;

    for (i = 0; i < len; i++) {
        if (unistr[i] != str[i]) {
            if (unistr[i] < str[i])
                return -1;
            else
                return 1;
        }
        if (str[i] == '\0')
            break;
    }
    return 0;
}

int uni_strnicmp_ascii(const unichar_t *unistr, const char *str,
                       unsigned int len)
{
    unsigned int i;

    for (i = 0; i < len; i++) {
        if (unistr[i] > 0xff)
            return 1;
        if (toupper((unsigned char)unistr[i]) != toupper((unsigned char)str[i])) {
            if (unistr[i] < str[i])
                return -1;
            else
                return 1;
        }
        if (str[i] == '\0')
            break;
    }
    return 0;
}

int uni_stricmp(const unichar_t *str1, const unichar_t *str2)
{
    unsigned int i;

    for (i = 0;; i++) {
        if (str1[i] == str2[i])
            ;
        else if (str1[i] > 0xff || str2[i] > 0xff ||
                 toupper((unsigned char)str1[i]) != toupper((unsigned char)str2[i])) {
            if (str1[i] < str2[i])
                return -1;
            else
                return 1;
        }
        if (str1[i] == '\0')
            break;
    }
    return 0;
}

int uni_strnicmp(const unichar_t *str1, const unichar_t *str2, unsigned int len)
{
    unsigned int i;

    for (i = 0; i < len; i++) {
        if (str1[i] == str2[i])
            ;
        else if (str1[i] > 0xff || str2[i] > 0xff ||
                 toupper((unsigned char)str1[i]) != toupper((unsigned char)str2[i])) {
            if (str1[i] < str2[i])
                return -1;
            else
                return 1;
        }
        if (str1[i] == '\0')
            break;
    }
    return 0;
}

unichar_t *uni_strdup(const unichar_t *str)
{
    unsigned int i, len = uni_strlen(str);
    unichar_t *uniStr;

    uniStr = (unichar_t *)malloc(sizeof(unichar_t) * (len+1));
    if (uniStr == NULL)
        exit(1);
    for (i = 0; i <= len; i++)
        uniStr[i] = str[i];
    return uniStr;
}

unichar_t *uni_strdup_ascii(const char *str)
{
    unsigned int i, len = strlen(str);
    unichar_t *uniStr;

    uniStr = (unichar_t *)malloc(sizeof(unichar_t) * (len+1));
    if (uniStr == NULL)
        exit(1);
    for (i = 0; i <= len; i++)
        uniStr[i] = str[i];
    return uniStr;
}

#else

char *uni_to_utf8_n(const unichar_t *data, unsigned int size)
{
    return strndup(data, size);
}

unichar_t *uni_utf8_to_unichar(const char *data, unsigned int size)
{
    return strndup(data, size);
}

unichar_t *uni_ascii_to_unichar(const char *data, unsigned int size)
{
    return strndup(data, size);
}

int uni_strncmp_ascii(const unichar_t *unistr, const char *str,
                      unsigned int len)
{
    return strncmp(unistr, str, len);
}

int uni_strnicmp_ascii(const unichar_t *unistr, const char *str,
                       unsigned int len)
{
    return strncasecmp(unistr, str, len);
}

unichar_t *uni_strdup_ascii(const char *str)
{
    return strdup(str);
}

#endif
