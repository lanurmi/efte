/*    h_diff.cpp
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

#ifdef CONFIG_HILIT_DIFF

int Hilit_DIFF(EBuffer *BF, int /*LN*/, PCell B, int Pos, int Width, ELine* Line, hlState& State, hsState *StateMap, int *ECol) {
    ChColor *Colors = BF->Mode->fColorize->Colors;
    HILIT_VARS(Colors[CLR_Normal], Line);
    
    if (Line->Count > 0) {
        switch (Line->Chars[0]) {
        case '>':
        case '+': Color = Colors[CLR_New]; break;
        case '<':
        case '-': Color = Colors[CLR_Old]; break;
        case '!': Color = Colors[CLR_Changed]; break;
        default:  Color = Colors[CLR_Normal]; break;
        }
    }

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
                    MoveMem(B, 0, Width, Line->Chars + Pos, Color, Width);
                if (StateMap)
                    memset(StateMap, State, Line->Count);
            } else {
                if (B)
                    MoveMem(B, 0, Width, Line->Chars + Pos, Color, Line->Count - Pos);
                if (StateMap)
                    memset(StateMap, State, Line->Count);
            }
        }
        C = Line->Count;
    }
    State = 0;
    *ECol = C;
    return 0;
}
#endif
