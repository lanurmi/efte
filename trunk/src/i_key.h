/*    i_key.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef EXKEY_H_
#define EXKEY_H_

class ExKey: public ExView {
public:
    char *Prompt;
    TKeyCode Key;
    char ch;

    ExKey(const char *APrompt);
    virtual ~ExKey();
    virtual void Activate(int gotfocus);

    virtual ExView* GetViewContext() {
        return Next;
    }
    virtual int BeginMacro();
    virtual void HandleEvent(TEvent &Event);
    virtual void UpdateView();
    virtual void RepaintView();
    virtual void UpdateStatus();
    virtual void RepaintStatus();
};

#endif
