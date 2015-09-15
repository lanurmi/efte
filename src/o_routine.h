/*    o_routine.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef ROUTINE_H_
#define ROUTINE_H_

class RoutineView: public EList {
public:
    EBuffer *Buffer;

    int RCount;
    int SearchLen;
    char SearchString[MAXISEARCH];
    int SearchPos[MAXISEARCH];

    RoutineView(int createFlags, EModel **ARoot, EBuffer *AB);
    virtual ~RoutineView();
    virtual EEventMap *GetEventMap();
    virtual int ExecCommand(int Command, ExState &State);
    virtual void HandleEvent(TEvent &Event);
	virtual int GetMatchingLine(int start, int direction) const;
    virtual void DrawLine(PCell B, int Line, int Col, ChColor color, int Width);
    virtual char* FormatLine(int Line) const;
    virtual int Activate(int No);
    void CancelSearch();
    virtual void RescanList();
    void UpdateList();
    virtual int GetContext() const;
    virtual void GetName(char *AName, int MaxLen) const;
    virtual void GetInfo(char *AInfo, int MaxLen) const;
    virtual void GetTitle(char *ATitle, int MaxLen, char *ASTitle, int SMaxLen) const;
};

#endif
