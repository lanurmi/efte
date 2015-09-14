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
    void Activate(int gotfocus) override;

    ExView *GetViewContext() override {
        return Next;
    }
    int BeginMacro() override;
    void HandleEvent(TEvent &Event) override;
    void UpdateView() override;
    void RepaintView() override;
    void UpdateStatus() override;
    void RepaintStatus() override;
};

#endif
