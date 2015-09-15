/*
 * e_mark.h
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 */
#ifndef EMARK_H_
#define EMARK_H_

#include "e_buffer.h"

#include <stdio.h> // FILE

class EMark {
public:
    EMark(const char *aName, const char *aFileName, EPoint aPoint, EBuffer *aBuffer = 0);
    ~EMark();

    int SetBuffer(EBuffer *aBuffer);
    int RemoveBuffer(EBuffer *aBuffer);

    char *GetName() {
        return Name;
    }
    char *GetFileName() {
        return FileName;
    }
    EPoint &GetPoint();
    EBuffer *GetBuffer() {
        return Buffer;
    }
private:
    /* bookmark */
    char *Name;
    EPoint Point;
    char *FileName;

    /* bookmark in file */
    EBuffer *Buffer;
};

class EMarkIndex {
public:
    EMarkIndex();
    ~EMarkIndex();

    EMark *Insert(const char *aName, const char *aFileName, EPoint aPoint, EBuffer *aBuffer = 0);
    EMark *Insert(const char *aName, EBuffer *aBuffer, EPoint aPoint);
    EMark *Locate(const char *aName);
    int Remove(const char *aName);
    int View(EView *aView, const char *aName);

//    int MarkPush(EBuffer *B, EPoint P);
//    int MarkPop(EView *V);
//    int MarkSwap(EView *V, EBuffer *B, EPoint P);
//    int MarkNext(EView *V);
    //    int MarkPrev(EView *V);
    EMark *PushMark(EBuffer *aBuffer, EPoint P);
    int PopMark(EView *aView);

    int RetrieveForBuffer(EBuffer *aBuffer);
    int StoreForBuffer(EBuffer *aBuffer);

    int SaveToDesktop(FILE *fp);

private:
    int markCount;
    EMark **marks;
};

extern EMarkIndex markIndex;

#endif
