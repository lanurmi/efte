#ifndef __FTEVER_H
#define __FTEVER_H

#define MAKE_VERSION(major,minor,release) ((major<<24L) | (minor << 16L) | release)

#define PROG_FTE      "efte"
#define PROG_CFTE     "cefte"
#define PROGRAM       PROG_FTE
#define EXTRA_VERSION ""
#define VERSION       "1.0" EXTRA_VERSION
#define VERNUM        MAKE_VERSION(0x01, 0x00, 0x00)
#define COPYRIGHT     "Copyright (c) 2008 eFTE Group" \
   "Copyright (c) 2000-2006 Others" \
   "Copyright (c) 1994-1998 Marko Macek"

#endif // __FTEVER_H
