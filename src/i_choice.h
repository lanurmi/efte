/*    i_choice.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef EXCHOICE_H_
#define EXCHOICE_H_

class ExChoice: public ExView {
public:
    char *Title;
    char Prompt[160];
    int NOpt;
    char *SOpt[10];
    int Cur;
    int lTitle;
    int lChoice;
    int MouseCaptured;

    ExChoice(const char *ATitle, int NSel, va_list ap /* choices, format, args */);
    virtual ~ExChoice();
    virtual void Activate(int gotfocus);

    virtual ExView* GetViewContext() {
        return Next;
    }
    virtual int BeginMacro();
    int FindChoiceByPoint(int x, int y);
    virtual void HandleEvent(TEvent &Event);
    virtual void UpdateView();
    virtual void RepaintView();
    virtual void UpdateStatus();
    virtual void RepaintStatus();
};

#endif
