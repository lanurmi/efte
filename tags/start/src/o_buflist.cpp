/*    o_buflist.cpp
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

BufferView *BufferList = 0;

BufferView::BufferView(int createFlags, EModel **ARoot): EList(createFlags, ARoot, "Buffers") {
    ModelNo = 0; // trick
    BList = 0;
    BCount = 0;
}

BufferView::~BufferView() {
    if (BList) {
        for (int i = 0; i < BCount; i++)
            if (BList[i])
                free(BList[i]);
        free(BList);
    }
    BufferList = 0;
}
    
EEventMap *BufferView::GetEventMap() {
    return FindEventMap("BUFFERS");
}

int BufferView::GetContext() {
    return CONTEXT_BUFFERS; 
}

void BufferView::DrawLine(PCell B, int Line, int Col, ChColor color, int Width) {
    if (Line < BCount)
        if (Col < int(strlen(BList[Line])))
            MoveStr(B, 0, Width, BList[Line] + Col, color, Width);
}

char* BufferView::FormatLine(int Line) {
    return strdup(BList[Line]);
}

void BufferView::UpdateList() {
    EModel *B = ActiveModel;
    int No;
    char s[512] = "";
    
    if (BList) {
        for (int i = 0; i < BCount; i++)
            if (BList[i])
                free(BList[i]);
        free(BList);
    }
    BList = 0;
    BCount = 0;
    while (B) {
        BCount++;
        B = B->Next;
        if (B == ActiveModel) break;
    }
    BList = (char **) malloc(sizeof(char *) * BCount);
    assert(BList != 0);
    B = ActiveModel;
    No = 0;
    while (B) {
        B->GetInfo(s, sizeof(s) - 1);
        BList[No++] = strdup(s);
        B = B->Next;
        if (B == ActiveModel) break;
        if (No >= BCount) break;
    }
    Count = BCount;
    NeedsUpdate = 1;
}
    
EModel *BufferView::GetBufferById(int No) {
    EModel *B;
    
    B = ActiveModel;
    while (B) {
        if (No == 0) {
            return B;
        }
        No--;
        B = B->Next;
        if (B == ActiveModel) break;
    }
    return 0;
}
    
int BufferView::ExecCommand(int Command, ExState &State) {
    switch (Command) {
    case ExCloseActivate:
        {
            EModel *B;

            B = GetBufferById(Row);
            if (B && B != this) {
                View->SwitchToModel(B);
                delete this;
                return ErOK;
            }
        }
        return ErFAIL;
    case ExBufListFileClose:
        {
            EModel *B = GetBufferById(Row);

            if (B && B != this && Count > 1) {
                if (B->ConfQuit(View->MView->Win)) {
                    View->DeleteModel(B);
                }
                UpdateList();
                return ErOK;
            }
        }
        return ErFAIL;
    case ExBufListFileSave:
        {
            EModel *B = GetBufferById(Row);

            if (B && B->GetContext() == CONTEXT_FILE)
                if (((EBuffer *)B)->Save())
                    return ErOK;
        }
        return ErFAIL;
        
    case ExActivateInOtherWindow:
        {
            EModel *B = GetBufferById(Row);

            if (B) {
                View->Next->SwitchToModel(B);
                return ErOK;
            }
        }
        return ErFAIL;
    }
    return EList::ExecCommand(Command, State);
}

int BufferView::Activate(int No) {
    EModel *B;
    
    B = GetBufferById(No);
    if (B) {
        View->SwitchToModel(B);
        return 1;
    }
    return 0;
}

void BufferView::GetInfo(char *AInfo, int MaxLen) {
    sprintf(AInfo, "%2d %04d/%03d Buffers", ModelNo, Row + 1, Count);
}

void BufferView::GetTitle(char *ATitle, int MaxLen, char *ASTitle, int SMaxLen) {
    strncpy(ATitle, "Buffers", MaxLen);
    ATitle[MaxLen - 1] = 0;
    strncpy(ASTitle, "Buffers", SMaxLen);
    ASTitle[SMaxLen - 1] = 0;
}
