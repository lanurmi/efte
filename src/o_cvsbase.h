/*
 * o_cvsbase.h
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 * Contributed by Martin Frydl <frydl@matfyz.cz>
 *
 * Base class for all other CVS-related classes. This is similar to EMessages
 * - starts CVS and shows its messages in list view.
 */

#ifndef CVSBASE_H_
#define CVSBASE_H_

typedef struct {
    char *File; // Relative to view's directory
    int Line;
    char *Msg;
    EBuffer *Buf;
    char Status;
    // bit 0 - hilited
    // bit 1 - marked
    // bit 2 - markable
} CvsLine;

class ECvsBase: public EList {
public:
    char *Command;
    char *Directory;
    char *OnFiles;
    char *OnFilesPos;

    int LineCount;
    CvsLine **Lines;
    int Running;

    int BufLen;
    int BufPos;
    int PipeId;
    int ReturnCode;
    char MsgBuf[4096];

    ECvsBase(int createFlags, EModel **ARoot, const char *ATitle);
    ~ECvsBase();

    void FreeLines();
    void AddLine(const char *file, int line, const char *msg, int hilit = 0);
    void FindBuffer(int line);
    void AssignBuffer(EBuffer *B, int line);
    void FindFileLines(EBuffer *B);
    void NotifyDelete(EModel *Deleting) override;

    int GetLine(char *Line, int max);
    virtual void ParseLine(const char *line, int len);
    void NotifyPipe(int APipeId);
    // Returns 0 if OK - calls ContinuePipe() several times to complete command for all files
    virtual int RunPipe(const char *Dir, const char *Command, const char *OnFiles);
    // Returns 0 if OK - continue with next files in queue
    virtual int ContinuePipe();
    // Reads ReturnCode, sets Running to 0, PipeId to -1
    virtual void ClosePipe();

    void DrawLine(PCell B, int Line, int Col, ChColor color, int Width);
    char *FormatLine(int Line) const override;
    void UpdateList();
    int Activate(int No);
    int CanActivate(int Line) const override;
    int IsHilited(int Line) const override;
    int IsMarked(int Line) const override;
    int Mark(int Line) override;
    int Unmark(int Line) override;

    int ExecCommand(int Command, ExState &State) override;
    void ShowLine(EView *V, int err);

    int GetContext() const override {
        return CONTEXT_CVSBASE;
    }
    EEventMap *GetEventMap() override;
    void GetName(char *AName, int MaxLen) const override;
    void GetInfo(char *AInfo, int MaxLen) const override;
    void GetPath(char *APath, int MaxLen) const override;
    void GetTitle(char *ATitle, int MaxLen, char *ASTitle, int SMaxLen) const override;
};

int AddCvsIgnoreRegexp(const char *);
void FreeCvsIgnoreRegexp();

#endif
