/*    c_hilit.cpp
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

static const struct {
    const char *Name;
    int Num;
    SyntaxProc Proc;
} HilitModes[] = {
    { "PLAIN", HILIT_PLAIN, Hilit_Plain },
    { "C", HILIT_C, Hilit_C },
    { "REXX", HILIT_REXX, Hilit_REXX },
    { "PERL", HILIT_PERL, Hilit_PERL },
    { "MAKE", HILIT_MAKE, Hilit_MAKE },
    { "IPF", HILIT_IPF, Hilit_IPF },
    { "Ada", HILIT_ADA, Hilit_ADA },
    { "MSG", HILIT_MSG, Hilit_MSG },
    { "SH", HILIT_SH, Hilit_SH },
    { "PASCAL", HILIT_PASCAL, Hilit_PASCAL },
    { "TEX", HILIT_TEX, Hilit_TEX },
    { "FTE", HILIT_FTE, Hilit_FTE },
    { "CATBS", HILIT_CATBS, Hilit_CATBS },
    { "SIMPLE", HILIT_SIMPLE, Hilit_SIMPLE },
};

static const struct {
    const char *Name;
    int Num;
} IndentModes[] = {
    { "C", INDENT_C },
    { "REXX", INDENT_REXX },
    { "SIMPLE", INDENT_REXX },
    { "PLAIN", INDENT_PLAIN },
};

EColorize *Colorizers = 0;

int GetIndentMode(const char *Str) {
    for (unsigned int i = 0; i < sizeof(IndentModes) / sizeof(IndentModes[0]); i++)
        if (strcmp(Str, IndentModes[i].Name) == 0)
            return IndentModes[i].Num;
    return 0;
}

int GetHilitMode(const char *Str) {
    for (unsigned int i = 0; i < sizeof(HilitModes) / sizeof(HilitModes[0]); i++)
        if (strcmp(Str, HilitModes[i].Name) == 0)
            return HilitModes[i].Num;
    return HILIT_PLAIN;
}

SyntaxProc GetHilitProc(int id) {
    for (unsigned int i = 0; i < sizeof(HilitModes) / sizeof(HilitModes[0]); i++)
        if (id == HilitModes[i].Num)
            return HilitModes[i].Proc;
    return 0;
}

int EBuffer::HilitAddWord(const char *Word) {
    if (HilitFindWord(Word) == 1)
        return 1;
    WordList = (char **)realloc((void *)WordList, (1 + WordCount) * sizeof(char *));
    if (WordList == 0) return 0;
    WordList[WordCount++] = strdup(Word);
    FullRedraw();
    return 1;
}

int EBuffer::HilitFindWord(const char *Word) {
    for (int i = 0; i < WordCount; i++) {
        if (BFI(this, BFI_MatchCase) == 1) {
            if (strcmp(Word, WordList[i]) == 0) return 1;
        } else {
            if (stricmp(Word, WordList[i]) == 0) return 1;
        }
    }
    return 0;
}

int EBuffer::HilitRemoveWord(const char *Word) {
    for (int i = 0; i < WordCount; i++) {
        if (BFI(this, BFI_MatchCase) == 1) {
            if (strcmp(Word, WordList[i]) != 0) continue;
        } else {
            if (stricmp(Word, WordList[i]) != 0) continue;
        }
        free(WordList[i]);
        memmove(WordList + i, WordList + i + 1, sizeof(char *) * (WordCount - i - 1));
        WordCount--;
        WordList = (char **)realloc((void *)WordList, WordCount * sizeof(char *));
        FullRedraw();
        return 1;
    }
    return 0;
}

int EBuffer::HilitWord() {
    PELine L = VLine(CP.Row);
    char s[CK_MAXLEN + 2];
    int P, len = 0;

    P = CharOffset(L, CP.Col);
    while ((P > 0) && ((ChClass(L->Chars[P - 1]) == 1) || (L->Chars[P - 1] == '_')))
        P--;
    while (len < CK_MAXLEN && P < L->Count && (ChClass(L->Chars[P]) == 1 || L->Chars[P] == '_'))
        s[len++] = L->Chars[P++];
    if (len == 0)
        return 0;
    s[len] = 0;

    return (HilitFindWord(s)) ? HilitRemoveWord(s) : HilitAddWord(s);
}

/* ======================================================================= */

EColorize::EColorize(const char *AName, const char *AParent) {
    Name = strdup(AName);
    SyntaxParser = HILIT_PLAIN;
    Next = Colorizers;
    hm = 0;
    Colorizers = this;
    Parent = FindColorizer(AParent);
    memset((void *)&Keywords, 0, sizeof(Keywords));
    memset((void *)Colors, 0, sizeof(Colors));
    if (Parent) {
        SyntaxParser = Parent->SyntaxParser;
        memcpy((void *)Colors, (void *)Parent->Colors, sizeof(Colors));
    } else {
        SyntaxParser = HILIT_PLAIN;
    }
}

EColorize::~EColorize() {
    free(Name);

    for (int i = 0; i < CK_MAXLEN; i++)
        free(Keywords.key[i]);

    delete hm;
}

EColorize *FindColorizer(const char *AName) {
    EColorize *p = Colorizers;

    while (p) {
        if (strcmp(AName, p->Name) == 0)
            return p;
        p = p->Next;
    }
    return 0;
}

int EColorize::SetColor(int idx, const char *Value) {
    unsigned int ColBg, ColFg;

    if (sscanf(Value, "%1X %1X", &ColFg, &ColBg) != 2)
        return 0;

    if (idx < 0 || idx >= COUNT_CLR)
        return 0;
    Colors[idx] = ColFg | (ColBg << 4);
    return 1;
}

/* ======================================================================= */

void HTrans::InitTrans() {
    match = 0;
    matchLen = 0;
    matchFlags = 0;
    nextState = 0;
    color = 0;
    regexp = 0;
}

/* ======================================================================= */

void HState::InitState() {
    memset((void *)&keywords, 0, sizeof(keywords));
    firstTrans = 0;
    transCount = 0;
    color = 0;
    wordChars = 0;
    options = 0;
    nextKwdMatchedState = -1;
    nextKwdNotMatchedState = -1;
    nextKwdNoCharState = -1;
}

int HState::GetHilitWord(int len, char *str, ChColor &clr) {
    char *p;

    if (len >= CK_MAXLEN || len < 1)
        return 0;

    p = keywords.key[len];
    if (options & STATE_NOCASE) {
        while (p && *p) {
            if (strnicmp(p, str, len) == 0) {
                clr = COUNT_CLR + ((unsigned char*)p)[len];
                return 1;
            }
            p += len + 1;
        }
    } else {
        while (p && *p) {
            if (memcmp(p, str, len) == 0) {
                clr = COUNT_CLR + ((unsigned char*)p)[len];
                return 1;
            }
            p += len + 1;
        }
    }
    return 0;
}

/* ======================================================================= */

HMachine::HMachine() {
    stateCount = 0;
    state = 0;
    transCount = 0;
    trans = 0;
}

HMachine::~HMachine() {

    // free states
    if (state) {
        int i;

        while (stateCount--) {
            for (i = 0; i < CK_MAXLEN; i++)
                free(state[stateCount].keywords.key[i]);

            free(state[stateCount].wordChars);
        }

        free(state);
    }

    // free transes
    if (trans) {
        while (transCount--) {
            if (trans[transCount].match) free(trans[transCount].match);
            if (trans[transCount].regexp) RxFree(trans[transCount].regexp);
        }

        free(trans);
    }

}

void HMachine::AddState(HState &aState) {
    state = (HState *)realloc(state, (stateCount + 1) * sizeof(HState));
    assert(state);
    state[stateCount] = aState;
    state[stateCount].firstTrans = transCount;
    stateCount++;
}

void HMachine::AddTrans(HTrans &aTrans) {
    assert(stateCount > 0);
    trans = (HTrans *)realloc(trans, (transCount + 1) * sizeof(HTrans));
    assert(trans);
    state[stateCount - 1].transCount++;
    trans[transCount] = aTrans;
    transCount++;
}
