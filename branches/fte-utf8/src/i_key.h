/*    i_key.h
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef __EXKEY_H
#define __EXKEY_H

class ExKey: public ExView {
public:
    char *Prompt;
    TKeyCode Key;
    char ch;
    
    ExKey(const char *APrompt);
    virtual ~ExKey();
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
