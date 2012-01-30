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

    void SetState(IState state);
};

#endif
