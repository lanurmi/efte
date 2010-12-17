/*    h_cont.cpp
 *
 *    Copyright (c) 2008, 2010 eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

int Indent_Continue(EBuffer *B, int Line, int /*PosCursor*/) {
    // Find the previous non-blank line
    for (int i = Line - 1; i >= 0; i--) {
        if (B->RLine(i)->Count > 0) {
            int I = B->LineIndented(i);
            B->SetPosR(I, Line);
            return 1;
        }
    }

    B->SetPosR(0, Line);

    return 1;
}
