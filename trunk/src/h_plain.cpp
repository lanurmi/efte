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
    int PLine = FindPrevNonEmptyLine(B, Line-1);
    int OI = B->LineIndented(Line);
    int OC = B->CP.Col;

    if (B->Mode->indent_count) {
        int indent_offset = B->LineIndented(PLine), skip = 0;
        RxMatchRes b;

        for (int i=0; i < B->Mode->indent_count; i++) {
            if (skip) {
                fprintf(stderr, "%i was skipped\n", i);
                skip = 0;
                continue;
            }

            int look_line = Line;
            if (B->Mode->indents[i].look_line != 0)
                look_line = FindPrevNonEmptyLine(B, Line + B->Mode->indents[i].look_line);
            char *ll = B->RLine(look_line)->Chars;
            int llcount = B->RLine(look_line)->Count;

            fprintf(stderr, "Line=%i, PLine=%i, i=%i, LLine=%i\n",
                    Line, PLine, i, look_line);

            if (RxExecMatch(B->Mode->indents[i].regex, ll, llcount, ll, &b, RX_CASE)) {
                fprintf(stderr, "...Matched\n");
                switch (B->Mode->indents[i].flags) {
                case 1:
                    fprintf(stderr, "...Continuation\n");
                    B->RLine(Line)->IndentContinuation = i;
                    if (B->RLine(PLine)->IndentContinuation != i) {
                        fprintf(stderr, "...Line not indented yet\n");
                        indent_offset += B->Mode->indents[i].indent * 4;
                    }
                    break;

                case 2:
                    fprintf(stderr, "...Skip Next\n");
                    skip = 1;
                    break;

                default:
                    fprintf(stderr, "...Normal\n");
                    indent_offset += B->Mode->indents[i].indent * 4;
                }
            } else if (B->Mode->indents[i].flags == 1 &&
                       B->RLine(look_line)->IndentContinuation == i)
            {
                fprintf(stderr, "...Unindenting indent\n");
                indent_offset -= B->Mode->indents[i].indent * 4;
            }
        }

        B->DelText(Line, 0, OI, 0);
        B->IndentLine(Line, indent_offset);

        int newCol = OC - (OI - indent_offset);
        if (OI == 0 || B->RLine(Line)->Count == 0)
            newCol = indent_offset;
        B->SetNearPos(newCol, Line);
    } else {
        B->IndentLine(Line, B->LineIndented(PLine));
        // Fix cursor position
        if (PosCursor) {
            int I = B->LineIndented(Line);
            int X = B->CP.Col;

            X = X - OI + I;
            if (X < I) X = I;
            if (X < 0) X = 0;
            B->SetPos(X, Line);
        }
    }

    return 1;
}

