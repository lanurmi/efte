#include "fte.h"

int UnTabStr(char *dest, int maxlen, const char *source, int slen) {
    char *p = dest;
    int i;
    int pos = 0;

    maxlen--;
    for (i = 0; i < slen; i++) {
        if (maxlen > 0) {
            if (source[i] == '\t') {
                do {
                    if (maxlen > 0) {
                        *p++ = ' ';
                        maxlen--;
                    }
                    pos++;
                } while (pos & 0x7);
            } else {
                *p++ = source[i];
                pos++;
                maxlen--;
            }
        } else
            break;
    }

    //dest[pos] = 0;
    *p = '\0';
    return pos;
}

#if !defined(HAVE_STRLCPY)
size_t strlcpy(char *dst, const char *src, size_t size)
{
    size_t ret = strlen(src);

    if (size) {
        size_t len = (ret >= size) ? size-1 : ret;
        memcpy(dst, src, len);
        dst[len] = '\0';
    }

    return ret;
}
#endif // !HAVE_STRLCPY

#if !defined(HAVE_STRLCAT)
size_t strlcat(char *dst, const char *src, size_t size)
{
    size_t dst_len = strlen(dst);
    size_t src_len = strlen(src);

    if (size) {
        size_t len = (src_len >= size-dst_len) ? (size-dst_len-1) : src_len;
        memcpy(&dst[dst_len], src, len);
        dst[dst_len + len] = '\0';
    }

    return dst_len + src_len;
}
#endif // !HAVE_STRLCAT

size_t tstrlcpy(TChar *dst, const TChar *src, size_t size)
{
    size_t ret = tstrlen(src);

    if (size) {
        size_t len = (ret >= size) ? size-1 : ret;
        memcpy(dst, src, len * sizeof(TChar));
        dst[len] = '\0';
    }

    return ret;
}

size_t tstrlcat(TChar *dst, const TChar *src, size_t size)
{
    size_t dst_len = tstrlen(dst);
    size_t src_len = tstrlen(src);

    if (size) {
        size_t len = (src_len >= size-dst_len) ? (size-dst_len-1) : src_len;
        memcpy(&dst[dst_len], src, len * sizeof(TChar));
        dst[dst_len + len] = '\0';
    }

    return dst_len + src_len;
}

size_t tstrlen(const TChar *src)
{
    size_t len = 0;

    while (*src++)
        len++;

    return len;
}

TChar *tstrdup(const TChar *src)
{
    size_t len;
    TChar *newstr;

    if (src == NULL) return NULL;

    len = tstrlen(src);
    newstr = (TChar *)malloc((len + 1) * sizeof(TChar));

    if (newstr != NULL)
    {
        for (size_t i = 0; i <= len; i++)
        {
            newstr[i] = src[i];
        }
    }

    return newstr;
}

#if defined(USE_UNICODE_INTERNALS)
size_t tstrlcpy(TChar *dst, const char *src, size_t size)
{
    size_t ret = strlen(src);

    if (size) {
        size_t len = (ret >= size) ? size-1 : ret;
        for (size_t i = 0; i < len; i++)
        {
            dst[i] = src[i];
        }
        dst[len] = '\0';
    }

    return ret;
}

size_t tstrlcat(TChar *dst, const char *src, size_t size)
{
    size_t dst_len = tstrlen(dst);
    size_t src_len = strlen(src);

    if (size) {
        size_t len = (src_len >= size-dst_len) ? (size-dst_len-1) : src_len;
        for (size_t i = 0; i < len; i++)
        {
            dst[i + dst_len] = src[i];
        }
        dst[dst_len + len] = '\0';
    }

    return dst_len + src_len;
}

size_t tstrlen(const char *src)
{
    size_t len = 0;

    while (*src++)
        len++;

    return len;
}

TChar *tstrdup(const char *src)
{
    size_t len;
    TChar *newstr;

    if (src == NULL) return NULL;

    len = strlen(src);
    newstr = (TChar *)malloc((len + 1) * sizeof(TChar));

    if (newstr != NULL)
    {
        for (size_t i = 0; i <= len; i++)
        {
            newstr[i] = src[i];
        }
    }

    return newstr;
}

#endif

