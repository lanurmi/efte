/*    e_undo.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef UNDO_H_
#define UNDO_H_

/*
 * only core operations can be directly undone
 * - Insert # of Lines
 * - Delete # of Lines
 * - Insert # Line
 * - Delete Line Text
 * - Insert Line Text
 * - Positioning
 * - Block marking
 */

#define ucInsLine          1
#define ucDelLine          2
#define ucInsChars         3
#define ucDelChars         4

#define ucJoinLine         5
#define ucSplitLine        6

#define ucPosition         7
#define ucBlock            8
#define ucModified         9

#define ucFoldCreate       11
#define ucFoldDestroy      12
#define ucFoldPromote      13
#define ucFoldDemote       14
#define ucFoldOpen         15
#define ucFoldClose        16

#define ucPlaceUserBookmark  17
#define ucRemoveUserBookmark 18

#endif
