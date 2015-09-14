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

    virtual bool IsSimpleCase();
    virtual int DoCompleteWord();
};
#endif
