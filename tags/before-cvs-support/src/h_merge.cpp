/*    h_merge.cpp
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

#ifdef CONFIG_HILIT_MERGE

#define hsMERGE_Normal     0
#define hsMERGE_Modified   1
#define hsMERGE_Original   2
#define hsMERGE_New        3
#define hsMERGE_Control    4

int Hilit_MERGE(EBuffer *BF, int /*LN*/, PCell B, int Pos, int Width, ELine* Line, hlState& State, hsState *StateMap, int *ECol) {
    ChColor *Colors = BF->Mode->fColorize->Colors;
    HILIT_VARS(Colors[CLR_Normal], Line);
    hlState StateO = State;
    hlState StateN = State;

    if (Line->Count >= 7) {
        State = hsMERGE_Control;
        if      (memcmp(Line->Chars, "<<<<<<<", 7) == 0)
            StateN = hsMERGE_Modified;
        else if (memcmp(Line->Chars, "|||||||", 7) == 0)
            StateN = hsMERGE_Original;
        else if (memcmp(Line->Chars, "=======", 7) == 0)
            StateN = hsMERGE_New;
        else if (memcmp(Line->Chars, ">>>>>>>", 7) == 0)
            StateN = hsMERGE_Normal;
        else
            State = StateO;
    }

    for (i = 0; i < Line->Count; ) {
        IF_TAB() else {
            switch(State) {
            case hsMERGE_Control:  Color = Colors[CLR_Control]; break;
            case hsMERGE_Modified: Color = Colors[CLR_Changed]; break;
            case hsMERGE_Original: Color = Colors[CLR_Old];     break;
            case hsMERGE_New:      Color = Colors[CLR_New];     break;
            default:               Color = Colors[CLR_Normal];  break;

            }
            ColorNext();
            continue;
        }
    }
    State = StateN;
    *ECol = C;
    return 0;
}

#endif
