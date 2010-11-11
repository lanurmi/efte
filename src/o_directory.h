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

    EDirectory(int createFlags, EModel **ARoot, char *aPath);
    virtual ~EDirectory();

    virtual int GetContext();
    virtual int GetMatchForward(int start=0);
    virtual int GetMatchBackward(int start=0);
    virtual EEventMap *GetEventMap();
    virtual int ExecCommand(int Command, ExState &State);
    virtual void HandleEvent(TEvent &Event);

    virtual void DrawLine(PCell B, int Line, int Col, ChColor color, int Width);
    virtual char *FormatLine(int Line);
    virtual int IsHilited(int Line);

    virtual void RescanList();
//    virtual void UpdateList();
    virtual void FreeList();
    virtual int CanActivate(int Line);
    virtual int Activate(int No);

    virtual void GetName(char *AName, int MaxLen);
    virtual void GetPath(char *APath, int MaxLen);
    virtual void GetInfo(char *AInfo, int MaxLen);
    virtual void GetTitle(char *ATitle, int MaxLen, char *ASTitle, int SMaxLen);

    int isDir(int No);
    int FmChDir(const char* Name);
    int FmLoad(const char* Name, EView *View);
    int FmMvFile(const char* Name);
    int FmRmFile(const char* Name);
    int FmMkDir();
    int ChangeDir(ExState &State);
    int RescanDir();
};
#endif
