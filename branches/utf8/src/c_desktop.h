/*    c_desktop.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef DESKTOP_H_
#define DESKTOP_H_

#ifdef UNIX
#    define        DESKTOP_NAME       ".efte-desktop"
#else
#    define        DESKTOP_NAME       "efte.dsk"
#endif

extern char DesktopFileName[256];

int SaveDesktop(char *FileName);
int LoadDesktop(char *FileName);

#endif
