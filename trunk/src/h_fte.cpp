/*    h_fte.cpp
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

#ifdef CONFIG_HILIT_FTE

enum {
    hsFTE_Normal = 200,
    hsFTE_Comment,
    hsFTE_Keyword,
    hsFTE_String1,
    hsFTE_String2,
    hsFTE_CPP,
    hsFTE_Regexp,
    hsFTE_KeySpec
};

int Hilit_FTE(EBuffer *BF, int /*LN*/, PCell B, int Pos, int Width, ELine *Line, hlState &State, hsState *StateMap, int *ECol) {
    int j = 0;
    HILIT_VARS(BF->Mode->fColorize->Colors, Line);
    int len1 = len;
    char *last = p + len1 - 1;

    C = 0;
    NC = 0;

    for (i = 0; i < Line->Count;) {
        IF_TAB() else {
            switch(State) {
            default:
            case hsFTE_Normal:
                if (isalpha(*p) || *p == '_') {
                    j = 0;
                    while (((i + j) < Line->Count) &&
                           (isalnum(Line->Chars[i+j]) ||
                            (Line->Chars[i + j] == '_'))
                          ) j++;
                    if (BF->GetHilitWord(j, &Line->Chars[i], Color)) {
                        State = hsFTE_Keyword;
                    } else {
                        Color = CLR_Normal;
                        State = hsFTE_Normal;
                    }
                    if (StateMap)
                        memset(StateMap + i, State, j);
                    if (B)
                        MoveMem(B, C - Pos, Width, Line->Chars + i, HILIT_CLRD(), j);
                    i += j;
                    len -= j;
                    p += j;
                    C += j;
                    State = hsFTE_Normal;
                    continue;
                } else if (*p == '#') {
                    State = hsFTE_Comment;
                    Color = CLR_Comment;
                    goto hilit;
                } else if (*p == '%') {
                    State = hsFTE_CPP;
                    Color = CLR_CPreprocessor;
                    goto hilit;
                } else if (isdigit(*p)) {
                    Color = CLR_Number;
                    ColorNext();
                    while (len && (isdigit(*p) || *p == 'e' || *p == 'E' || *p == '.')) ColorNext();
                    if (len && (toupper(*p) == 'U')) ColorNext();
                    if (len && (toupper(*p) == 'L')) ColorNext();
                    continue;
                } else if (*p == '\'') {
                    State = hsFTE_String1;
                    Color = CLR_String;
                    goto hilit;
                } else if (*p == '[') {
                    State = hsFTE_KeySpec;
                    Color = CLR_Command;
                    goto hilit;
                } else if (*p == '"') {
                    State = hsFTE_String2;
                    Color = CLR_String;
                    goto hilit;
                } else if (*p == '/') {
                    State = hsFTE_Regexp;
                    Color = CLR_Regexp;
                    goto hilit;
                } else if (ispunct(*p)) {
                    Color = CLR_Punctuation;
                    ColorNext();
                    continue;
                }
                Color = CLR_Normal;
                ColorNext();
                continue;
            case hsFTE_Comment:
                Color = CLR_Comment;
                goto hilit;
            case hsFTE_CPP:
                Color = CLR_CPreprocessor;
                goto hilit;
            case hsFTE_String1:
                Color = CLR_String;
                if ((len >= 2) && (*p == '\\')) {
                hilit2:
                    ColorNext();
                hilit:
                    ColorNext();
                    continue;
                } else if (*p == '\'') {
                    ColorNext();
                    State = hsFTE_Normal;
                    continue;
                }
                goto hilit;
            case hsFTE_String2:
                Color = CLR_String;
                if ((len >= 2) && (*p == '\\')) {
                    goto hilit2;
                } else if (*p == '"') {
                    ColorNext();
                    State = hsFTE_Normal;
                    continue;
                }
                goto hilit;
            case hsFTE_KeySpec:
                Color = CLR_Command;
                if ((len >= 2) && (*p == '\\')) {
                    goto hilit2;
                } else if (*p == ']') {
                    ColorNext();
                    State = hsFTE_Normal;
                    continue;
                }
                goto hilit;
            case hsFTE_Regexp:
                Color = CLR_Regexp;
                if ((len >= 2) && (*p == '\\')) {
                    goto hilit2;
                } else if (*p == '/') {
                    ColorNext();
                    State = hsFTE_Normal;
                    continue;
                }
                goto hilit;
            }
        }
    }

    if (State == hsFTE_Comment)
        State = hsFTE_Normal;

    if ((len1 == 0) || (*last != '\\')) {
        switch(State) {
        case hsFTE_CPP:
        case hsFTE_String1:
        case hsFTE_String2:
        case hsFTE_KeySpec:
        case hsFTE_Regexp:
            State = hsFTE_Normal;
            break;
        }
    }
    *ECol = C;
    return 0;
}
#endif
