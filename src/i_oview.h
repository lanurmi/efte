/*    i_oview.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef IOVIEW_H_
#define IOVIEW_H_

class GxView;
class EBuffer;
class EWindow;

class ExView {
public:
    GxView *Win;
    ExView *Next;

    ExView();
    virtual ~ExView();

    virtual EEventMap *GetEventMap();
    virtual int ExecCommand(int Command, ExState &State);

    virtual void Activate(int gotfocus);
    virtual int GetContext() const;
    virtual ExView *GetViewContext() {
        return this;
    }
    virtual ExView *GetStatusContext() {
        return this;
    }
    virtual int BeginMacro();
    virtual void HandleEvent(TEvent &Event);
    virtual void UpdateView();
    virtual void UpdateStatus();
    virtual void RepaintView();
    virtual void RepaintStatus();
    virtual void Resize(int width, int height);
    virtual void EndExec(int NewResult);
    int IsActive() const;

    void Repaint() {
        RepaintStatus();
        RepaintView();
    }
    void Update() {
        UpdateStatus();
        UpdateView();
    }

    int ConPutBox(int X, int Y, int W, int H, PCell Cell);
    int ConScroll(int Way, int X, int Y, int W, int H, TAttr Fill, int Count);
    int ConQuerySize(int *X, int *Y);
    int ConSetCursorPos(int X, int Y);
    int ConShowCursor();
    int ConHideCursor();
    void ConSetInsertState(bool insert);

    virtual int IsModelView() const;
    virtual void WnSwitchBuffer(EModel *M);
};

#endif
