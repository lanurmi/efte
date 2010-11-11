/*    i_view.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef I_VIEW_H_
#define I_VIEW_H_

class EView;

class GxView: public GView {
public:
    ExView *Top;
    ExView *Bottom;
    int MouseCaptured;

    GxView(GFrame *Parent);
    virtual ~GxView();

    void PushView(ExView *view);
    ExView *PopView();
    void NewView(ExView *view);

    EEventMap *GetEventMap();
    int ExecCommand(int Command, ExState &State);

    virtual int GetContext();
    virtual ExView* GetStatusContext() {
        if (Top) return Top->GetStatusContext();
        else return 0;
    }
    virtual ExView* GetViewContext() {
        if (Top) return Top->GetViewContext();
        else return 0;
    }
    virtual int BeginMacro();
    virtual void HandleEvent(TEvent &Event);
    virtual void Update();
    virtual void Repaint();
    virtual void Activate(int gotfocus);
    virtual void Resize(int width, int height);

    void UpdateTitle(char *Title, char *STitle);

    int ReadStr(const char *Prompt, unsigned int BufLen, char *Str, Completer Comp, int Select, int HistId);
    int Choice(unsigned long Flags, const char *Title, int NSel, ... /* choices, format, args */);
    TKeyCode GetChar(const char *Prompt);
    int IncrementalSearch(EView *V);
    int PickASCII();
    int ICompleteWord(EView *View);

    int GetStr(const char *Prompt, unsigned int BufLen, char *Str, int HistId);
    int GetFile(const char *Prompt, unsigned int BufLen, char *Str, int HistId, int Flags);

    int IsModelView() {
        return Top ? Top->IsModelView() : 0;
    }
};

#endif
