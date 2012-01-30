/*
 * ftever.h
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 */

#ifndef FTEVER_H_
#define FTEVER_H_

#define MAKE_VERSION(major,minor,release) ((major<<24L) | (minor << 16L) | release)

#define PROG_FTE      "efte"
#define PROG_CFTE     "cefte"
#define PROGRAM       PROG_FTE
#define EXTRA_VERSION ""
#define VERSION       "1.1" EXTRA_VERSION
#define VERNUM        MAKE_VERSION(0x00, 0x62, 0x00)
#define COPYRIGHT     "Copyright (c) 2008-2009 eFTE Group\n" \
   "Copyright (c) 2000-2006 Others\n" \
   "Copyright (c) 1994-1998 Marko Macek"

#endif // __FTEVER_H
