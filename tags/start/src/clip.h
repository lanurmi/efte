/*    clip.h
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef __CLIPLIB_H
#define __CLIPLIB_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned long fLen;
    char *fChar;
} ClipData;

int GetClipText(ClipData *cd);
int PutClipText(ClipData *cd);

#ifdef __cplusplus
}
#endif
#endif
