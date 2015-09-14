/*    i_ascii.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef EXASCII_H_
#define EXASCII_H_

class ExASCII: public ExView {
public:
    int Pos, LPos;

    ExASCII();
    virtual ~ExASCII();
    void Activate(int gotfocus) override;

    ExView* GetViewContext() override {
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
