#ifndef __S_STRING_H__
#define __S_STRING_H__

int UnTabStr(char *dest, int maxlen, const char *source, int slen);

#if !defined(HAVE_STRLCPY)
size_t strlcpy(char *dst, const char *src, size_t size);
#endif // !HAVE_STRLCPY

#if !defined(HAVE_STRLCAT)
size_t strlcat(char *dst, const char *src, size_t size);
#endif // !HAVE_STRLCAT

// TChar helpers
size_t tstrlcpy(TChar *dst, const TChar *src, size_t size);
size_t tstrlcat(TChar *dst, const TChar *src, size_t size);
size_t tstrlen(const TChar *src);
TChar *tstrdup(const TChar *src);
size_t tstrpos(const TChar *src, TChar ch);

#if defined(USE_UNICODE_INTERNALS)
size_t tstrlcpy(TChar *dst, const char *src, size_t size);
size_t tstrlcat(TChar *dst, const char *src, size_t size);
size_t tstrlen(const char *src);
TChar *tstrdup(const char *src);
size_t tstrpos(const char *src, TChar ch);
#endif

#endif
