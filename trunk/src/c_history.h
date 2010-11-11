/*    c_history.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef FPOSHIST_H_
#define FPOSHIST_H_

#ifdef UNIX
#define HISTORY_NAME ".efte-history"
#else
#define HISTORY_NAME "efte.his"
#endif

typedef struct {
    char *Name;
    int Row, Col;
} HBookmark;

typedef struct {
    char *FileName;
    int Row, Col;
    HBookmark **Books;
    int BookCount;
} FPosHistory;

#define MAX_INPUT_HIST 128

typedef struct {
    int Count;
    char **Line;
    int *Id;
} InputHistory;

extern char HistoryFileName[256];

void ClearHistory();
int SaveHistory(char *FileName);
int LoadHistory(char *FileName);

int UpdateFPos(char *FileName, int Row, int Col);
int RetrieveFPos(char *FileName, int &Row, int &Col);

int AddInputHistory(int Id, char *String);
int CountInputHistory(int Id);
int GetInputHistory(int Id, char *String, int maxlen, int Nth);

// some platforms don't know about EBuffer yet
class EBuffer;
/*
 * Get bookmarks for given Buffer (file) from history.
 */
int RetrieveBookmarks(EBuffer *buf);
/*
 * Store given Buffer's bookmarks to history.
 */
int StoreBookmarks(EBuffer *buf);

/* history values */
#define HIST_DEFAULT    0
#define HIST_PATH       1
#define HIST_SEARCH     2
#define HIST_POSITION   3
#define HIST_SETUP      4
#define HIST_SHELL      5
#define HIST_COMPILE    6
#define HIST_SEARCHOPT  7
#define HIST_BOOKMARK   8
#define HIST_REGEXP     9
#define HIST_TRANS     10
#define HIST_TAGFILES  11
#define HIST_CVS       12
#define HIST_CVSDIFF   13
#define HIST_CVSCOMMIT 14
#define HIST_SVN       15
#define HIST_SVNDIFF   16
#define HIST_SVNCOMMIT 17

#endif
