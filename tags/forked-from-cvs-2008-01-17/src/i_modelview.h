/*    i_modelview.h
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef __EXEDIT_H
#define __EXEDIT_H

class EView;

class ExModelView: public ExView {
public:
    EView *View;
    int MouseCaptured;
    int MouseMoved;

    ExModelView(EView *AView);
    virtual ~ExModelView();
    virtual void Activate(int gotfocus);
    
    virtual EEventMap *GetEventMap();
    virtual int ExecCommand(int Command, ExState &State);
    
    virtual int GetContext();
    virtual int BeginMacro();
    virtual void HandleEvent(TEvent &Event);
    virtual void UpdateView();
    virtual void RepaintView();
    virtual void UpdateStatus();
    virtual void RepaintStatus();
    virtual void Resize(int width, int height);
    virtual void WnSwitchBuffer(EModel *M);
    virtual int IsModelView() { return 1; }
};

#endif
