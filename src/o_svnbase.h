/*
 * o_svnbase.h
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 * S.Pinigin copy o_cvsbase.h and replace cvs/Cvs/CVS to svn/Svn/SVN.
 *
 * Base class for all other SVN-related classes. This is similar to EMessages
 * - starts SVN and shows its messages in list view.
 */

#ifndef SVNBASE_H_
#define SVNBASE_H_

typedef struct {
    char *File; // Relative to view's directory
    int Line;
    char *Msg;
    EBuffer *Buf;
    char Status;
    // bit 0 - hilited
    // bit 1 - marked
    // bit 2 - markable
} SvnLine;

class ESvnBase: public EList {
public:
    char *Command;
    char *Directory;
    char *OnFiles;
    char *OnFilesPos;

    int LineCount;
    SvnLine **Lines;
    int Running;

    int BufLen;
    int BufPos;
    int PipeId;
    int ReturnCode;
    char MsgBuf[4096];

    ESvnBase(int createFlags, EModel **ARoot, const char *ATitle);
    ~ESvnBase();

    void FreeLines();
    void AddLine(const char* file, int line, const char* msg, int hilit = 0);
    void FindBuffer(int line);
    void AssignBuffer(EBuffer *B, int line);
    void FindFileLines(EBuffer *B);
    virtual void NotifyDelete(EModel *Deleting);

    int GetLine(char *Line, int max);
    virtual void ParseLine(const char *line, int len);
    void NotifyPipe(int APipeId) override;
    // Returns 0 if OK - calls ContinuePipe() several times to complete command for all files
    virtual int RunPipe(const char *Dir, const char *Command, const char *OnFiles);
    // Returns 0 if OK - continue with next files in queue
    virtual int ContinuePipe();
    // Reads ReturnCode, sets Running to 0, PipeId to -1
    virtual void ClosePipe();

    void DrawLine(PCell B, int Line, int Col, ChColor color, int Width) override;
    char *FormatLine(int Line) const override;
    void UpdateList() override;
    int Activate(int No) override;
    int CanActivate(int Line) const override;
    int IsHilited(int Line) const override;
    int IsMarked(int Line) const override;
    int Mark(int Line) override;
    int Unmark(int Line) override;

    int ExecCommand(int Command, ExState &State) override;
    void ShowLine(EView *V, int err);

    int GetContext() const override {
        return CONTEXT_SVNBASE;
    }
    EEventMap *GetEventMap() override;
    void GetName(char *AName, int MaxLen) const override;
    void GetInfo(char *AInfo, int MaxLen) const override;
    void GetPath(char *APath, int MaxLen) const override;
    void GetTitle(char *ATitle, int MaxLen, char *ASTitle, int SMaxLen) const override;
};

int AddSvnIgnoreRegexp(const char *);
void FreeSvnIgnoreRegexp();

#endif
