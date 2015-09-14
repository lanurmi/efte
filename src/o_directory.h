/*    o_directory.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef ODIRECTORY_H_
#define ODIRECTORY_H_

class EDirectory: public EList {
public:
    char *Path;
    FileInfo **Files;
    int FCount;
    int SearchLen;
    char SearchName[MAXISEARCH];
    int SearchPos[MAXISEARCH];

    EDirectory(int createFlags, EModel **ARoot, const char *aPath);
    virtual ~EDirectory();

    int GetContext() const override;
    virtual int GetMatchForward(int start=0) const;
    virtual int GetMatchBackward(int start=0) const;
    EEventMap *GetEventMap() override;
    int ExecCommand(int Command, ExState &State) override;
    void HandleEvent(TEvent &Event) override;

    void DrawLine(PCell B, int Line, int Col, ChColor color, int Width) override;
    char *FormatLine(int Line) const override;
    int IsHilited(int Line) const override;

    void RescanList() override;
//    void UpdateList() override;
    void FreeList() override;
    int CanActivate(int Line) const override;
    int Activate(int No) override;

    void GetName(char *AName, int MaxLen) const override;
    void GetPath(char *APath, int MaxLen) const override;
    void GetInfo(char *AInfo, int MaxLen) const override;
    void GetTitle(char *ATitle, int MaxLen, char *ASTitle, int SMaxLen) const override;

    int IsDir(int No) const;
    int FmChDir(const char* Name);
    int FmLoad(const char* Name, EView *View);
    int FmMvFile(const char* Name);
    int FmRmFile(const char* Name);
    int FmMkDir();
    int ChangeDir(ExState &State);
    int RescanDir();
};
#endif
