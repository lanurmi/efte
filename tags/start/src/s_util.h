/*    s_util.h
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef __EDITOR_H__
#define __EDITOR_H__

#define USE_CtrlEnter    1

#define S_BUSY     0
#define S_INFO     1
#define S_BOLD     2
#define S_ERROR    3

char* MakeBackup(char *FileName, char *NewName);

int GetPMClip();
int PutPMClip();

int FileLoad(int createFlags, const char *FileName, const  char *Mode, EView *View);
int MultiFileLoad(int createFlags, const char *FileName, const char *Mode, EView *View);
int ParseSearchOption(int replace, char c, unsigned long &opt);
int ParseSearchOptions(int replace, const char *str, unsigned long &Options);
int ParseSearchReplace(EBuffer *B, const char *str, int replace, SearchReplaceOptions &opt);
int SetDefaultDirectory(EModel *M);
int GetDefaultDirectory(EModel *M, char *Path, int MaxLen);
int UnTabStr(char *dest, int maxlen, const char *source, int slen);

#endif
