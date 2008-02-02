/*    s_direct.cpp
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

// TODO: on OS/2 choose between UNIX and OS/2 style patterns (now OS/2 only)
// TODO: on OS/2 fetch multiple entries at once and cache them for speed
#include "fte.h"

#ifdef OS2
#define INCL_BASE
#include <os2.h>
#endif

#ifdef NT
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif

FileInfo::FileInfo(char *Name, int Type, off_t Size, time_t MTime) {
    name = new char[strlen(Name) + 1];
    if (name)
        strcpy(name, Name);
    size = Size;
    type = Type;
    mtime = MTime;
}

FileInfo::~FileInfo() {
    delete [] name;
}

FileFind::FileFind(const char *aDirectory, const char *aPattern, int aFlags) {
    Directory = new char[strlen(aDirectory) + 1];
    if (Directory) {
        strcpy(Directory, aDirectory);
        Slash(Directory, 0);
    }
    if (aPattern) {
        Pattern = new char[strlen(aPattern) + 1];
        if (Pattern)
            strcpy(Pattern, aPattern);
    } else {
        Pattern = 0;
    }
    Flags = aFlags;
#if defined(USE_DIRENT)
    dir = 0;
#elif defined(OS2) && !defined(USE_DIRENT)
    dir = 0;
#elif defined(NT) && !defined(USE_DIRENT)
    dir = 0;
#endif
}

FileFind::~FileFind() {
    delete [] Directory;
    if (Pattern)
        delete [] Pattern;
#if defined(USE_DIRENT)
    if (dir)
        closedir(dir);
#elif defined(OS2) && !defined(USE_DIRENT)
    if (dir)
        DosFindClose(dir);
#elif defined(NT) && !defined(USE_DIRENT)
    if (dir)
        FindClose((HANDLE)dir);
#endif
}

int FileFind::FindFirst(FileInfo **fi) {
#if defined(USE_DIRENT)
    if (dir)
        closedir(dir);
    if ((dir = opendir(Directory)) == 0)
        return -1;
    return FindNext(fi);
#elif defined(OS2) && !defined(USE_DIRENT)
    char fullpattern[MAXPATH];
    HDIR hdir = HDIR_CREATE;
    ULONG attr = FILE_ARCHIVED | FILE_READONLY;
    ULONG count = 1;
    FILEFINDBUF3 find; // need to improve to fetch multiple entries at once
    char fullpath[MAXPATH];
    char *name;
    struct tm t;
    int rc;

    if (dir)
        DosFindClose(dir);

    if (Flags & ffDIRECTORY)
        attr |= FILE_DIRECTORY;

    if (Flags & ffHIDDEN)
        attr |= FILE_HIDDEN | FILE_SYSTEM; // separate ?

    if (Pattern)
        JoinDirFile(fullpattern, Directory, Pattern);
    else
        JoinDirFile(fullpattern, Directory, "*");

    if ((rc = DosFindFirst(fullpattern,
                           &hdir,
                           attr,
                           &find, sizeof(find),
                           &count,
                           FIL_STANDARD)) != 0) {
        //fprintf(stderr, "%s: %d\n\n", fullpattern, rc);
        return -1;
    }
    dir = hdir;
    if (count != 1)
        return -1;
    name = find.achName;
    if (Flags & ffFULLPATH) {
        JoinDirFile(fullpath, Directory, name);
        name = fullpath;
    }
    memset((void *)&t, 0, sizeof(t));
    t.tm_year = find.fdateLastWrite.year + 80; // ugh!
    t.tm_mon = find.fdateLastWrite.month - 1;
    t.tm_mday = find.fdateLastWrite.day;
    t.tm_hour = find.ftimeLastWrite.hours;
    t.tm_min = find.ftimeLastWrite.minutes;
    t.tm_sec = find.ftimeLastWrite.twosecs * 2; // ugh!
    t.tm_isdst = -1;
    *fi = new FileInfo(name,
                       (find.attrFile & FILE_DIRECTORY) ? fiDIRECTORY : fiFILE,
                       find.cbFile,
                       mktime(&t));
    return 0;
#elif defined(NT) && !defined(USE_DIRENT)
#if defined(USE_VCFIND)
    char fullpattern[MAXPATH];

    _finddata_t find; // need to improve to fetch multiple entries at once
    char fullpath[MAXPATH];
    char *name;
    struct tm t;
    int rc;

    if (dir)
        _findclose(dir);

    /*if (Flags & ffDIRECTORY)
    attr |= FILE_DIRECTORY;

    if (Flags & ffHIDDEN)
    attr |= FILE_HIDDEN | FILE_SYSTEM; // separate ?
    */
    if (Pattern)
        JoinDirFile(fullpattern, Directory, Pattern);
    else
        JoinDirFile(fullpattern, Directory, "*");

    if ((rc = _findfirst(fullpattern, &find)) == -1) {
        //        fprintf(stderr, "%s: %d\n\n", fullpattern, rc);
        return -1;
    }
    dir = rc;

    name = find.name;
    if (Flags & ffFULLPATH) {
        JoinDirFile(fullpath, Directory, name);
        name = fullpath;
    }

    *fi = new FileInfo(name,
                       (find.attrib & _A_SUBDIR) ? fiDIRECTORY : fiFILE,
                       find.size,
                       find.time_create);
    return 0;
#else
    char fullpattern[MAXPATH];

    WIN32_FIND_DATA find; // need to improve to fetch multiple entries at once
    char fullpath[MAXPATH];
    char *name;
    struct tm t;
    SYSTEMTIME st;
    FILETIME localft;                   // needed for time conversion
    int rc;

    if (dir)
        _findclose(dir);

    /*if (Flags & ffDIRECTORY)
    attr |= FILE_DIRECTORY;

    if (Flags & ffHIDDEN)
    attr |= FILE_HIDDEN | FILE_SYSTEM; // separate ?
    */
    if (Pattern)
        JoinDirFile(fullpattern, Directory, Pattern);
    else
        JoinDirFile(fullpattern, Directory, "*");

    if ((rc = (int) FindFirstFile(fullpattern, &find)) < 0) {
        //fprintf(stderr, "%s: %d\n\n", fullpattern, rc);
        return -1;
    }
    dir = rc;

    name = find.cFileName;
    if (Flags & ffFULLPATH) {
        JoinDirFile(fullpath, Directory, name);
        name = fullpath;
    }

    /*
    * since filetime is in UTC format we need to convert it first to
    * localtime and when we have "correct" time we can use it.
    */

    FileTimeToLocalFileTime(&find.ftLastWriteTime, &localft);
    FileTimeToSystemTime(&localft, &st);

    t.tm_year = st.wYear - 1900;
    t.tm_mon = st.wMonth - 1;           // in system time january is 1...
    t.tm_mday = st.wDay;
    t.tm_hour = st.wHour;
    t.tm_min = st.wMinute;
    t.tm_sec = st.wSecond;
    t.tm_isdst = -1;


    *fi = new FileInfo(name,
                       (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? fiDIRECTORY : fiFILE,
                       find.nFileSizeLow, // !!!
                       mktime(&t));
    return 0;
#endif
#endif
}

int FileFind::FindNext(FileInfo **fi) {
#if defined(USE_DIRENT)
    struct dirent *dent;
    char fullpath[MAXPATH];
    char *name;

    *fi = 0;
again:
    if ((dent = readdir(dir)) == NULL)
        return -1;
    name = dent->d_name;

    if (name[0] == '.')
        if (!(Flags & ffHIDDEN))
            goto again;

    if (Pattern && fnmatch(Pattern, dent->d_name, 0) != 0)
        goto again;

    if (Flags & ffFULLPATH) {
        JoinDirFile(fullpath, Directory, dent->d_name);
        name = fullpath;
    }

    if (Flags & ffFAST) {
        *fi = new FileInfo(name, fiUNKNOWN, 0, 0);
    } else {
        struct stat st;

        if (!(Flags & ffFULLPATH)) // need it now
            JoinDirFile(fullpath, Directory, dent->d_name);

        if (Flags && ffLINK) {
            // if we are handling location of symbolic links, lstat cannot be used
            // instead use normal stat
            if (stat(fullpath, &st) != 0 && 0)
                goto again;
        } else {
            if (
#if defined(UNIX) // must use lstat if available
                lstat
#else
                stat
#endif
                (fullpath, &st) != 0 && 0)
                goto again;
        }

        if (!(Flags & ffDIRECTORY) && S_ISDIR(st.st_mode))
            goto again;

        *fi = new FileInfo(name,
                           S_ISDIR(st.st_mode) ? fiDIRECTORY : fiFILE,
                           st.st_size,
                           st.st_mtime);
    }
    //printf("ok\n");
    return 0;
#elif defined(OS2) && !defined(USE_DIRENT)
    ULONG count = 1;
    FILEFINDBUF3 find; // need to improve to fetch multiple entries at once
    char fullpath[MAXPATH];
    char *name;
    struct tm t;
    int rc;

    if ((rc = DosFindNext(dir,
                          &find, sizeof(find),
                          &count)) != 0) {
        //fprintf(stderr, "%d\n\n", rc);
        return -1;
    }
    if (count != 1)
        return -1;
    name = find.achName;
    if (Flags & ffFULLPATH) {
        JoinDirFile(fullpath, Directory, name);
        name = fullpath;
    }
    memset((void *)&t, 0, sizeof(t));
    t.tm_year = find.fdateLastWrite.year + 80;
    t.tm_mon = find.fdateLastWrite.month - 1;
    t.tm_mday = find.fdateLastWrite.day;
    t.tm_hour = find.ftimeLastWrite.hours;
    t.tm_min = find.ftimeLastWrite.minutes;
    t.tm_sec = find.ftimeLastWrite.twosecs * 2; // ugh!
    t.tm_isdst = -1;
    *fi = new FileInfo(name,
                       (find.attrFile & FILE_DIRECTORY) ? fiDIRECTORY : fiFILE,
                       find.cbFile,
                       mktime(&t));

    return 0;
#elif defined(NT) && !defined(USE_DIRENT)
#if defined(USE_VCFIND)
    _finddata_t find;
    char fullpath[MAXPATH];
    char *name;
    struct tm t;
    int rc;

    if ((rc = _findnext(dir,
                        &find)) != 0) {
        // fprintf(stderr, "%d\n\n", rc);
        return -1;
    }

    name = find.name;
    if (Flags & ffFULLPATH) {
        JoinDirFile(fullpath, Directory, name);
        name = fullpath;
    }

    *fi = new FileInfo(name,
                       (find.attrib & _A_SUBDIR) ? fiDIRECTORY : fiFILE,
                       find.size,
                       find.time_create);
    return 0;
#else
    WIN32_FIND_DATA find;
    char fullpath[MAXPATH];
    char *name;
    struct tm t;
    SYSTEMTIME st;
    FILETIME localft;                   // needed for time conversion
    int rc;

    if ((rc = FindNextFile((HANDLE)dir,
                           &find)) != TRUE) {
        //fprintf(stderr, "%d\n\n", rc);
        return -1;
    }

    name = find.cFileName;
    if (Flags & ffFULLPATH) {
        JoinDirFile(fullpath, Directory, name);
        name = fullpath;
    }

    /*
    * since filetime is in UTC format we need to convert it first to
    * localtime and when we have "correct" time we can use it.
    */

    FileTimeToLocalFileTime(&find.ftLastWriteTime, &localft);
    FileTimeToSystemTime(&localft, &st);

    t.tm_year = st.wYear - 1900;
    t.tm_mon = st.wMonth - 1;           // in system time january is 1...
    t.tm_mday = st.wDay;
    t.tm_hour = st.wHour;
    t.tm_min = st.wMinute;
    t.tm_sec = st.wSecond;
    t.tm_isdst = -1;

    *fi = new FileInfo(name,
                       (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? fiDIRECTORY : fiFILE,
                       find.nFileSizeLow,
                       mktime(&t));
    return 0;
#endif
    //    put code here
#endif
}
