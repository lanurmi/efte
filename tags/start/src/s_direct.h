/*    s_direct.h
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef __SDIRECT_H
#define __SDIRECT_H

// error handling needs some work

#define fiUNKNOWN   0
#define fiFILE      1
#define fiDIRECTORY 2

class FileInfo {
private:
    char *name;   // minimum set of file information
    off_t size;
    time_t mtime;
    int type;
    
public:
    FileInfo(char *Name, int type, off_t Size, time_t MTime);
    ~FileInfo();

    char *Name() { return name; }
    off_t Size() { return size; }
    int Type() { return type; }
    time_t MTime() { return mtime; }
};

#define ffFAST       1  // optimization for UNIX (return name only, NO TYPE CHECK), ignored on OS/2 and NT
#define ffFULLPATH   2  // return full path to files
#define ffDIRECTORY  4  // return directories beside files (see ffFAST)
#define ffHIDDEN     8  // return hidden files (dot-files for UNIX)

class FileFind {
private:
    char *Directory;
    char *Pattern;
    int Flags;

#if defined(USE_DIRENT)
    DIR *dir;
#elif defined(OS2) && !defined(USE_DIRENT)
    unsigned long dir; // should be HDIR, but we don't #include huge os2.h globally
#elif defined(NT) && !defined(USE_DIRENT)
    unsigned long dir; // should be HANDLE
#endif
    
public:
    FileFind(const char *aDirectory, const char *aPattern, int aFlags);
    ~FileFind();

    int FindFirst(FileInfo **fi);
    int FindNext(FileInfo **fi);
};

#endif
