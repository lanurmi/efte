/*    o_routine.h
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef __ROUTINE_H__
#define __ROUTINE_H__

#ifdef CONFIG_OBJ_ROUTINE

class RoutineView: public EList {
public:
    EBuffer *Buffer;
    
    RoutineView(int createFlags, EModel **ARoot, EBuffer *AB);
    virtual ~RoutineView();
    virtual EEventMap *GetEventMap();
    virtual int ExecCommand(int Command, ExState &State);
    virtual void DrawLine(PCell B, int Line, int Col, ChColor color, int Width);
    virtual char* FormatLine(int Line);
    virtual int Activate(int No);
    virtual void RescanList();
    void UpdateList();
    virtual int GetContext();
    virtual void GetName(char *AName, int MaxLen);
    virtual void GetInfo(char *AInfo, int MaxLen);
    virtual void GetTitle(char *ATitle, int MaxLen, char *ASTitle, int SMaxLen);
};
#endif

#endif
