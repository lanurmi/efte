/*    c_history.h
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef __FPOSHIST_H__
#define __FPOSHIST_H__

#ifdef CONFIG_HISTORY

#ifdef UNIX
#define HISTORY_NAME ".fte-history"
#else
#define HISTORY_NAME "fte.his"
#endif

typedef struct {
    char *FileName;
    int Row, Col;
} FPosHistory;

#define MAX_INPUT_HIST 128

typedef struct {
    int Count;
    char **Line;
    int *Id;
} InputHistory;

extern char HistoryFileName[256];


int SaveHistory(char *FileName);
int LoadHistory(char *FileName);

int UpdateFPos(char *FileName, int Row, int Col);
int RetrieveFPos(char *FileName, int &Row, int &Col);

int AddInputHistory(int Id, char *String);
int CountInputHistory(int Id);
int GetInputHistory(int Id, char *String, int maxlen, int Nth);

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

#endif

#endif
