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
    EEventMap *GetEventMap() override;
    int ExecCommand(int Command, ExState &State) override;
    void HandleEvent(TEvent &Event) override;
	virtual int GetMatchingLine(int start, int direction) const override;
    void DrawLine(PCell B, int Line, int Col, ChColor color, int Width) override;
    char* FormatLine(int Line) const;
    int Activate(int No) override;
    void CancelSearch();
    void RescanList() override;
    void UpdateList() override;
    int GetContext() const override;
    void GetName(char *AName, int MaxLen) const override;
    void GetInfo(char *AInfo, int MaxLen) const override;
    void GetTitle(char *ATitle, int MaxLen, char *ASTitle, int SMaxLen) const override;
};

#endif
