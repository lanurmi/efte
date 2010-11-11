/*    i_input.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef EXINPUT_H_
#define EXINPUT_H_

typedef int (*Completer)(const char *Name, char *Completed, int Num);

class ExInput: public ExView {
public:
    char *Prompt;
    char *Line;
    char *MatchStr;
    char *CurStr;
    unsigned int Pos;
    unsigned int LPos;
    unsigned int MaxLen;
    Completer Comp;
    int TabCount;
    int HistId;
    int CurItem;
    unsigned int SelStart;
    unsigned int SelEnd;

    ExInput(const char *APrompt, char *ALine, unsigned int AMaxLen, Completer AComp, int Select, int AHistId);
    virtual ~ExInput();
    virtual void Activate(int gotfocus);

    virtual ExView *GetViewContext() {
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
