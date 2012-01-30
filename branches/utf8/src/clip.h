/*    clip.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef CLIPLIB_H_
#define CLIPLIB_H_

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
