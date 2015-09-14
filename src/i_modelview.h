/*    i_modelview.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef EXEDIT_H_
#define EXEDIT_H_

class EView;

class ExModelView: public ExView {
public:
    EView *View;
    int MouseCaptured;
    int MouseMoved;

    ExModelView(EView *AView);
    virtual ~ExModelView();
    void Activate(int gotfocus) override;

    EEventMap *GetEventMap() override;
    int ExecCommand(int Command, ExState &State) override;

    int GetContext() const override;
    int BeginMacro();
    void HandleEvent(TEvent &Event) override;
    void UpdateView() override;
    void RepaintView() override;
    void UpdateStatus() override;
    void RepaintStatus() override;
    void Resize(int width, int height) override;
    void WnSwitchBuffer(EModel *M) override;
    int IsModelView() const override {
        return 1;
    }
};

#endif
