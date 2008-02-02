/*    o_messages.cpp
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"
#include "s_files.h"
#include "c_commands.h"

#define MAXREGEXP  32

EMessages *CompilerMsgs = 0;

static int NCRegexp = 0;
static struct {
    int RefFile;
    int RefLine;
    int RefMsg;
    RxNode *rx;
} CRegexp[MAXREGEXP];

int AddCRegexp(int file, int line, int msg, const char *regexp) {
    if (NCRegexp >= MAXREGEXP) return 0;
    CRegexp[NCRegexp].RefFile = file;
    CRegexp[NCRegexp].RefLine = line;
    CRegexp[NCRegexp].RefMsg = msg;
    if ((CRegexp[NCRegexp].rx = RxCompile(regexp)) == NULL) {
        return 0;
    }
    NCRegexp++;
    return 1;
}

void FreeCRegexp() {
    while (NCRegexp--) {
        RxFree(CRegexp[NCRegexp].rx);
    }
}

EMessages::EMessages(int createFlags, EModel **ARoot, char *ADir, char *ACommand): EList(createFlags, ARoot, "Messages") {
    CompilerMsgs = this;
    ErrCount = 0;
    ErrList = 0;
    Running = 0;
    BufLen = 0;
    BufPos = 0;
    Command = 0;
    Directory = 0;
    MatchCount = 0;
    ReturnCode = -1;
    Running = 1;
    curr_dir = 0;
    RunPipe(ADir, ACommand);
}

EMessages::~EMessages() {
    gui->ClosePipe(PipeId);
    FreeErrors();
    free(Command);
    free(Directory);
    CompilerMsgs = 0;
    freeDirStack();
}

void EMessages::freeDirStack() {
    while (curr_dir != 0) {
        aDir *a = curr_dir;
        curr_dir = curr_dir->next;
        free(a->name);
        delete a;
    }
}

void EMessages::NotifyDelete(EModel *Deleting) {
    for (int i = 0; i < ErrCount; i++) {
        if (ErrList[i]->Buf == Deleting) {
            /* NOT NEEDED!
             char bk[16];
             sprintf(bk, "_MSG.%d", i);
             ((EBuffer *)Deleting)->RemoveBookmark(bk);
             */
            ErrList[i]->Buf = 0;
        }
    }
}

void EMessages::FindErrorFiles() {
    for (int i = 0; i < ErrCount; i++)
        if (ErrList[i]->Buf == 0 && ErrList[i]->file != 0)
            FindErrorFile(i);
}

void EMessages::FindErrorFile(int err) {
    assert(err >= 0 && err < ErrCount);
    if (ErrList[err]->file == 0)
        return ;

    EBuffer *B;

    ErrList[err]->Buf = 0;

    B = FindFile(ErrList[err]->file);
    if (B == 0)
        return ;

    if (B->Loaded == 0)
        return;

    AddFileError(B, err);
}

void EMessages::AddFileError(EBuffer *B, int err) {
    char bk[16];
    EPoint P;

    assert(err >= 0 && err < ErrCount);

    sprintf(bk, "_MSG.%d", err);
    P.Col = 0;
    P.Row = ErrList[err]->line - 1; // offset 0


    if (P.Row >= B->RCount)
        P.Row = B->RCount - 1;
    if (P.Row < 0)
        P.Row = 0;

    if (B->PlaceBookmark(bk, P) == 1)
        ErrList[err]->Buf = B;
}

void EMessages::FindFileErrors(EBuffer *B) {
    for (int i = 0; i < ErrCount; i++)
        if (ErrList[i]->Buf == 0 && ErrList[i]->file != 0) {
            if (filecmp(B->FileName, ErrList[i]->file) == 0) {
                AddFileError(B, i);
            }
        }
}

int EMessages::RunPipe(char *ADir, char *ACommand) {
    if (!KeepMessages)
        FreeErrors();

    free(Command);
    free(Directory);

    Command = strdup(ACommand);
    Directory = strdup(ADir);

    MatchCount = 0;
    ReturnCode = -1;
    Running = 1;
    BufLen = BufPos = 0;
    Row = ErrCount - 1;

    {
        char s[2 * MAXPATH * 4];

        sprintf(s, "[running '%s' in '%s']", Command, Directory);
        AddError(0, -1, 0, s);
    }

    {
        char s[MAXPATH * 2];
        sprintf(s, "Messages [%s]: %s", Directory, Command);
        SetTitle(s);
    }

    ChangeDir(Directory);
    PipeId = gui->OpenPipe(Command, this);
    return 0;
}

EEventMap *EMessages::GetEventMap() {
    return FindEventMap("MESSAGES");
}

int EMessages::ExecCommand(int Command, ExState &State) {
    switch (Command) {
    case ExChildClose:
        if (Running == 0 || PipeId == -1)
            break;
        ReturnCode = gui->ClosePipe(PipeId);
        PipeId = -1;
        Running = 0;
        {
            char s[30];

            sprintf(s, "[aborted, status=%d]", ReturnCode);
            AddError(0, -1, 0, s);
        }
        return ErOK;

    case ExActivateInOtherWindow:
        ShowError(View->Next, Row);
        return ErOK;
    }
    return EList::ExecCommand(Command, State);
}

void EMessages::AddError(Error *p) {
    ErrCount++;
    ErrList = (Error **) realloc(ErrList, sizeof(void *) * ErrCount);
    ErrList[ErrCount - 1] = p;
    ErrList[ErrCount - 1]->Buf = 0;
    FindErrorFile(ErrCount - 1);

    if (ErrCount > Count)
        if (Row >= Count - 1) {
            //if (ErrCount > 1 && !ErrList[TopRow]->file)
            Row = ErrCount - 1;
        }

    UpdateList();
}

void EMessages::AddError(char *file, int line, char *msg, const char *text, int hilit) {
    Error *pe;

    pe = (Error *) malloc(sizeof(Error));
    if (pe == 0)
        return ;

    pe->file = file ? strdup(file) : 0;
    pe->line = line;
    pe->msg = msg ? strdup(msg) : 0;
    pe->text = text ? strdup(text) : 0;
    pe->hilit = hilit;

    AddError(pe);
}

void EMessages::FreeErrors() {
    if (ErrList) {
        for (int i = 0; i < ErrCount; i++) {
            if (ErrList[i]->Buf != 0) {
                char bk[16];
                sprintf(bk, "_MSG.%d", i);
                ((EBuffer *)(ErrList[i]->Buf))->RemoveBookmark(bk);
            }
            free(ErrList[i]->msg);
            free(ErrList[i]->text);
            free(ErrList[i]->file);
            free(ErrList[i]);
        }
        free(ErrList);
    }
    ErrCount = 0;
    ErrList = 0;
    BufLen = BufPos = 0;
}

int EMessages::GetLine(char *Line, int maxim) {
    int rc;
    char *p;
    int l;

    //fprintf(stderr, "GetLine: %d\n", Running);

    *Line = 0;
    if (Running && PipeId != -1) {
        rc = gui->ReadPipe(PipeId, MsgBuf + BufLen, sizeof(MsgBuf) - BufLen);
        //fprintf(stderr, "GetLine: ReadPipe rc = %d\n", rc);
        if (rc == -1) {
            ReturnCode = gui->ClosePipe(PipeId);
            PipeId = -1;
            Running = 0;
        }
        if (rc > 0)
            BufLen += rc;
    }
    l = maxim - 1;
    if (BufLen - BufPos < l)
        l = BufLen - BufPos;
    //fprintf(stderr, "GetLine: Data %d\n", l);
    p = (char *)memchr(MsgBuf + BufPos, '\n', l);
    if (p) {
        *p = 0;
        strcpy(Line, MsgBuf + BufPos);
        l = strlen(Line);
        if (l > 0 && Line[l - 1] == '\r')
            Line[l - 1] = 0;
        BufPos = p + 1 - MsgBuf;
        //fprintf(stderr, "GetLine: Line %d\n", strlen(Line));
    } else if (Running && sizeof(MsgBuf) != BufLen) {
        memmove(MsgBuf, MsgBuf + BufPos, BufLen - BufPos);
        BufLen -= BufPos;
        BufPos = 0;
        //fprintf(stderr, "GetLine: Line Incomplete\n");
        return 0;
    } else {
        if (l == 0)
            return 0;
        memcpy(Line, MsgBuf + BufPos, l);
        Line[l] = 0;
        if (l > 0 && Line[l - 1] == '\r')
            Line[l - 1] = 0;
        BufPos += l;
        //fprintf(stderr, "GetLine: Line Last %d\n", l);
    }
    memmove(MsgBuf, MsgBuf + BufPos, BufLen - BufPos);
    BufLen -= BufPos;
    BufPos = 0;
    //fprintf(stderr, "GetLine: Got Line\n");
    return 1;
}


static void getWord(char* dest, char*& pin) {
    char *pout, *pend;
    char ch, ec;

    while (*pin == ' ' || *pin == '\t')
        pin++;

    pout = dest;
    pend = dest + 256 - 1;
    if (*pin == '\'' || *pin == '"' || *pin == '`') {
        ec = *pin++;
        if (ec == '`')
            ec = '\'';
        for (;;) {
            ch  = *pin++;
            if (ch == '`')
                ch = '\'';
            if (ch == ec || ch == 0)
                break;

            if (pout < pend)
                *pout++ = ch;
        }
        if (ch == 0)
            pin--;
    } else {
        for (;;) {
            ch  = *pin++;
            if (ch == ' ' || ch == '\t' || ch == 0)
                break;
            if (pout < pend) *pout++ = ch;
        }
    }
    *pout = 0;
}


void EMessages::GetErrors() {
    char line[4096];
    RxMatchRes RM;
    //int retc;
    int i, n;
    int didmatch = 0;
    int WasRunning = Running;
    char fn[256];

    //fprintf(stderr, "Reading pipe\n");
    while (GetLine((char *)line, sizeof(line))) {
        if (strlen(line) > 0 && line[strlen(line)-1] == '\n') line[strlen(line)-1] = 0;
        didmatch = 0;
        for (i = 0; i < NCRegexp; i++) {
            if (RxExec(CRegexp[i].rx, line, strlen(line), line, &RM) == 1) {
                char ln[256] = "";
                char msg[256] = "";
                char fn1[256] = "";
                char fn2[256] = "";
                char *file = 0;
                *fn = 0;

                memset(fn, 0, 256);
                memset(ln, 0, 256);
                memset(msg, 0, 256);
                n = CRegexp[i].RefFile;
                if (RM.Close[n] - RM.Open[n] < 256)
                    memcpy(fn, line + RM.Open[n], RM.Close[n] - RM.Open[n]);
                n = CRegexp[i].RefLine;
                if (RM.Close[n] - RM.Open[n] < 256)
                    memcpy(ln, line + RM.Open[n], RM.Close[n] - RM.Open[n]);
                n = CRegexp[i].RefMsg;
                if (RM.Close[n] - RM.Open[n] < 256)
                    memcpy(msg, line + RM.Open[n], RM.Close[n] - RM.Open[n]);

                if (IsFullPath(fn))
                    file = fn;
                else {
                    if (curr_dir == 0)
                        strcpy(fn1, Directory);
                    else
                        strcpy(fn1, curr_dir->name);
                    Slash(fn1, 1);
                    strcat(fn1, fn);
                    if (ExpandPath(fn1, fn2, sizeof(fn2)) == 0)
                        file = fn2;
                    else
                        file = fn1;
                }
                AddError(file, atoi(ln), msg[0] ? msg : 0, line, 1);
                didmatch = 1;
                MatchCount++;
                break;
            }
        }
        if (!didmatch) {
            AddError(0, -1, 0, line);
            //** Quicky: check for gnumake 'entering directory'
            //** make[x]: entering directory `xxx'
            //** make[x]: leaving...
            static char t1[] = "entering directory";
            static char t2[] = "leaving directory";
            char*   pin;

            if ((pin = strstr(line, "]:")) != 0) {
                //** It *is* some make line.. Check for 'entering'..
                pin += 2;

                //while(*pin != ':' && *pin != 0) pin++;
                //pin++;
                while (*pin == ' ')
                    pin++;
                if (strnicmp(pin, t1, sizeof(t1) - 1) == 0) {  // Entering?
                    //** Get the directory name from the line,
                    pin += sizeof(t1) - 1;
                    getWord(fn, pin);
                    //dbg("entering %s", fn);

                    if (*fn) {
                        //** Indeedy entering directory! Link in list,
                        aDir * a = new aDir;
                        assert(a != 0);
                        a->name = strdup(fn);
                        assert(a->name != 0);
                        a->next = curr_dir;
                        curr_dir = a;
                    }
                } else if (strnicmp(pin, t2, sizeof(t2) - 1) == 0) {  // Leaving?
                    pin += sizeof(t2) - 1;
                    getWord(fn, pin);                   // Get dirname,
                    //dbg("leaving %s", fn);

                    aDir *a;

                    a = curr_dir;
                    if (a != 0)
                        curr_dir = curr_dir->next;       // Remove from stack,
                    if (a != 0 && stricmp(a->name, fn) == 0) {
                        //** Pop this item.
                        free(a->name);
                        delete a;
                    } else {
                        //** Mismatch filenames -> error, and revoke stack.
                        //dbg("mismatch on %s", fn);
                        AddError(0, -1, 0, "efte: mismatch in directory stack!?");

                        //** In this case we totally die the stack..
                        while (a != 0) {
                            free(a->name);
                            delete a;
                            a = curr_dir;
                            if (a != 0)
                                curr_dir = curr_dir->next;
                        }
                    }
                }
            }
        }
    }
    //fprintf(stderr, "Reading Stopped\n");
    if (!Running && WasRunning) {
        char s[30];

        sprintf(s, "[done, status=%d]", ReturnCode);
        AddError(0, -1, 0, s);
    }
    //UpdateList();
    //NeedsUpdate = 1;
}

int EMessages::CompilePrevError(EView *V) {
    if (ErrCount > 0) {
bad:
        if (Row > 0) {
            Row--;
            if (ErrList[Row]->line == -1 || !ErrList[Row]->file) goto bad;
            ShowError(V, Row);
        } else {
            V->Msg(S_INFO, "No previous error.");
            return 0;
        }
    } else {
        V->Msg(S_INFO, "No errors.");
        return 0;
    }
    return 1;
}

int EMessages::CompileNextError(EView *V) {
    if (ErrCount > 0) {
bad:
        if (Row < ErrCount - 1) {
            Row++;
            if (ErrList[Row]->line == -1 || !ErrList[Row]->file) goto bad;
            ShowError(V, Row);
        } else {
            if (Running)
                V->Msg(S_INFO, "No more errors (yet).");
            else
                V->Msg(S_INFO, "No more errors.");
            return 0;
        }
    } else {
        if (Running)
            V->Msg(S_INFO, "No errors (yet).");
        else
            V->Msg(S_INFO, "No errors.");
        return 0;
    }
    return 1;
}

int EMessages::Compile(char * /*Command*/) {
    return 0;
}

void EMessages::ShowError(EView *V, int err) {
    if (err >= 0 && err < ErrCount) {
        if (ErrList[err]->file) {
            // should check if relative path
            // possibly scan for (gnumake) directory info in output
            if (ErrList[err]->Buf != 0) {
                char bk[16];

                V->SwitchToModel(ErrList[err]->Buf);

                sprintf(bk, "_MSG.%d", err);
                ErrList[err]->Buf->GotoBookmark(bk);
            } else {
                if (FileLoad(0, ErrList[err]->file, 0, V) == 1) {
                    V->SwitchToModel(ActiveModel);
                    ((EBuffer *)ActiveModel)->CenterNearPosR(0, ErrList[err]->line - 1);
                }
            }
            if (ErrList[err]->msg != 0)
                V->Msg(S_INFO, "%s", ErrList[err]->msg);
            else
                V->Msg(S_INFO, "%s", ErrList[err]->text);
        }
    }
}

void EMessages::DrawLine(PCell B, int Line, int Col, ChColor color, int Width) {
    if (Line < ErrCount)
        if (Col < int(strlen(ErrList[Line]->text))) {
            char str[1024];
            int len;

            len = UnTabStr(str, sizeof(str),
                           ErrList[Line]->text,
                           strlen(ErrList[Line]->text));

            if (len > Col)
                MoveStr(B, 0, Width, str + Col, color, Width);
        }
}

char* EMessages::FormatLine(int Line) {
    char *p;
    if (Line < ErrCount)
        p = strdup(ErrList[Line]->text);
    else
        p = 0;
    return p;
}

int EMessages::IsHilited(int Line) {
    return (Line >= 0 && Line < ErrCount) ? ErrList[Line]->hilit : 0;
}

void EMessages::UpdateList() {
    Count = ErrCount;
    EList::UpdateList();
}

int EMessages::Activate(int /*No*/) {
    //assert(No == Row);
    //Row = No;
    ShowError(View, Row);
    return 1;
}

int EMessages::CanActivate(int Line) {
    int ok = 0;
    if (Line < ErrCount)
        if (ErrList[Line]->file || ErrList[Line]->line != -1) ok = 1;
    return ok;
}

void EMessages::NotifyPipe(int APipeId) {
    //fprintf(stderr, "Got notified");
    if (APipeId == PipeId)
        GetErrors();
}

void EMessages::GetName(char *AName, int MaxLen) {
    strncpy(AName, "Messages", MaxLen);
}

void EMessages::GetInfo(char *AInfo, int /*MaxLen*/) {
    sprintf(AInfo,
            "%2d %04d/%03d Messages: %d (%s) ",
            ModelNo,
            Row, Count,
            MatchCount,
            Command);
}

void EMessages::GetPath(char *APath, int MaxLen) {
    strncpy(APath, Directory, MaxLen);
    APath[MaxLen - 1] = 0;
    Slash(APath, 0);
}

void EMessages::GetTitle(char *ATitle, int /*MaxLen*/, char *ASTitle, int SMaxLen) {
    sprintf(ATitle, "Messages: %s", Command);
    strncpy(ASTitle, "Messages", SMaxLen);
    ASTitle[SMaxLen - 1] = 0;
}

// get row length for specified row, used in MoveLineEnd to get actual row length
int EMessages::GetRowLength(int ARow) {
    if ((ARow >= 0) && (ARow < ErrCount)) {
        return strlen(ErrList[ARow]->text);
    }

    return 0;
}
