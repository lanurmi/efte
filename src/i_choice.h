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
    void Activate(int gotfocus) override;

    ExView* GetViewContext()  override {
        return Next;
    }
    int BeginMacro() override;
    int FindChoiceByPoint(int x, int y);
    void HandleEvent(TEvent &Event) override;
    void UpdateView() override;
    void RepaintView() override;
    void UpdateStatus() override;
    void RepaintStatus() override;
};

#endif
