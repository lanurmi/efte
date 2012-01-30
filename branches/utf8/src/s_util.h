/*    s_util.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef S_UTIL_H_
#define S_UTIL_H_

#define USE_CtrlEnter    1

#define S_BUSY     0
#define S_INFO     1
#define S_BOLD     2
#define S_ERROR    3

class EView;
class EBuffer;
class EModel;

char* MakeBackup(char *FileName, char *NewName);

int GetPMClip(int clipboard);
int PutPMClip(int clipboard);

int FileLoad(int createFlags, const char *FileName, const  char *Mode, EView *View);
int MultiFileLoad(int createFlags, const char *FileName, const char *Mode, EView *View);
int SetDefaultDirectory(EModel *M);
int GetDefaultDirectory(EModel *M, char *Path, int MaxLen);

#endif
