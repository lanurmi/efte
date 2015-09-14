/*    i_search.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef ISEARCH_H_
#define ISEARCH_H_

#define MAXISEARCH 256

class ExISearch: public ExView {
public:
    typedef enum { IOk, INoMatch, INoPrev, INoNext } IState;

    char ISearchStr[MAXISEARCH + 1];
    EPoint Orig;
    EPoint stack[MAXISEARCH];
    int len;
    int stacklen;
    EBuffer *Buffer;
    IState state;
    int Direction;

    ExISearch(EBuffer *B);
    virtual ~ExISearch();
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

    void SetState(IState state);
};

#endif
