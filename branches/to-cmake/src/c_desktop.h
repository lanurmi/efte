/*    c_desktop.h
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef __DESKTOP_H__
#define __DESKTOP_H__

#ifdef UNIX
#    define        DESKTOP_NAME       ".efte-desktop"
#else
#    define        DESKTOP_NAME       "efte.dsk"
#endif

extern char DesktopFileName[256];

int SaveDesktop(char *FileName);
int LoadDesktop(char *FileName);

#endif
