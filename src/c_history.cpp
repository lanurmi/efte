/*    c_history.cpp
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

#ifdef CONFIG_HISTORY

#define HISTORY_VER "FTE History 1\n"

char HistoryFileName[256] = "";

static FPosHistory **FPHistory = 0;
static int FPHistoryCount = 0;

static InputHistory inputHistory = { 0, 0, 0 };

void ClearHistory() { /*FOLD00*/

    // free filenames from all entries
    while(FPHistoryCount--)
    {
        free(FPHistory[FPHistoryCount]->FileName);
        free(FPHistory[FPHistoryCount]);
    }

    // free history list
    free(FPHistory);

    // free input history
    {
        while(inputHistory.Count--)
        {
            free(inputHistory.Line[inputHistory.Count]);
        }
        free(inputHistory.Line);
        free(inputHistory.Id);
    }
}

int SaveHistory(char *FileName) { /*FOLD00*/
    FILE *fp;
    
    fp = fopen(FileName, "w");
    if (fp == 0)
        return 0;
    setvbuf(fp, FileBuffer, _IOFBF, sizeof(FileBuffer));
    fprintf(fp, HISTORY_VER);
    if (FPHistory) { // file position history
        int i;
        for (i = 0; i < FPHistoryCount; i++)
            fprintf(fp, "F|%d|%d|%s\n",
                    FPHistory[i]->Row,
                    FPHistory[i]->Col,
                    FPHistory[i]->FileName);
    }
    { // input history
        for (int i = 0; i < inputHistory.Count; i++) {
            fprintf(fp, "I|%d|%s\n", inputHistory.Id[i], inputHistory.Line[i]);
        }
    }
    fclose(fp);
    return 1;
}

int LoadHistory(char *FileName) { /*fold00*/
    FILE *fp;
    char line[2048];
    char *p, *e;
    
    fp = fopen(FileName, "r");
    if (fp == 0)
        return 0;

    setvbuf(fp, FileBuffer, _IOFBF, sizeof(FileBuffer));

    if (fgets(line, sizeof(line), fp) == 0 ||
        strcmp(line, HISTORY_VER) != 0)
    {
        fclose(fp);
        return 0;
    }
    while (fgets(line, sizeof(line), fp) != 0) {
        if (line[0] == 'F' && line[1] == '|') { // input history
            int r, c;
            p = line + 2;
            r = strtol(p, &e, 10);
            if (e == p)
                break;
            if (*e == '|')
                e++;
            else
                break;
            c = strtol(p = e, &e, 10);
            if (e == p)
                break;
            if (*e == '|')
                e++;
            else
                break;
            e = strchr(p = e, '\n');
            if (e == 0)
                break;
            *e = 0;
            if (UpdateFPos(p, r, c) == 0)
                break;
        } else if (line[0] == 'I' && line[1] == '|') { // file position history
            int i;
            
            p = line + 2;
            i = strtol(p, &e, 10);
            if (e == p)
                break;
            if (*e == '|')
                e++;
            else
                break;
            e = strchr(p = e, '\n');
            if (e == 0)
                break;
            *e = 0;
            AddInputHistory(i, p);
        }
    }
    fclose(fp);
    return 1;
}

int UpdateFPos(char *FileName, int Row, int Col) { /*fold00*/
    int L = 0, R = FPHistoryCount, M, N;
    FPosHistory *fp, **NH;
    int cmp;

    if (FPHistory != 0) {
        while (L < R) {
            M = (L + R) / 2;
            cmp = filecmp(FileName, FPHistory[M]->FileName);
            if (cmp == 0) {
                FPHistory[M]->Row = Row;
                FPHistory[M]->Col = Col;
                return 1;
            } else if (cmp < 0) {
                R = M;
            } else {
                L = M + 1;
            }
        }
    } else {
        FPHistoryCount = 0;
        L = 0;
    }
    assert(L >= 0 && L <= FPHistoryCount);
    fp = (FPosHistory *)malloc(sizeof(FPosHistory));
    if (fp == 0)
        return 0;
    fp->Row = Row;
    fp->Col = Col;
    fp->FileName = strdup(FileName);
    if (fp->FileName == 0) {
        free(fp);
        return 0;
    }

    N = 64;
    while (N <= FPHistoryCount) N *= 2;
    NH = (FPosHistory **)realloc((void *)FPHistory, N * sizeof(FPosHistory *));
    if (NH == 0)
    {
        free(fp->FileName);
        free(fp);
        return 0;
    }

    FPHistory = NH;
    
    if (L < FPHistoryCount)
        memmove(FPHistory + L + 1,
                FPHistory + L,
                (FPHistoryCount - L) * sizeof(FPosHistory *));
    FPHistory[L] = fp;
    FPHistoryCount++;
    return 1;
}

int RetrieveFPos(char *FileName, int &Row, int &Col) { /*FOLD00*/
    int L = 0, R = FPHistoryCount, M;
    int cmp;

    if (FPHistory == 0)
        return 0;

    while (L < R) {
        M = (L + R) / 2;
        cmp = filecmp(FileName, FPHistory[M]->FileName);
        if (cmp == 0) {
            Row = FPHistory[M]->Row;
            Col = FPHistory[M]->Col;
            return 1;
        } else if (cmp < 0) {
            R = M;
        } else {
            L = M + 1;
        }
    }
    return 0;
}

int AddInputHistory(int Id, char *String) { /*fold00*/
    if (inputHistory.Count < MAX_INPUT_HIST) {
        inputHistory.Count++;
        inputHistory.Line = (char **) realloc((void *) inputHistory.Line,
                                              inputHistory.Count * sizeof(char *));
        inputHistory.Id = (int *) realloc((void *) inputHistory.Id,
                                          inputHistory.Count * sizeof(int *));
    } else {
        free(inputHistory.Line[inputHistory.Count - 1]);
    }
    memmove(inputHistory.Line + 1,
            inputHistory.Line,
            (inputHistory.Count - 1) * sizeof(char *));
    memmove(inputHistory.Id + 1,
            inputHistory.Id,
            (inputHistory.Count - 1) * sizeof(int *));
    inputHistory.Id[0] = Id;
    inputHistory.Line[0] = strdup(String);
    return 1;
}

int CountInputHistory(int Id) { /*fold00*/
    int i, c = 0;
    
    for (i = 0; i < inputHistory.Count; i++)
        if (inputHistory.Id[i] == Id) c++;
    return c;
}

int GetInputHistory(int Id, char *String, int len, int Nth) { /*fold00*/
    int i = 0;

    assert(len > 0);
    
    while (i < inputHistory.Count) {
        if (inputHistory.Id[i] == Id) {
            Nth--;
            if (Nth == 0) {
                strncpy(String, inputHistory.Line[i], len);
                String[len - 1] = 0;
                return 1;
            }
        }
        i++;
    }
    return 0;
}
#endif
