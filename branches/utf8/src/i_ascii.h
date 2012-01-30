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
