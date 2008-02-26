/*
 *    e_tags.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef __TAGS_H
#define __TAGS_H

#include <stdio.h> // FILE

class EView;
class EBuffer;

int TagsAdd(const char *FileName);
int TagsSave(FILE *fp);
int TagsLoad(int id);

int TagLoad(const char *FileName);
void TagClear();
int TagGoto(EView *V, const char *Tag);
int TagDefined(const char *Tag);
int TagFind(EBuffer *B, EView *V, const char *Tag);
int TagComplete(char **Words, int *WordsPos, int WordsMax, const char *Tag);
int TagNext(EView *V);
int TagPrev(EView *V);
int TagPop(EView *V);

#endif // __TAGS_H
