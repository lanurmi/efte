/*    h_plain.cpp
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"
#include "e_regex.h"

#define hsPLAIN_Normal 0

int Hilit_Plain(EBuffer *BF, int /*LN*/, PCell B, int Pos, int Width, ELine* Line, hlState& State, hsState *StateMap, int *ECol) {
    HILIT_VARS(BF->Mode->fColorize->Colors, Line);

    int j = 0;

    if (BF->Mode->fColorize->Keywords.TotalCount > 0 ||
            BF->WordCount > 0) { /* words have to be hilited, go slow */
        for (i = 0; i < Line->Count;) {
            IF_TAB() else {
                if (isalpha(*p) || (*p == '_')) {
                    j = 0;
                    while (((i + j) < Line->Count) &&
                            (isalnum(Line->Chars[i+j]) ||
                             (Line->Chars[i + j] == '_'))
                          ) j++;
                    if (BF->GetHilitWord(j, Line->Chars + i, Color, 1)) ;
                    else {
                        Color = CLR_Normal;
                        State = hsPLAIN_Normal;
                    }
                    if (StateMap)
                        memset(StateMap + i, State, j);
                    if (B)
                        MoveMem(B, C - Pos, Width, Line->Chars + i, HILIT_CLRD(), j);
                    i += j;
                    len -= j;
                    p += j;
                    C += j;
                    State = hsPLAIN_Normal;
                    Color = CLR_Normal;
                    continue;
                }
                ColorNext();
                continue;
            }
        }
    } else {
        if (ExpandTabs) { /* use slow mode */
            for (i = 0; i < Line->Count;) {
                IF_TAB() else {
                    ColorNext();
                }
            }
        } else { /* fast mode */
            if (Pos < Line->Count) {
                if (Pos + Width < Line->Count) {
                    if (B)
                        MoveMem(B, 0, Width, Line->Chars + Pos, HILIT_CLRD(), Width);
                    if (StateMap)
                        memset(StateMap, State, Line->Count);
                } else {
                    if (B)
                        MoveMem(B, 0, Width, Line->Chars + Pos, HILIT_CLRD(), Line->Count - Pos);
                    if (StateMap)
                        memset(StateMap, State, Line->Count);
                }
            }
            C = Line->Count;
        }
    }
    *ECol = C;
    State = 0;
    return 0;
}

int FindPrevNonEmptyLine(EBuffer *B, int Line) {
    while (Line > 0) {
        if (B->RLine(Line)->Count > 0)
            return Line;
        Line--;
    }

    return 0;
}

int Indent_Plain(EBuffer *B, int Line, int PosCursor) {
    int PLine = FindPrevNonEmptyLine(B, Line);
    int OI = B->LineIndented(Line);

    if (B->Mode->indent_count) {
        RxMatchRes b;
        int indent, handled = 0;

        for (int i=0; i < B->Mode->indent_count; i++) {
            int affect_line = Line + B->Mode->indents[i].affect_line;
            char *ll = B->RLine(PLine)->Chars;
            int llcount = B->RLine(PLine)->Count;

            if (RxExecMatch(B->Mode->indents[i].regex, ll, llcount, ll, &b, 0)) {
                indent = B->LineIndented(PLine) + B->Mode->indents[i].indent * 4;
                B->IndentLine(affect_line, indent);

                if (affect_line == Line)
                    handled = 1;
            }
        }

        if (handled == 0)
            B->IndentLine(Line, B->LineIndented(PLine));
    } else {
        B->IndentLine(Line, B->LineIndented(PLine));
    }

    // Fix cursor position
    if (PosCursor) {
        int I = B->LineIndented(Line);
        int X = B->CP.Col;

        X = X - OI + I;
        if (X < I) X = I;
        if (X < 0) X = 0;
        B->SetPos(X, Line);
    }

    return 1;
}

