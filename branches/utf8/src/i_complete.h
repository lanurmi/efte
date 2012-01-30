/*    i_ascii.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef EXCOMPLETE_H_
#define EXCOMPLETE_H_

#include <ctype.h>

// maximum words which will be presented to the user
#define MAXCOMPLETEWORDS 300

class ExComplete: public ExView {
    EPoint Orig;
    EBuffer *Buffer;
    int WordsLast;
    char **Words;
    char *WordBegin;
    char *WordContinue;
    int WordPos;
    size_t WordFixed;
    size_t WordFixedCount;

    int RefreshComplete();
    inline int CheckASCII(int c) {
        return ((c < 256)
                && (isalnum(c) || (c == '_') || (c == '.'))) ? 1 : 0;
    }
    void FixedUpdate(int add);

public:

    ExComplete(EBuffer *B);
    virtual ~ExComplete();
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

    virtual bool IsSimpleCase();
    virtual int DoCompleteWord();
};
#endif
