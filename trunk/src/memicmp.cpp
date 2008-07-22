/*
 * memicmp.cpp
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 */

// Contributed by Markus F.X.J. Oberhumer <markus.oberhumer@jk.uni-linz.ac.at>

#include <stddef.h>
#include <ctype.h>

#if defined(UNIX)

#ifdef __cplusplus
extern "C"
#endif
    int memicmp(const void *s1, const void *s2, size_t n) {
    if (n != 0) {
        const unsigned char *p1 = (const unsigned char *) s1;
        const unsigned char *p2 = (const unsigned char *) s2;

        do {
            if (*p1 != *p2) {
                int c = toupper(*p1) - toupper(*p2);
                if (c)
                    return c;
            }
            p1++;
            p2++;
        } while (--n != 0);
    }
    return 0;
}

#endif

