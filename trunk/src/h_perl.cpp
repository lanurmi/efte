/*    h_perl.cpp
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 * Perl Mode
 *
 * TODO:
 *    here documents, formats
 *    OS/2 EXTPROC...,
 *    UNIX #! starts hilit ?
 *    POD highlighting (need two keyword sets).
 *    some tab handling (in & foo, etc -- if allowed)/
 */

#include "fte.h"

#ifdef CONFIG_HILIT_PERL

#define X_BIT     0x80     /* set if last was number, var, */
#define X_MASK    0x7F
#define X_NOT(state) (!((state) & X_BIT))

#define kwd(x) (isalnum(x) || (x) == '_')

#define IS_OBRACE(x) \
    ((x) == '(' || (x) == '{' || (x) == '<' || (x) == '[')

#define NUM_BRACE(x) \
    ( \
    (x) == '(' ? 0U : \
    (x) == '{' ? 1U : \
    (x) == '<' ? 2U : \
    (x) == '[' ? 3U : 0U \
    )

#define GET_BRACE(x) \
    ( \
    (x) == 0 ? '(' : \
    (x) == 1 ? '{' : \
    (x) == 2 ? '<' : \
    (x) == 3 ? '[' : 0 \
    )

#define IS_MBRACE(y,x) \
    ( \
    ((y) == '(' && (x) == ')') || \
    ((y) == '{' && (x) == '}') ||\
    ((y) == '<' && (x) == '>') ||\
    ((y) == '[' && (x) == ']') \
    )

#define QCHAR(state) ((char)(((state) >> 8) & 0xFF))
#define QSET(state, ch) ((unsigned short)((unsigned short)(state) | (((unsigned short)(ch)) << 8)))

#define hsPerl_Punct        0
#define hsPerl_Comment      1
#define hsPerl_Normal      30
#define hsPerl_Keyword      4
#define hsPerl_String1     10
#define hsPerl_String2     11
#define hsPerl_StringBk    22
#define hsPerl_Variable    23
#define hsPerl_Number      24
#define hsPerl_Function    25
#define hsPerl_RegexpM     26
#define hsPerl_RegexpS1    28
#define hsPerl_RegexpS2    29
#define hsPerl_Docs        31
#define hsPerl_Data        32
#define hsPerl_RegexpS3    33

#define hsPerl_Quote1Op    35
#define hsPerl_Quote1      36
#define hsPerl_Quote1M     37

#define hsPerl_Regexp1Op   38
#define hsPerl_Regexp1     39
#define hsPerl_Regexp1M    40

#define hsPerl_Regexp2Op   41
#define hsPerl_Regexp2     42
#define hsPerl_Regexp2M    43

#define hsPerl_HereDoc     44 // hack (eod not detected properly)

#define opQ  1
#define opQQ 2
#define opQW 3
#define opQX 4
#define opM  5
#define opS  6
#define opTR 7

int Hilit_PERL(EBuffer *BF, int /*LN*/, PCell B, int Pos, int Width, ELine *Line, hlState &State, hsState *StateMap, int *ECol) {
    ChColor *Colors = BF->Mode->fColorize->Colors;
    int j;
    HILIT_VARS(Colors[CLR_Normal], Line);
    int firstnw = 0;
    int op;
    int setHereDoc = 0;
#define MAXSEOF 100
    static char hereDocKey[MAXSEOF];

    C = 0;
    NC = 0;
    int isEOHereDoc = 0;
    for(i = 0; i < Line->Count;) {
        if (*p != ' ' && *p != 9) firstnw++;
        if ((State & X_MASK) == hsPerl_HereDoc && 0 == i)
        {
            isEOHereDoc = strlen(hereDocKey) == (size_t)len &&
                strncmp(hereDocKey, Line->Chars, len) == 0;
        }
        IF_TAB() else {
            //         printf("State = %d pos = %d", State, i); fflush(stdout);
            switch (State & X_MASK) {
            default:
            case hsPerl_Normal:
                if (i == 0 && X_NOT(State) && len == 7 &&
                    p[0] == '_' &&
                    p[1] == '_' &&
                    p[2] == 'E' &&
                    p[3] == 'N' &&
                    p[4] == 'D' &&
                    p[5] == '_' &&
                    p[6] == '_')
                {
                    State = hsPerl_Data;
                    Color = Colors[CLR_Comment];
                hilit5:
                    ColorNext();
                //hilit4:
                    ColorNext();
                //hilit3:
                    ColorNext();
                hilit2:
                    ColorNext();
                hilit:
                    ColorNext();
                    continue;
                } else if (i == 0 && X_NOT(State) && (*p == '=') && len > 4 &&
                    p[1] == 'h' && p[2] == 'e' && p[3] == 'a' && p[4] == 'd')
                {
                    State = hsPerl_Docs;
                    Color = Colors[CLR_Comment];
                    goto hilit5;
                } else if (isalpha(*p) || *p == '_') {
                    op = -1;
                    
                    j = 0;
                    while (((i + j) < Line->Count) &&
                           (isalnum(Line->Chars[i+j]) ||
                            (Line->Chars[i + j] == '_' || Line->Chars[i + j] == '\''))
                           ) j++;
                    if (BF->GetHilitWord(j, &Line->Chars[i], Color)) {
                        //Color = hcPERL_Keyword;
                        State = hsPerl_Keyword;
                    } else {
                        int x;
                        x = i + j;
                        while ((x < Line->Count) && 
                               ((Line->Chars[x] == ' ') || (Line->Chars[x] == 9))) x++;
                        if ((x < Line->Count) && (Line->Chars[x] == '(')) {
                            Color = Colors[CLR_Function];
                        } else {
                            Color = Colors[CLR_Normal];
                        }
                        State = hsPerl_Normal;
                    }
                    if (j == 1) {
                        if (*p == 'q') op = opQ;
                        else if (*p == 's' || *p == 'y') op = opS;
                        else if (*p == 'm') op = opM;
                    } else if (j == 2) {
                        if (*p == 'q') {
                            if (*p == 'q') op = opQQ;
                            else if (*p == 'w') op = opQW;
                            else if (*p == 'x') op = opQX;
                        } else if (*p == 't' && p[1] == 'r') op = opTR;
                    }
                    if (StateMap)
                        memset(StateMap + i, State, j);
                    if (B) 
                        MoveMem(B, C - Pos, Width, Line->Chars + i, Color, j);
                    i += j;
                    len -= j;
                    p += j;
                    C += j;
                    
                    switch (op) {
                    case opQ:
                        State = hsPerl_Quote1Op;   // q{} operator
                        Color = Colors[CLR_Punctuation];
                        continue;
                        
                    case opQQ:
                    case opQW:
                    case opQX:
                        State = hsPerl_Quote1Op;   // qq{} qx{} qw{} operators
                        Color = Colors[CLR_Punctuation];
                        continue;
                        
                    case opM:
                        State = hsPerl_Regexp1Op;   // m{} operator
                        Color = Colors[CLR_Punctuation];
                        continue;
                        
                    case opTR:
                        State = hsPerl_Regexp2Op;   // tr{} operators
                        Color = Colors[CLR_RegexpDelim];
                        continue;
                        
                    case opS:
                        State = hsPerl_Regexp2Op;   // s{}{} operator
                        Color = Colors[CLR_Punctuation];
                        continue;
                        
                    default:
                        State = hsPerl_Normal;
                        continue;
                    }
                } else if (len >= 2 && ((*p == '-' && p[1] == '-') || (*p == '+' && p[1] == '+'))) {
                    hlState s = State;
                    State = hsPerl_Punct;
                    Color = Colors[CLR_Punctuation];
                    ColorNext();
                    ColorNext();
                    State = s;
                    continue;
                } else if (len >= 2 && *p == '&' && (p[1] == '&' || isspace(p[1]))) {
                    State = hsPerl_Punct;
                    Color = Colors[CLR_Punctuation];
                    ColorNext();
                    ColorNext();
                    State = hsPerl_Normal;
                    continue;
                } else if (*p == '&' && (len < 2 || p[1] != '&') && X_NOT(State)) {
                    State = hsPerl_Function;
                    Color = Colors[CLR_Function];
                    ColorNext();
                    while ((len > 0) && (*p == '$' ||
                                         *p == '@' ||
                                         *p == '*' ||
                                         *p == '%' ||
                                         *p == '\\'))
                        ColorNext();
                    while ((len > 0) && (isalnum(*p) || 
                                         *p == '_' ||
                                         *p == '\''))
                        ColorNext();
                    State = hsPerl_Normal | X_BIT;
                    continue;
                } else if ((*p == '$') && (len > 1) &&
                           ((p[1] == '$') || p[1] == '"')) {
                    State = hsPerl_Variable;
                    Color = Colors[CLR_Variable];
                    ColorNext();
                    ColorNext();
                    State = hsPerl_Normal | X_BIT;
                    continue;
                } else if (*p == '$' || *p == '@' || *p == '\\' || (len > 2 && (*p == '%' || *p == '*') && X_NOT(State))) {
                    char var_type = *p;
                    State = hsPerl_Variable;
                    Color = Colors[CLR_Variable];
                    ColorNext();
                    while ((len > 0) && ((*p == ' ') || (*p == '\t'))) {
                        IF_TAB() else
                            ColorNext();
                    }
                    /*while ((len > 0) && (*p == '$' ||
                                         *p == '@' ||
                                         *p == '*' ||
                                         *p == '%' ||
                                         *p == '\\'))
                        ColorNext();*/
                    char first = *p;
                    if (len > 0 && *p != ' ' && *p != '\t' && *p != '"' && *p != '\'')
                        ColorNext();
                    // the following are one-character-ONLY
                    if (
                        (var_type == '$' && strchr("_&`'+*.!/|,\\\";#%=-~:?$<>()[]", first) != NULL) ||
                        (var_type == '@' && strchr("_", first) != NULL)
                       )
                    {
                        // nothing.
                    }
                    // the following are one-or-two-characters-ONLY
                    else if (first == '^')
                    {
                        if (len > 0 && isalpha(*p))
                            ColorNext();
                    }
                    else if (first == '{')
                    {
                        int Count[] = {
                            1, // { } - we're starting with one.
                            0, // ( )
                            0, // [ ]
                        };

                        while (len > 0)
                        {
                            switch (*p) {
                            case '{':
                                ++Count[0];
                                break;
                            case '}':
                                --Count[0];
                                break;
                            case '[':
                                ++Count[1];
                                break;
                            case ']':
                                --Count[1];
                                break;
                            case '(':
                                ++Count[2];
                                break;
                            case ')':
                                --Count[2];
                                break;
                            }
                            ColorNext();
                            if (TEST_ZERO)
                                break;
                        }

                    }
                    else
                    {
                        while ((len > 0) && (isalnum(*p) ||
                                             (first == '{' && (*p == '^' || *p == '}')) ||
                                             *p == '_' ||
                                             *p == '\'')
                              )
                            ColorNext();
                    }
                    State = hsPerl_Normal | X_BIT;
                    continue;
                } else if ((len >= 2) && (*p == '0') && (*(p+1) == 'x')) {
                    State = hsPerl_Number;
                    Color = Colors[CLR_Number];
                    ColorNext();
                    ColorNext();
                    while (len && (isxdigit(*p) || *p == '_')) ColorNext();
                    //                    if (len && (toupper(*p) == 'U')) ColorNext();
                    //                    if (len && (toupper(*p) == 'L')) ColorNext();
                    State = hsPerl_Normal | X_BIT;
                    continue;
                } else if (isdigit(*p)) {
                    State = hsPerl_Number;
                    Color = Colors[CLR_Number];
                    ColorNext();
                    while (len && (isdigit(*p) || (*p == 'e' || *p == 'E' || *p == '_'))) ColorNext();
                    //                    if (len && (toupper(*p) == 'U')) ColorNext();
                    //                    if (len && (toupper(*p) == 'L')) ColorNext();
                    State = hsPerl_Normal | X_BIT;
                    continue;
                } else if (*p == '\'') {
                    State = QSET(hsPerl_String1, '\'');
                    Color = Colors[CLR_String];
                    goto hilit;
                } else if (*p == '"') {
                    State = QSET(hsPerl_String2, '"');
                    Color = Colors[CLR_String];
                    goto hilit;
                } else if (*p == '<' && len > 2 && p[1] == '<' &&
                           (p[2] == '"' || p[2] == '\'' || p[2] == '_' || (toupper(p[2]) >= 'A' && toupper(p[2]) <= 'Z')))
                {
                    int hereDocKeyLen;
                    setHereDoc++;
                    for (hereDocKeyLen = 0;
                         hereDocKeyLen < len && (
                                                 p[2 + hereDocKeyLen] == '_' ||
                                                 (toupper(p[2 + hereDocKeyLen]) >= 'A' && toupper(p[2 + hereDocKeyLen]) <= 'Z')
                                                );
                         ++hereDocKeyLen)
                    {
                        hereDocKey[hereDocKeyLen] = p[2 + hereDocKeyLen];
                    }
                    hereDocKey[hereDocKeyLen] = '\0';
                    State = hsPerl_Punct;
                    Color = Colors[CLR_Punctuation];
                    ColorNext();
                    State = hsPerl_Normal;
                    continue;
                } else if (*p == '`') {
                    State = QSET(hsPerl_StringBk, '`');
                    Color = Colors[CLR_String];
                    goto hilit;
                } else if (*p == '#') {
                    State = hsPerl_Comment | (State & X_BIT);
                    continue;
                } else if (X_NOT(State) && *p == '/') {
                    State = QSET(hsPerl_Regexp1, '/');
                    Color = Colors[CLR_RegexpDelim];
                    goto hilit;
                } else if (X_NOT(State) &&
                           *p == '-' &&
                           len >= 2 &&
                           isalpha(p[1])
                          ) {
                    Color = Colors[CLR_Normal]; // default.
                    if (strchr("wrxoRWXOezsfdlpSbctugkTB", p[1]) != NULL) {
                        Color = Colors[CLR_Punctuation]; // new default.
                        if (len > 2) {
                            switch(p[2]) {
                            case '_': // there may be others...
                                Color = Colors[CLR_Normal];
                                break;
                            default:
                                if (isalnum(p[2]))
                                    Color = Colors[CLR_Normal];
                                break;
                            }
                        }
                    }
                    ColorNext();
                    ColorNext();
                    State = hsPerl_Normal;
                    continue;
                } else if (*p == ')' || *p == ']') {
                    State = hsPerl_Punct;
                    Color = Colors[CLR_Punctuation];
                    ColorNext();
                    State = hsPerl_Normal | X_BIT;
                    continue;
                } else if (ispunct(*p)) {
                    State = hsPerl_Punct;
                    Color = Colors[CLR_Punctuation];
                    ColorNext();
                    State = hsPerl_Normal;
                    continue;
                }
                Color = Colors[CLR_Normal];
                goto hilit;
            case hsPerl_Quote1Op:
                if (*p != ' ' && !kwd(*p)) {
                    if (IS_OBRACE(*p))
                        State = QSET(hsPerl_Quote1M,
                                     (1U << 2) | NUM_BRACE(*p));
                    else
                        State = QSET(hsPerl_Quote1, *p);
                    Color = Colors[CLR_QuoteDelim];
                    goto hilit;
                } else if (kwd(*p)) {
                    State = hsPerl_Normal | X_BIT;
                    continue;
                }
                Color = Colors[CLR_Punctuation];
                goto hilit;
            case hsPerl_Quote1:
                Color = Colors[CLR_String];
                if ((len >= 2) && (*p == '\\')) {
                    goto hilit2;
                } else if (*p == QCHAR(State)) {
                    Color = Colors[CLR_QuoteDelim];
                    ColorNext();
                    State = hsPerl_Normal | X_BIT;
                    continue;
                }
                goto hilit;
            case hsPerl_Quote1M:
                Color = Colors[CLR_String];
                if ((len >= 2) && (*p == '\\')) {
                    goto hilit2;
                } else if (GET_BRACE(QCHAR(State) & 3) == *p) {
                    State += 1 << (2 + 8);
                    goto hilit;
                } else if (IS_MBRACE(GET_BRACE(QCHAR(State) & 3), *p)) {
                    State -= 1 << (2 + 8);
                    if ((QCHAR(State) >> 2) == 0) {
                        Color = Colors[CLR_QuoteDelim];
                        ColorNext();
                        State = hsPerl_Normal | X_BIT;
                    } else
                        goto hilit;
                    continue;
                }
                goto hilit;
            case hsPerl_Regexp1Op:
                if (*p != ' ' && !kwd(*p)) {
                    if (IS_OBRACE(*p))
                        State = QSET(hsPerl_Regexp1M,
                                     (1U << 2) | NUM_BRACE(*p));
                    else
                        State = QSET(hsPerl_Regexp1, *p);
                    Color = Colors[CLR_RegexpDelim];
                    goto hilit;
                } else if (kwd(*p)) {
                    State = hsPerl_Normal | X_BIT;
                    continue;
                }
                Color = Colors[CLR_Regexp];
                goto hilit;
            case hsPerl_Regexp1:
                Color = Colors[CLR_Regexp];
                if ((len >= 2) && (*p == '\\')) {
                    goto hilit2;
                } else if (*p == QCHAR(State)) {
                    Color = Colors[CLR_RegexpDelim];
                    ColorNext();
                    Color = Colors[CLR_Punctuation];
                    while (len > 0 && isalpha(*p))
                        ColorNext();
                    State = hsPerl_Normal | X_BIT;
                    continue;
                }
                goto hilit;
            case hsPerl_Regexp1M:
                Color = Colors[CLR_Regexp];
                if ((len >= 2) && (*p == '\\')) {
                    goto hilit2;
                } else if (GET_BRACE(QCHAR(State) & 3) == *p) {
                    State += 1 << (2 + 8);
                    goto hilit;
                } else if (IS_MBRACE(GET_BRACE(QCHAR(State) & 3), *p)) {
                    State -= 1 << (2 + 8);
                    if ((QCHAR(State) >> 2) == 0) {
                        Color = Colors[CLR_RegexpDelim];
                        ColorNext();
                        Color = Colors[CLR_Punctuation];
                        while (len > 0 && isalpha(*p))
                            ColorNext();
                        State = hsPerl_Normal | X_BIT;
                    } else
                        goto hilit;
                    continue;
                }
                goto hilit;
            case hsPerl_Regexp2Op:
                if (*p != ' ' && !kwd(*p)) {
                    if (IS_OBRACE(*p))
                        State = QSET(hsPerl_Regexp2M,
                                     (1U << 2) | NUM_BRACE(*p));
                    else
                        State = QSET(hsPerl_Regexp2, *p);
                    Color = Colors[CLR_RegexpDelim];
                    goto hilit;
                } else if (kwd(*p)) {
                    State = hsPerl_Normal | X_BIT;
                    continue;
                }
                Color = Colors[CLR_Regexp];
                goto hilit;
            case hsPerl_Regexp2:
                Color = Colors[CLR_Regexp];
                if ((len >= 2) && (*p == '\\')) {
                    goto hilit2;
                } else if (*p == QCHAR(State)) {
                    Color = Colors[CLR_RegexpDelim];
                    ColorNext();
                    /*State = hsPerl_Normal | X_BIT;*/
                    State = QSET(hsPerl_Regexp1, QCHAR(State));
                    continue;
                }
                goto hilit;
            case hsPerl_Regexp2M:
                Color = Colors[CLR_Regexp];
                if ((len >= 2) && (*p == '\\')) {
                    goto hilit2;
                } else if (GET_BRACE(QCHAR(State) & 3) == *p) {
                    State += 1 << (2 + 8);
                    goto hilit;
                } else if (IS_MBRACE(GET_BRACE(QCHAR(State) & 3), *p)) {
                    State -= 1 << (2 + 8);
                    if ((QCHAR(State) >> 2) == 0) {
                        Color = Colors[CLR_RegexpDelim];
                        ColorNext();
                        /*State = hsPerl_Normal | X_BIT;*/
                        State = hsPerl_Regexp1Op;
                    } else
                        goto hilit;
                    continue;
                }
                goto hilit;
            case hsPerl_Data:
                Color = Colors[CLR_Comment];
                goto hilit;
            case hsPerl_HereDoc:
                if (!isEOHereDoc)
                {
                    Color = Colors[CLR_String];
                    goto hilit;
                }
                Color = Colors[CLR_Punctuation];
                setHereDoc = QCHAR(State);
                while (len > 0)
                    ColorNext();
                State = hsPerl_Normal | (State & X_BIT);
                continue;
            case hsPerl_Docs:
                Color = Colors[CLR_Comment];
                if (i == 0 && *p == '=' && len > 3 &&
                    p[1] == 'c' && p[2] == 'u' && p[3] == 't')
                {
                    ColorNext();
                    ColorNext();
                    ColorNext();
                    ColorNext();
                    State = hsPerl_Normal;
                    Color = Colors[CLR_Normal];
                    continue;
                }
                goto hilit;
            case hsPerl_Comment:
                Color = Colors[CLR_Comment];
                goto hilit;
            case hsPerl_String1:
                Color = Colors[CLR_String];
                if ((len >= 2) && (*p == '\\')) {
                    goto hilit2;
                } else if (*p == QCHAR(State)) {
                    ColorNext();
                    State = hsPerl_Normal | X_BIT;
                    continue;
                }
                goto hilit;
            case hsPerl_String2:
                Color = Colors[CLR_String];
                if ((len >= 2) && (*p == '\\')) {
                    goto hilit2;
                } else if (*p == QCHAR(State)) {
                    ColorNext();
                    State = hsPerl_Normal | X_BIT;
                    continue;
                }
                goto hilit;
            case hsPerl_StringBk:
                Color = Colors[CLR_String];
                if ((len >= 2) && (*p == '\\')) {
                    goto hilit2;
                } else if (*p == QCHAR(State)) {
                    ColorNext();
                    State = hsPerl_Normal | X_BIT;
                    continue;
                }
                goto hilit;
            }
        }
    }
    if ((State & X_MASK) == hsPerl_Comment)
        State = hsPerl_Normal | (State & X_BIT);
    if (setHereDoc)
        State = QSET(hsPerl_HereDoc | (State & X_BIT), setHereDoc - 1);
    *ECol = C;
    return 0;
}
#endif
