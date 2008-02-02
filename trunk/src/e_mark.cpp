/*
 * e_mark.cpp
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

EMarkIndex markIndex;

EMark::EMark(char *aName, char *aFileName, EPoint aPoint, EBuffer *aBuffer) {
    Name = new char[strlen(aName) + 1];
    FileName = new char[strlen(aFileName) + 1];
    Buffer = 0;
    Point = aPoint;
    assert(Name != 0);
    assert(FileName != 0);
    strcpy(Name, aName);
    strcpy(FileName, aFileName);
    if (aBuffer == 0)
        aBuffer = FindFile(aFileName);
    if (aBuffer && aBuffer->Loaded)
        setBuffer(aBuffer);
    else
        aBuffer = 0;
}

EMark::~EMark() {
    if (Buffer)
        removeBuffer(Buffer);
    delete[] Name;
    delete[] FileName;
}

int EMark::setBuffer(EBuffer *aBuffer) {
    assert(aBuffer != 0);
    assert(filecmp(aBuffer->FileName, FileName) == 0);

    if (Point.Row >= aBuffer->RCount)
        Point.Row = aBuffer->RCount - 1;
    if (Point.Row < 0)
        Point.Row = 0;

    if (aBuffer->PlaceBookmark(Name, Point) == 1) {
        Buffer = aBuffer;
        return 1;
    }
    return 0;
}

int EMark::removeBuffer(EBuffer *aBuffer) {
    assert(aBuffer != 0);
    if (Buffer == 0 || Buffer != aBuffer)
        return 0;
    assert(filecmp(aBuffer->FileName, FileName) == 0);

    if (Buffer->GetBookmark(Name, Point) == 0)
        return 0;
    if (Buffer->RemoveBookmark(Name) == 0)
        return 0;

    Buffer = 0;
    return 1;
}

EPoint &EMark::getPoint() {
    if (Buffer) {
        assert(Buffer->GetBookmark(Name, Point) != 0);
    }
    return Point;
}

EMarkIndex::EMarkIndex() {
    markCount = 0;
    marks = 0;
}

EMarkIndex::~EMarkIndex() {
    if (markCount > 0 && marks) {
        for (int n = 0; n < markCount; n++)
            delete marks[n];
        free(marks);
        marks = 0;
    }
}

EMark *EMarkIndex::insert(char *aName, char *aFileName, EPoint aPoint, EBuffer *aBuffer) {
    int L = 0, R = markCount, M, cmp;

    assert(aName != 0 && aName[0] != 0);
    assert(aFileName != 0 && aFileName[0] != 0);

    while (L < R) {
        M = (L + R) / 2;
        cmp = strcmp(aName, marks[M]->getName());
        if (cmp == 0)
            return 0;
        else if (cmp > 0)
            L = M + 1;
        else
            R = M;
    }

    EMark **newMarks = (EMark **)realloc(marks,
                                         sizeof(marks[0]) * (markCount + 1));
    if (newMarks == 0)
        return 0;
    marks = newMarks;

    EMark *m = new EMark(aName, aFileName, aPoint, aBuffer);
    if (m == 0)
        return 0;

    memmove(marks + L + 1, marks + L, sizeof(marks[0]) * (markCount - L));
    markCount++;
    marks[L] = m;
    return m;
}

EMark *EMarkIndex::insert(char *aName, EBuffer *aBuffer, EPoint aPoint) {
    assert(aName != 0 && aName[0] != 0);
    assert(aBuffer != 0);
    assert(aBuffer->FileName != 0);

    return insert(aName, aBuffer->FileName, aPoint, aBuffer);
}

EMark *EMarkIndex::locate(char *aName) {
    int L = 0, R = markCount, M, cmp;

    assert(aName != 0 && aName[0] != 0);

    while (L < R) {
        M = (L + R) / 2;
        cmp = strcmp(aName, marks[M]->getName());
        if (cmp == 0)
            return marks[M];
        else if (cmp > 0)
            L = M + 1;
        else
            R = M;
    }
    return 0;
}

int EMarkIndex::remove(char *aName) {
    int L = 0, R = markCount, M, cmp;

    assert(aName != 0 && aName[0] != 0);

    while (L < R) {
        M = (L + R) / 2;
        cmp = strcmp(aName, marks[M]->getName());
        if (cmp == 0) {
            EMark *m = marks[M];

            memmove(marks + M,
                    marks + M + 1,
                    sizeof(marks[0]) * (markCount - M - 1));
            markCount--;

            EMark **newMarks = (EMark **)realloc(marks,
                                                 sizeof(marks[0]) * (markCount));
            if (newMarks != 0 || markCount == 0)
                marks = newMarks;

            delete m;
            return 1;
        } else if (cmp > 0)
            L = M + 1;
        else
            R = M;
    }
    return 0;
}

int EMarkIndex::view(EView *aView, char *aName) {
    EMark *m = locate(aName);
    if (m) {
        EBuffer *b = m->getBuffer();
        if (b == 0) {
            if (FileLoad(0, m->getFileName(), 0, aView) == 0)
                return 0;
            if (retrieveForBuffer((EBuffer *)ActiveModel) == 0)
                return 0;
            b = (EBuffer *)ActiveModel;
        }
        aView->SwitchToModel(b);
        return b->GotoBookmark(m->getName());
    }
    return 0;
}

int EMarkIndex::retrieveForBuffer(EBuffer *aBuffer) {
    for (int n = 0; n < markCount; n++)
        if (marks[n]->getBuffer() == 0 &&
                filecmp(aBuffer->FileName, marks[n]->getFileName()) == 0) {
            if (marks[n]->setBuffer(aBuffer) == 0)
                return 0;
        }
    return 1;
}

int EMarkIndex::storeForBuffer(EBuffer *aBuffer) {
    for (int n = 0; n < markCount; n++)
        if (marks[n]->getBuffer() == aBuffer)
            if (marks[n]->removeBuffer(aBuffer) == 0)
                return 0;
    return 1;
}

int EMarkIndex::saveToDesktop(FILE *fp) {
    for (int n = 0; n < markCount; n++) {
        EPoint p = marks[n]->getPoint();

        // ??? file of buffer or of mark? (different if file renamed) ???
        // perhaps marks should be duplicated?
        fprintf(fp, "M|%d|%d|%s|%s\n",
                p.Row, p.Col,
                marks[n]->getName(),
                marks[n]->getFileName());
    }
    return 1;
}

// needs performance fixes (perhaps a redesign ?)

EMark *EMarkIndex::pushMark(EBuffer *aBuffer, EPoint P) {
    int stackTop = -1;

    for (int n = 0; n < markCount; n++) {
        char *name = marks[n]->getName();
        if (name && name[0] == '#' && isdigit(name[1])) {
            int no = atoi(name + 1);
            if (no > stackTop)
                stackTop = no;
        }
    }
    char name[20];
    sprintf(name, "#%d", stackTop + 1);
    return insert(name, aBuffer, P);
}

int EMarkIndex::popMark(EView *aView) {
    int stackTop = -1;

    for (int n = 0; n < markCount; n++) {
        char *name = marks[n]->getName();
        if (name && name[0] == '#' && isdigit(name[1])) {
            int no = atoi(name + 1);
            if (no > stackTop)
                stackTop = no;
        }
    }
    if (stackTop == -1)
        return 0;
    char name[20];
    sprintf(name, "#%d", stackTop);
    if (view(aView, name) == 0)
        return 0;
    assert(remove(name) == 1);
    return 1;
}
