/*    o_buflist.h
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef __BUFLIST_H
#define __BUFLIST_H

class BufferView: public EList {
public:
    char **BList;
    int BCount;

    BufferView(int createFlags, EModel **ARoot);
    virtual ~BufferView();
    virtual EEventMap *GetEventMap();
    virtual int GetContext();
    virtual void DrawLine(PCell B, int Line, int Col, ChColor color, int Width);
    virtual char* FormatLine(int Line);
    virtual void UpdateList();
    EModel *GetBufferById(int No);
    virtual int ExecCommand(int Command, ExState &State);
    virtual int Activate(int No);
    virtual void GetInfo(char *AInfo, int MaxLen);
    virtual void GetTitle(char *ATitle, int MaxLen, char *ASTitle, int SMaxLen);
};

#endif
