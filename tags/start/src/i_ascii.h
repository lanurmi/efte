/*    i_ascii.h
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef __EXASCII_H
#define __EXASCII_H

class ExASCII: public ExView {
public:
    int Pos, LPos;
    
    ExASCII();
    virtual ~ExASCII();
    virtual void Activate(int gotfocus);
    
    virtual ExView* GetViewContext() { return Next; }
    virtual int BeginMacro();
    virtual void HandleEvent(TEvent &Event);
    virtual void UpdateView();
    virtual void RepaintView();
    virtual void UpdateStatus();
    virtual void RepaintStatus();
};

#endif
