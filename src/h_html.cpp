/*    h_html.cpp
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

#ifdef CONFIG_HILIT_HTML

#define hsHTML_Normal  0
#define hsHTML_Command 1
#define hsHTML_String1 2
#define hsHTML_String2 3
#define hsHTML_Char    4
#define hsHTML_Slashed 5
#define hsHTML_Comment 6

int Hilit_HTML(EBuffer *BF, int /*LN*/, PCell B, int Pos, int Width, ELine *Line, hlState &State, hsState *StateMap, int *ECol) {
    ChColor *Colors = BF->Mode->fColorize->Colors;
    HILIT_VARS(Colors[CLR_Normal], Line);
    int j;

    for (i = 0; i < Line->Count;) {
        IF_TAB() else {
            switch (State) {
            case hsHTML_Normal:
                Color = Colors[CLR_Normal];
                if (*p == '<') {
                    State = hsHTML_Command;
                    Color = Colors[CLR_Command];
                    ColorNext();
                    if ((len > 0) && (*p == '/')) ColorNext();
                    continue;
                } else if (*p == '&') {
                    State = hsHTML_Char;
                    Color = Colors[CLR_Symbol];
                }
                goto hilit;
            case hsHTML_Slashed:
                Color = Colors[CLR_Tag];
                if (*p == '/') {
                    Color = Colors[CLR_Command];
                    ColorNext();
                    State = hsHTML_Normal;
                    continue;
                }
                goto hilit;
            case hsHTML_Command:
                Color = Colors[CLR_Command];
                if (isalpha(*p) || *p == '_') {
                    j = 0;
                    while (((i + j) < Line->Count) &&
                           (isalnum(Line->Chars[i+j]) ||
                            (Line->Chars[i + j] == '_'))
                          ) j++;
                    if (BF->GetHilitWord(j, &Line->Chars[i], Color, 1)) {
                    }
                    if (StateMap)
                        memset(StateMap + i, State, j);
                    if (B)
                        MoveMem(B, C - Pos, Width, Line->Chars + i, Color, j);
                    i += j;
                    len -= j;
                    p += j;
                    C += j;
                    Color = Colors[CLR_Command];
                    continue;
                } else if (*p == '-' && len > 1 && p[1] == '-') {
                    State = hsHTML_Comment;
                    Color = Colors[CLR_Comment];
                    ColorNext();
                    goto hilit;
                } else if (*p == '"') {
                    State = hsHTML_String2;
                    Color = Colors[CLR_String];
                    goto hilit;
                } else if (*p == '\'') {
                    State = hsHTML_String1;
                    Color = Colors[CLR_String];
                    goto hilit;
                } else if (*p == '>') {
                    ColorNext();
                    State = hsHTML_Normal;
                    continue;
                } else if (*p == '/') {
                    ColorNext();
                    State = hsHTML_Slashed;
                    continue;
                }
                goto hilit;
            case hsHTML_String2:
                Color = Colors[CLR_String];
                if (*p == '"') {
                    ColorNext();
                    State = hsHTML_Command;
                    continue;
                }
                goto hilit;
            case hsHTML_String1:
                Color = Colors[CLR_String];
                if (*p == '\'') {
                    ColorNext();
                    State = hsHTML_Command;
                    continue;
                }
                goto hilit;
            case hsHTML_Char:
                Color = Colors[CLR_Symbol];
                if (*p == ';' || *p == ' ' || *p == '<') {
                    ColorNext();
                    State = hsHTML_Normal;
                    continue;
                }
                goto hilit;
            case hsHTML_Comment:
                Color = Colors[CLR_Comment];
                if (*p == '-' && len > 1 && p[1] == '-') {
                    ColorNext();
                    ColorNext();
                    State = hsHTML_Command;
                    continue;
                }
                goto hilit;
            default:
                State = hsHTML_Normal;
                Color = Colors[CLR_Normal];
            hilit:
                ColorNext();
                continue;
            }
        }
    }
    if (State == hsHTML_Char)
        State = hsHTML_Normal;
    *ECol = C;
    return 0;
}

#endif
