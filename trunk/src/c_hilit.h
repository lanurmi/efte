/*    c_hilit.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef HILIT_H_
#define HILIT_H_

#include "console.h"
#include "c_mode.h"
#include "e_regex.h"

#include <sys/types.h>

class EBuffer;
class ELine;

typedef unsigned short hlState;
typedef unsigned char hsState;

#define HILIT_P(proc) \
    int proc(EBuffer *BF, int LN, PCell B, int Pos, int Width, ELine *Line, hlState &State, hsState *StateMap, int *ECol)

//typedef int (*SyntaxProc)(EBuffer *BF, int LN, PCell B, int Pos, int Width, ELine *Line, hlState &State, hsState *StateMap);
typedef HILIT_P((*SyntaxProc));


int Indent_Plain(EBuffer *B, int Line, int PosCursor);
HILIT_P(Hilit_Plain);

/* highlighting state */

HILIT_P(Hilit_C);
HILIT_P(Hilit_PERL);
HILIT_P(Hilit_MAKE);
HILIT_P(Hilit_REXX);
HILIT_P(Hilit_IPF);
HILIT_P(Hilit_ADA);
HILIT_P(Hilit_MSG);
HILIT_P(Hilit_SH);
HILIT_P(Hilit_PASCAL);
HILIT_P(Hilit_TEX);
HILIT_P(Hilit_FTE);
HILIT_P(Hilit_CATBS);
HILIT_P(Hilit_SIMPLE);

int Indent_C(EBuffer *B, int Line, int PosCursor);
int Indent_REXX(EBuffer *B, int Line, int PosCursor);
int Indent_SIMPLE(EBuffer *B, int Line, int PosCursor);

/*
 * NT has 2-byte charcode and attribute... Following is not portable to non-
 * intel; should be replaced by formal TCell definition' usage instead of
 * assumed array.. (Jal)
 */
#ifdef NTCONSOLE
#    define PCLI unsigned short
#else
#    define PCLI unsigned char
#endif

#define HILIT_CLRD() \
    ((Color < COUNT_CLR) ? Colors[Color] : Color - COUNT_CLR)

#define ColorChar() \
    do {\
    BPos = C - Pos; \
    if (B) \
    if (BPos >= 0 && BPos < Width) { \
    BPtr = (PCLI *) (B + BPos); \
    BPtr[0] = *p; \
    BPtr[1] = HILIT_CLRD(); \
    } \
    if (StateMap) StateMap[i] = (hsState)(State & 0xFF); \
    } while (0)

// MoveChar(B, C - Pos, Width, *p, Color, 1);
// if (StateMap) StateMap[i] = State; }

#define NextChar() do { i++; p++; len--; C++; } while (0)
#define ColorNext() do { ColorChar(); NextChar(); } while (0)

#define UntilMatchBrace(first, cmd) \
    do { \
        int Count[] = { 0, 0, 0, }; \
        switch (first) \
        { \
            case '{': ++Count[0]; break; \
            case '[': ++Count[1]; break; \
            case '(': ++Count[2]; break; \
        } \
\
        while (len > 0)        \
        {                      \
            switch (*p) {      \
            case '{':          \
                ++Count[0];    \
                break;         \
            case '}':          \
                --Count[0];    \
                break;         \
            case '[':          \
                ++Count[1];    \
                break;         \
            case ']':          \
                --Count[1];    \
                break;         \
            case '(':          \
                ++Count[2];    \
                break;         \
            case ')':          \
                --Count[2];    \
                break;         \
            }                  \
            cmd;               \
            if (TEST_ZERO)     \
                break;         \
        } \
    } while (0)

#define HILIT_VARS(ColorTable, Line) \
    PCLI *BPtr; \
    int BPos; \
    ChColor *Colors = ColorTable; \
    ChColor Color = CLR_Normal; \
    int i; \
    int len = Line->Count; \
    char *p = Line->Chars; \
    int NC = 0, C = 0; \
    int TabSize = BFI(BF, BFI_TabSize); \
    int ExpandTabs = BFI(BF, BFI_ExpandTabs);

//#define HILIT_VARS2()
//    int len1 = len;
//    char *last = p + len1 - 1;

#define IF_TAB() \
    if (*p == '\t' && ExpandTabs) { \
    NC = NextTab(C, TabSize); \
    if (StateMap) StateMap[i] = hsState(State);\
    if (B) MoveChar(B, C - Pos, Width, ' ', HILIT_CLRD(), NC - C);\
    if (BFI(BF, BFI_ShowTabs)) ColorChar();\
    i++,len--,p++;\
    C = NC;\
    continue;\
    }

#define CK_MAXLEN 64

inline bool isZeroArray(int* Count, size_t len) {
    for (size_t i = 0; i < len; ++i)
        if (Count[i] != 0)
            return 0;
    return 1;
}

#define TEST_ZERO isZeroArray(Count, sizeof(Count)/sizeof(Count[0]))

typedef struct {
    int TotalCount;
    int count[CK_MAXLEN];
    char *key[CK_MAXLEN];
} ColorKeywords;

struct HTrans {
    char *match;
    int matchLen;
    long matchFlags;
    int nextState;
    int color;
    RxNode *regexp;

    void InitTrans();
};

struct HState {
    int transCount;
    int firstTrans;
    int color;

    ColorKeywords keywords;
    char *wordChars;
    long options;
    int nextKwdMatchedState;
    int nextKwdNotMatchedState;
    int nextKwdNoCharState;

    void InitState();
    int GetHilitWord(int len, char *str, ChColor &clr);
};

class HMachine {
public:
    int stateCount;
    int transCount;
    HState *state;
    HTrans *trans;

    HMachine();
    ~HMachine();
    void AddState(HState &aState);
    void AddTrans(HTrans &aTrans);

    HState *LastState() {
        return state + stateCount - 1;
    }
};

class EColorize {
public:
    char *Name;
    EColorize *Next;
    EColorize *Parent;
    int SyntaxParser;
    ColorKeywords Keywords; // keywords to highlight
    HMachine *hm;
    ChColor Colors[COUNT_CLR];

    EColorize(const char *AName, const char *AParent);
    ~EColorize();

    int SetColor(int clr, const char *value);
};

extern EColorize *Colorizers;
EColorize *FindColorizer(const char *AName);

SyntaxProc GetHilitProc(int id);

int IsState(hsState *Buf, hsState State, int Len);
int LookAt(EBuffer *B, int Row, unsigned int Pos, const char *What, hsState State, int NoWord = 1, int CaseInsensitive = 0);
inline int LookAtNoCase(EBuffer *B, int Row, unsigned int Pos, const char *What, hsState State, int NoWord = 1) {
    return LookAt(B, Row, Pos, What, State, NoWord, 1);
}

#endif /* __HILIT_H_ */
