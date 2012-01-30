/*    o_buflist.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef BUFLIST_H_
#define BUFLIST_H_

class BufferView: public EList {
public:
    char **BList;
    int BCount;
    int SearchLen;
    char SearchString[MAXISEARCH];
    int SearchPos[MAXISEARCH];

    BufferView(int createFlags, EModel **ARoot);
    virtual ~BufferView();
    virtual EEventMap *GetEventMap();
    virtual int GetContext();
    virtual void DrawLine(PCell B, int Line, int Col, ChColor color, int Width);
    virtual char* FormatLine(int Line);
    virtual void UpdateList();
    EModel *GetBufferById(int No);
    virtual int ExecCommand(int Command, ExState &State);
    virtual void HandleEvent(TEvent &Event);
    int getMatchingLine(int start, int direction);
    virtual int Activate(int No);
    void CancelSearch();
    virtual void GetInfo(char *AInfo, int MaxLen);
    virtual void GetTitle(char *ATitle, int MaxLen, char *ASTitle, int SMaxLen);
};

#endif
