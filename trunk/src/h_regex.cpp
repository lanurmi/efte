/*    h_user.cpp
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"
#include "e_regex.h"

int FindPrevNonEmptyLine(EBuffer *B, int Line) {
    while (Line > 0) {
        if (B->RLine(Line)->Count > 0)
            return Line;
        Line--;
    }

    return 0;
}

int Indent_REGEX(EBuffer *B, int Line, int PosCursor) {
    int PLine = FindPrevNonEmptyLine(B, Line-1);
    int OI = B->LineIndented(Line);
    int OC = B->CP.Col;

    if (Line == 0) {
        B->IndentLine(Line, 0);
    } else if (B->Mode->indent_count) {
        int indent_offset = B->LineIndented(PLine), skip = 0;
        RxMatchRes b;

        for (int i=0; i < B->Mode->indent_count; i++) {
            if (skip) {
                skip = 0;
                continue;
            }

            int look_line = Line;
            if (B->Mode->indents[i].look_line != 0)
                look_line = FindPrevNonEmptyLine(B, Line + B->Mode->indents[i].look_line);
            char *ll = B->RLine(look_line)->Chars;
            int llcount = B->RLine(look_line)->Count;

            if (RxExec(B->Mode->indents[i].regex, ll, llcount, ll, &b, RX_CASE)) {
                switch (B->Mode->indents[i].flags) {
                case 1:
                    B->RLine(Line)->IndentContinuation = i;
                    if (B->RLine(PLine)->IndentContinuation != i) {
                        indent_offset += B->Mode->indents[i].indent * 4;
                    }
                    break;

                case 2:
                    skip = 1;
                    break;

                default:
                    indent_offset += B->Mode->indents[i].indent * 4;
                }
            } else if (B->Mode->indents[i].flags == 1 &&
                       B->RLine(look_line)->IndentContinuation == i)
            {
                indent_offset -= B->Mode->indents[i].indent * 4;
            }
        }

        B->DelText(Line, 0, OI, 0);
        B->IndentLine(Line, indent_offset);

        int newCol = OC - (OI - indent_offset);
        if ((OI == 0 || B->RLine(Line)->Count == 0) && indent_offset != 0)
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
