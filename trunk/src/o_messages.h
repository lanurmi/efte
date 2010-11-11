/*    o_messages.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef MESSAGES_H_
#define MESSAGES_H_

typedef struct {
    char *file;
    int line;
    char *msg;
    char *text;
    int hilit;
    EBuffer *Buf;
} Error;

struct aDir {
    aDir*       next;
    char*       name;
};

class EMessages: public EList {
public:
    char *Command;
    char *Directory;

    int ErrCount;
    Error **ErrList;
    int Running;

    int BufLen;
    int BufPos;
    int PipeId;
    int ReturnCode;
    int MatchCount;
    char MsgBuf[4096];
    aDir*   curr_dir;                       // top of dir stack.

    EMessages(int createFlags, EModel **ARoot, char *Dir, char *ACommand);
    ~EMessages();
    void freeDirStack();

    virtual void NotifyDelete(EModel *Deleting);
    void FindErrorFiles();
    void FindErrorFile(int err);
    void AddFileError(EBuffer *B, int err);
    void FindFileErrors(EBuffer *B);

    virtual int GetContext() {
        return CONTEXT_MESSAGES;
    }
    virtual EEventMap *GetEventMap();
    virtual int ExecCommand(int Command, ExState &State);

    void AddError(Error *p);
    void AddError(char *file, int line, char *msg, const char *text, int hilit = 0);

    void FreeErrors();
    int GetLine(char *Line, int maxim);
    void GetErrors();
    int Compile(char *Command);
    void ShowError(EView *V, int err);
    void DrawLine(PCell B, int Line, int Col, ChColor color, int Width);
    char* FormatLine(int Line);
    int IsHilited(int Line);
    void UpdateList();
    int Activate(int No);
    int CanActivate(int Line);
    void NotifyPipe(int APipeId);
    virtual void GetName(char *AName, int MaxLen);
    virtual void GetInfo(char *AInfo, int MaxLen);
    virtual void GetPath(char *APath, int MaxLen);
    virtual void GetTitle(char *ATitle, int MaxLen, char *ASTitle, int SMaxLen);
    virtual int GetRowLength(int ARow);


    int RunPipe(char *Dir, char *Command);

    int CompilePrevError(EView *V);
    int CompileNextError(EView *V);
};

extern EMessages *CompilerMsgs;

void FreeCRegexp();

#endif
