/*    o_model.cpp
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

EModel* ActiveModel = 0;

EModel *FindModelID(EModel *Model, int ID) {
    EModel *M = Model;
    int No = ID;

    while (M) {
        if (M->ModelNo == No)
            return M;
        M = M->Next;
        if (M == Model)
            break;
    }
    return 0;
}

int GetNewModelID(EModel *B) {
    static int lastid = -1;

    if (ReassignModelIds) lastid = 0;   // 0 is used by buffer list
    while (FindModelID(B, ++lastid) != 0) /* */;

    return lastid;
}

EModel::EModel(int createFlags, EModel **ARoot) {
    Root = ARoot;

    if (Root) {
        if (*Root) {
            if (createFlags & cfAppend) {
                Prev = *Root;
                Next = (*Root)->Next;
            } else {
                Next = *Root;
                Prev = (*Root)->Prev;
            }
            Prev->Next = this;
            Next->Prev = this;
        } else
            Prev = Next = this;

        if (!(createFlags & cfNoActivate))
            *Root = this;
    } else
        Prev = Next = this;
    View = 0;
    ModelNo = -1;
    ModelNo = GetNewModelID(this);
}

EModel::~EModel() {
    EModel *D = this;

    while (D) {
        D->NotifyDelete(this);
        D = D->Next;
        if (D == this)
            break;
    }

    if (Next != this) {
        Prev->Next = Next;
        Next->Prev = Prev;
        if (*Root == this)
            *Root = Next;
    } else
        *Root = 0;
}

void EModel::AddView(EView *V) {
    RemoveView(V);
    if (V)
        V->NextView = View;
    View = V;
}

void EModel::RemoveView(EView *V) {
    EView **X = &View;

    if (!V) return;
    while (*X) {
        if ((*X) == V) {
            *X = V->NextView;
            return;
        }
        X = (&(*X)->NextView);
    }
}

void EModel::SelectView(EView *V) {
    RemoveView(V);
    AddView(V);
}

EViewPort *EModel::CreateViewPort(EView * /*V*/) {
    return 0;
}

int EModel::ExecCommand(int /*Command*/, ExState &/*State*/) {
    return ErFAIL;
}

void EModel::HandleEvent(TEvent &/*Event*/) {
}

void EModel::Msg(int level, const char *s, ...) {
    char msgbuftmp[MSGBUFTMP_SIZE];
    va_list ap;

    if (View == 0)
        return;

    va_start(ap, s);
    vsprintf(msgbuftmp, s, ap);
    va_end(ap);

    if (level != S_BUSY)
        View->SetMsg(msgbuftmp);
}

int EModel::CanQuit() {
    return 1;
}

int EModel::ConfQuit(GxView * /*V*/, int /*multiFile*/) {
    return 1;
}

int EModel::GetContext() {
    return CONTEXT_NONE;
}
EEventMap *EModel::GetEventMap() {
    return 0;
}
int EModel::BeginMacro() {
    return 1;
}
void EModel::GetName(char *AName, int /*MaxLen*/) {
    *AName = 0;
}
void EModel::GetPath(char *APath, int /*MaxLen*/) {
    *APath = 0;
}
void EModel::GetInfo(char *AInfo, int /*MaxLen*/) {
    *AInfo = 0;
}
void EModel::GetTitle(char *ATitle, int /*MaxLen*/, char *ASTitle, int /*SMaxLen*/) {
    *ATitle = 0;
    *ASTitle = 0;
}
void EModel::NotifyPipe(int /*PipeId*/) { }

void EModel::NotifyDelete(EModel * /*Deleted*/) {
}
void EModel::DeleteRelated() {
}

EViewPort::EViewPort(EView *V) {
    View = V;
    ReCenter = 0;
}
EViewPort::~EViewPort() {}
void EViewPort::HandleEvent(TEvent &/*Event*/) { }
void EViewPort::UpdateView() { }
void EViewPort::RepaintView() { }
void EViewPort::UpdateStatus() { }
void EViewPort::RepaintStatus() { }
void EViewPort::GetPos() { }
void EViewPort::StorePos() { }
void EViewPort::Resize(int /*Width*/, int /*Height*/) {}

void EModel::UpdateTitle() {
    char Title[256] = ""; //fte: ";
    char STitle[256] = ""; //"fte: ";
    EView *V;

    GetTitle(Title, sizeof(Title) - 1,
             STitle, sizeof(STitle) - 1);

    V = View;
    while (V) {
        V->MView->Win->UpdateTitle(Title, STitle);
        V = V->NextView;
    }
}

int EModel::GetStrVar(int var, char *str, int buflen) {
    switch (var) {
    case mvCurDirectory:
        return GetDefaultDirectory(this, str, buflen);
    }
    return 0;
}

int EModel::GetIntVar(int /*var*/, int * /*value*/) {
    return 0;
}
