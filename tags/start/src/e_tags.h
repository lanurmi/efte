/*    e_tags.h
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef __TAGS_H
#define __TAGS_H

#ifdef CONFIG_TAGS

int TagsAdd(char *FileName);
int TagsSave(FILE *fp);
int TagsLoad(int id);

int TagLoad(char *FileName);
void TagClear();
int TagGoto(EView *V, char *Tag);
int TagDefined(char *Tag);
int TagFind(EBuffer *B, EView *V, char *Tag);
int TagComplete(char **Words, int *WordsPos, int WordsMax, char *Tag);
int TagNext(EView *V);
int TagPrev(EView *V);
int TagPop(EView *V);

#endif

#endif
