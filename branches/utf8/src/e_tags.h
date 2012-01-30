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

#ifndef TAGS_H_
#define TAGS_H_

#include <stdio.h> // FILE

class EView;
class EBuffer;

int TagsAdd(char *FileName);
int TagsSave(FILE *fp);
int TagsLoad(int id);

int TagLoad(char *FileName);
void TagClear();
int TagGoto(EView *V, char *Tag);
int TagDefined(const char *Tag);
int TagFind(EBuffer *B, EView *V, char *Tag);
int TagComplete(char **Words, int *WordsPos, int WordsMax, char *Tag);
int TagNext(EView *V);
int TagPrev(EView *V);
int TagPop(EView *V);

#endif // __TAGS_H
