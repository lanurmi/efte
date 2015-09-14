/*    o_list.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef OLIST_H_
#define OLIST_H_

class EList;

class EListPort: public EViewPort {
public:
    EList *List;
    int Row, TopRow, LeftCol;
    int OldRow, OldTopRow, OldLeftCol, OldCount;
    EListPort(EList *L, EView *V);
    virtual ~EListPort();

    void StorePos();
    void GetPos();

    void HandleEvent(TEvent &Event) override;
    void HandleMouse(TEvent &Event);

    void PaintView(int PaintAll);

    void UpdateView() override;
    void RepaintView() override;
    void UpdateStatus() override;
    void RepaintStatus() override;
};

class EList: public EModel {
public:
    char *Title;
    int Row, LeftCol, TopRow, Count;
    int MouseCaptured;
    int MouseMoved;
    int NeedsUpdate, NeedsRedraw;

    EList(int createFlags, EModel **ARoot, const char *aTitle);
    virtual ~EList();

    virtual EViewPort *CreateViewPort(EView *V);
    EListPort *GetViewVPort(EView *V);
    EListPort *GetVPort();

    void SetTitle(const char *ATitle);

    int ExecCommand(int Command, ExState &State) override;
    EEventMap *GetEventMap() override;
    int GetContext() const override;
    int BeginMacro() override;
    void HandleEvent(TEvent &Event) override;


    virtual void DrawLine(PCell B, int Line, int Col, ChColor color, int Width);
    virtual char *FormatLine(int Line) const;
    virtual int IsHilited(int Line) const;
    virtual int IsMarked(int Line) const;
    virtual int Mark(int Line);
    virtual int Unmark(int Line);

    int SetPos(int ARow, int ACol);
    void FixPos();
    virtual int GetRowLength(int ARow) const {
        return 0;
    };

    virtual void RescanList();
    virtual void UpdateList();
    virtual void FreeList();
    virtual int CanActivate(int Line) const;
    virtual int Activate(int No);

    int MoveLeft();
    int MoveRight();
    int MoveUp();
    int MoveDown();
    int MoveLineStart();
    int MoveLineEnd();
    int MovePageUp();
    int MovePageDown();
    int ScrollLeft(int Cols);
    int ScrollRight(int Cols);
    int ScrollUp(int Rows);
    int ScrollDown(int Rows);
    int MovePageStart();
    int MovePageEnd();
    int MoveFileStart();
    int MoveFileEnd();
    int Activate();
    int Mark();
    int Unmark();
    int ToggleMark();
    int MarkAll();
    int UnmarkAll();
    int ToggleMarkAll();

    int UpdateRows(int minim, int maxim);
};

#endif
