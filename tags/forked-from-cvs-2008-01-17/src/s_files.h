/*    s_files.h
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef __FILESYS_H__
#define __FILESYS_H__

#define SDOT   "."    

#ifdef UNIX
#define SLASH  '/'
#define SSLASH "/"

#define ISSLASH(c) ((c) == SLASH)
#define ISSEP(c) ((c) == SLASH)
#endif

#if defined(OS2) || defined(NT) || defined(DOS) || defined(DOSP32)
#if defined(DJGPP)
#define SLASH '/'
#define SSLASH "/"
#else
#define SLASH '\\'
#define SSLASH "\\"
#endif

#define ISSLASH(c) (((c) == '/') || ((c) == '\\'))
#define ISSEP(c) (((c) == ':') || ISSLASH(c))
#endif

char *Slash(char *Path, int Add);
char *SlashDir(char *Path);
int ExpandPath(const char *Path, char *Expanded, int ExpandSize);
int CompletePath(const char *Path, char *Completed, int Num);
int IsSameFile(const char *Path1, const char *Path2);
int JustDirectory(const char *Path, char *Dir, int DirSize);
int JustFileName(const char *Path, char *Name, int NameSize);
int JustLastDirectory(const char *Path, char *Dir, int DirSize);
int JustRoot(const char *Path, char *Root, int RootSize);
int FileExists(const char *Path);
int IsFullPath(const char *Path);
int IsDirectory(const char *Path);
const char *ShortFName(const char *Path, int len);
int ChangeDir(char *Dir);
int JoinDirFile(char *Dest, const char *Dir, const char *Name);
char *SepRChr(char *Dir);
int RelativePathName(const char *Dir, const char *Path, char *RelPath);

#endif
