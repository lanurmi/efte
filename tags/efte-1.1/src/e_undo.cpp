/*    e_undo.cpp
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

int EBuffer::NextCommand() {
    if (Match.Row != -1) {
        Draw(Match.Row, Match.Row);
        Match.Col = Match.Row = -1;
    }
    if (View)
        View->SetMsg(0);
    return BeginUndo();
}

int EBuffer::PushBlockData() {
    if (BFI(this, BFI_Undo) == 0) return 1;
    if (PushULong(BB.Col) == 0) return 0;
    if (PushULong(BB.Row) == 0) return 0;
    if (PushULong(BE.Col) == 0) return 0;
    if (PushULong(BE.Row) == 0) return 0;
    if (PushULong(BlockMode) == 0) return 0;
    if (PushUChar(ucBlock) == 0) return 0;
    return 1;
}

int EBuffer::BeginUndo() {
    US.NextCmd = 1;
    return 1;
}

int EBuffer::EndUndo() {
    int N = US.Num - 1;

    assert(N >= 0);
    if (N >= 1) {
        int Order = 1;

        while (Order < N) Order <<= 1;

        US.Data = (void **) realloc(US.Data, sizeof(void *) * Order);
        US.Top = (int *) realloc(US.Top, sizeof(int) * Order);
        US.Num--;
    } else {
        free(US.Data);
        US.Data = 0;
        free(US.Top);
        US.Top = 0;
        US.Num = 0;
    }
    return 1;
}

int EBuffer::PushULong(unsigned long l) {
    return PushUData(&l, sizeof(unsigned long));
}

int EBuffer::PushUChar(unsigned char ch) {
    return PushUData(&ch, sizeof(unsigned char));
}


int EBuffer::PushUData(void *data, int len) {
    int N;
    int Order = 1;

    if (BFI(this, BFI_Undo) == 0) return 0;
    if (US.Record == 0) return 1;
    if (US.NextCmd || US.Num == 0 || US.Data == 0 || US.Top == 0) {
        N = US.Num;
        if ((BFI(this, BFI_UndoLimit) == -1) || (US.Undo) || (US.Num < BFI(this, BFI_UndoLimit))) {
            N++;
            US.Data = (void **) realloc(US.Data, sizeof(void *) * (N | 255));
            US.Top = (int *)   realloc(US.Top,  sizeof(int)    * (N | 255));
            if (US.Num == US.UndoPtr && !US.Undo)
                US.UndoPtr++;
            US.Num++;
        } else {
            N = US.Num;
            free(US.Data[0]);
            memmove(US.Data, US.Data + 1, (N - 1) * sizeof(US.Data[0]));
            memmove(US.Top,  US.Top  + 1, (N - 1) * sizeof(US.Top[0]));
        }
        assert(US.Data);
        assert(US.Top);
        N = US.Num - 1;
        US.Data[N] = 0;
        US.Top[N] = 0;
        if (US.NextCmd == 1) {
            US.NextCmd = 0;
            if (PushULong(CP.Col) == 0) return 0;
            if (PushULong(CP.Row) == 0) return 0;
            if (PushUChar(ucPosition) == 0) return 0;
        }
        US.NextCmd = 0;
    }

    N = US.Num - 1;
    assert(N >= 0);

    if (US.Undo == 0) US.UndoPtr = US.Num;

    while (Order < (US.Top[N] + len)) Order <<= 1;
    US.Data[N] = realloc(US.Data[N], Order);
    memcpy((char *) US.Data[N] + US.Top[N], data, len);
    US.Top[N] += len;
    return 1;
}

int EBuffer::GetUData(int No, int pos, void **data, int len) {
    int N;

    if (No == -1)
        N = US.Num - 1;
    else
        N = No;

    if (BFI(this, BFI_Undo) == 0) return 0;
    if (N < 0) return 0;
    if (US.Data[N] == 0) return 0;
    if (US.Top[N] == 0) return 0;

    if (pos == -1)
        pos = US.Top[N];


    if (pos == 0)
        return 0;

    assert(pos >= len);
    *data = ((char *) US.Data[N]) + pos - len;
    return 1;
}

// TODO: void *d is dangerous, this needs fixed!
#define UGETC(rc,no,pos,what) \
    do { void *d = 0; \
    if ((rc = GetUData(no, pos, &d, sizeof(unsigned char)))) \
    *(unsigned char *)&what = *(unsigned char *)d; \
    pos -= sizeof(unsigned char); \
    } while (0)

// TODO: void *d is dangerous, this needs fixed!
#define UGET(rc,no,pos,what) \
    do { void *d = 0; \
    if ((rc = GetUData(no, pos, &d, sizeof(what)))) \
    memcpy((void *)&what, d, sizeof(what)); \
    pos -= sizeof(what); \
    } while (0)

int EBuffer::Undo(int undo) {
    unsigned char UndoCmd;
    int rc;
    unsigned long Line;
    unsigned long Len;
    unsigned long ACount;
    unsigned long Col;
    void *data;

    int No;
    int Pos;

    if (BFI(this, BFI_Undo) == 0)
        return 0;

    if (undo)
        No = US.UndoPtr - 1;
    else
        No = US.Num - 1;

    Pos = US.Top[No];

    if (No == 0 && Pos == 0) {
        return 0;
    }
    UGETC(rc, No, Pos, UndoCmd);
    while (rc == 1) {
        switch (UndoCmd) {
        case ucInsLine:
            UGET(rc, No, Pos, Line);
            if (rc == 0) return 0;
            if (DelLine(Line) == 0) return 0;
            break;

        case ucDelLine:
            UGET(rc, No, Pos, Line);
            if (rc == 0) return 0;
            UGET(rc, No, Pos, Len);
            if (rc == 0) return 0;
            if (GetUData(No, Pos, &data, Len) == 0) return 0;
            if (InsLine(Line, 0) == 0) return 0;
            if (InsText(Line, 0, Len, (char *) data) == 0) return 0;
            Pos -= Len;
            break;

        case ucInsChars:
            UGET(rc, No, Pos, ACount);
            if (rc == 0) return 0;
            UGET(rc, No, Pos, Col);
            if (rc == 0) return 0;
            UGET(rc, No, Pos, Line);
            if (rc == 0) return 0;
            if (DelChars(Line, Col, ACount) == 0) return 0;
            break;

        case ucDelChars:
            UGET(rc, No, Pos, Line);
            if (rc == 0) return 0;
            UGET(rc, No, Pos, Col);
            if (rc == 0) return 0;
            UGET(rc, No, Pos, ACount);
            if (rc == 0) return 0;
            if (GetUData(No, Pos, &data, ACount) == 0) return 0;
            if (InsChars(Line, Col, ACount, (char *) data) == 0) return 0;
            Pos -= ACount;
            break;

        case ucPosition:
            UGET(rc, No, Pos, Line);
            if (rc == 0) return 0;
            UGET(rc, No, Pos, Col);
            if (rc == 0) return 0;
            if (SetPos(Col, Line) == 0) return 0;
            break;

        case ucBlock: {
            EPoint P;
            unsigned long l;

            UGET(rc, No, Pos, l);
            if (rc == 0) return 0;
            if (BlockMode != (int)l) BlockRedraw();
            BlockMode = l;
            UGET(rc, No, Pos, l);
            if (rc == 0) return 0;
            P.Row = l;
            UGET(rc, No, Pos, l);
            if (rc == 0) return 0;
            P.Col = l;
            if (SetBE(P) == 0) return 0;
            UGET(rc, No, Pos, l);
            if (rc == 0) return 0;
            P.Row = l;
            UGET(rc, No, Pos, l);
            if (rc == 0) return 0;
            P.Col = l;
            if (SetBB(P) == 0) return 0;
        }
        break;

        case ucFoldCreate:
            UGET(rc, No, Pos, Line);
            if (rc == 0) return 0;
            if (FoldDestroy(Line) == 0) return 0;
            break;

        case ucFoldDestroy:
        {
            unsigned long level;
            int ff;

            UGET(rc, No, Pos, Line);
            if (rc == 0) return 0;
            UGET(rc, No, Pos, level);
            if (rc == 0) return 0;
            if (FoldCreate(Line) == 0) return 0;

            ff = FindFold(Line);
            assert(ff != -1);
            FF[ff].level = (unsigned char) level;
        }
        break;
        case ucFoldPromote:
            UGET(rc, No, Pos, Line);
            if (rc == 0) return 0;
            if (FoldDemote(Line) == 0) return 0;
            break;

        case ucFoldDemote:
            UGET(rc, No, Pos, Line);
            if (rc == 0) return 0;
            if (FoldPromote(Line) == 0) return 0;
            break;

        case ucFoldOpen:
            UGET(rc, No, Pos, Line);
            if (rc == 0) return 0;
            if (FoldClose(Line) == 0) return 0;
            break;

        case ucFoldClose:
            UGET(rc, No, Pos, Line);
            if (rc == 0) return 0;
            if (FoldOpen(Line) == 0) return 0;
            break;

        case ucModified:
            Modified = 0;
            break;

        case ucPlaceUserBookmark:
            UGET(rc, No, Pos, ACount);
            if (rc == 0) return 0;
            if (GetUData(No, Pos, &data, ACount) == 0) return 0;
            Pos -= ACount;
            UGET(rc, No, Pos, Col);
            if (rc == 0) return 0;
            UGET(rc, No, Pos, Line);
            if (rc == 0) return 0;
            if (Col == (unsigned long) - 1 || Line == (unsigned long) - 1) {
                if (RemoveUserBookmark((const char *)data) == 0) return 0;
            } else {
                if (PlaceUserBookmark((const char *)data, EPoint(Line, Col)) == 0) return 0;
            }
            break;

        case ucRemoveUserBookmark:
            UGET(rc, No, Pos, ACount);
            if (rc == 0) return 0;
            if (GetUData(No, Pos, &data, ACount) == 0) return 0;
            Pos -= ACount;
            UGET(rc, No, Pos, Col);
            if (rc == 0) return 0;
            UGET(rc, No, Pos, Line);
            if (rc == 0) return 0;
            if (PlaceUserBookmark((const char *)data, EPoint(Line, Col)) == 0) return 0;
            break;

        default:
            assert(1 == "Oops: invalid undo command.\n"[0]);
        }
        UGETC(rc, No, Pos, UndoCmd);
    }

    if (undo)
        US.UndoPtr--;
    else {
        US.UndoPtr++;
        free(US.Data[No]);
        if (EndUndo() == 0) return 0;
    }

    return 1;
}

int EBuffer::Redo() {
    int rc;

    if (BFI(this, BFI_Undo) == 0) return 0;

    if (US.Num == 0 || US.UndoPtr == US.Num) {
        Msg(S_INFO, "Nothing to redo.");
        return 0;
    }

    US.Record = 0;
    rc =  Undo(0);
    US.Record = 1;
    return rc;
}

int EBuffer::Undo() {
    int rc;

    if (BFI(this, BFI_Undo) == 0) return 0;

    assert(US.Num >= 0);
    assert(US.UndoPtr >= 0);
    if (US.Num == 0 || US.UndoPtr == 0) {
        Msg(S_INFO, "Nothing to undo.");
        return 0;
    }

    US.Undo = 1;
    rc = Undo(1);
    US.Undo = 0;
    return rc;
}
