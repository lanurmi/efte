/*    g_draw.cpp
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "console.h"

int CStrLen(const char *p) {
    int len = 0, was = 0;
    while (*p) {
        len++;
        if (*p == '&' && !was) {
            len--;
            was = 1;
        }
        p++;
        was = 0;
    }
    return len;
}

void MoveCh(PCell B, char Ch, TAttr Attr, int Count) {
    PCell p = B;

    while (Count > 0) {
        p->Ch = Ch;
        p->Attr = Attr;
        Count--;
        p++;
    }
}

void MoveTCh(PCell B, TChar Ch, TAttr Attr, int Count) {
    PCell p = B;

    while (Count > 0) {
        p->Ch = Ch;
        p->Attr = Attr;
        Count--;
        p++;
    }
}

void MoveChar(PCell B, int Pos, int Width, const char Ch, TAttr Attr, int Count) {
    PCell p = B;

    if (Pos < 0) {
        Count += Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + Count > Width) Count = Width - Pos;
    if (Count <= 0) return;

    for (p += Pos; Count > 0; Count--) {
        p->Ch = Ch;
        p->Attr = Attr;
        p++;
    }
}

void MoveTChar(PCell B, int Pos, int Width, const TChar Ch, TAttr Attr, int Count) {
    PCell p = B;

    if (Pos < 0) {
        Count += Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + Count > Width) Count = Width - Pos;
    if (Count <= 0) return;

    for (p += Pos; Count > 0; Count--) {
        p->Ch = Ch;
        p->Attr = Attr;
        p++;
    }
}

void MoveMem(PCell B, int Pos, int Width, const char* Ch, TAttr Attr, int Count) {
    PCell p = B;
    
    if (Pos < 0) {
        Count += Pos;
        Ch -= Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + Count > Width) Count = Width - Pos;
    if (Count <= 0) return;
    for (p += Pos; Count > 0; Count--) {
        p->Ch = *Ch++;
        p->Attr = Attr;
        p++;
    }
}

void MoveTMem(PCell B, int Pos, int Width, const TChar* Ch, TAttr Attr, int Count) {
    PCell p = B;
    
    if (Pos < 0) {
        Count += Pos;
        Ch -= Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + Count > Width) Count = Width - Pos;
    if (Count <= 0) return;
    for (p += Pos; Count > 0; Count--) {
        p->Ch = *Ch++;
        p->Attr = Attr;
        p++;
    }
}

void MoveStr(PCell B, int Pos, int Width, const char* Ch, TAttr Attr, int MaxCount) {
    PCell p = B;
    
    if (Pos < 0) {
        MaxCount += Pos;
        Ch -= Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + MaxCount > Width) MaxCount = Width - Pos;
    if (MaxCount <= 0) return;
    for (p += Pos; MaxCount > 0 && (*Ch != 0); MaxCount--) {
        p->Ch = *Ch++;
        p->Attr = Attr;
        p++;
    }
}

void MoveTStr(PCell B, int Pos, int Width, const TChar* Ch, TAttr Attr, int MaxCount) {
    PCell p = B;
    
    if (Pos < 0) {
        MaxCount += Pos;
        Ch -= Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + MaxCount > Width) MaxCount = Width - Pos;
    if (MaxCount <= 0) return;
    for (p += Pos; MaxCount > 0 && (*Ch != 0); MaxCount--) {
        p->Ch = *Ch++;
        p->Attr = Attr;
        p++;
    }
}

void MoveCStr(PCell B, int Pos, int Width, const char* Ch, TAttr A0, TAttr A1, int MaxCount) {
    PCell p = B;
    char was;
    
    if (Pos < 0) {
        MaxCount += Pos;
        Ch -= Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + MaxCount > Width) MaxCount = Width - Pos;
    if (MaxCount <= 0) return;
    was = 0;
    for (p += Pos; MaxCount > 0 && (*Ch != 0); MaxCount--) {
        if (*Ch == '&' && !was) {
            Ch++;
            MaxCount++;
            was = 1;
            continue;
        } 
        p->Ch = (unsigned char) (*Ch++);
        if (was) {
            p->Attr = A1;
            was = 0;
        } else
            p->Attr = A0;
        p++;
    }
}

void MoveTCStr(PCell B, int Pos, int Width, const TChar* Ch, TAttr A0, TAttr A1, int MaxCount) {
    PCell p = B;
    char was;
    
    if (Pos < 0) {
        MaxCount += Pos;
        Ch -= Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + MaxCount > Width) MaxCount = Width - Pos;
    if (MaxCount <= 0) return;
    was = 0;
    for (p += Pos; MaxCount > 0 && (*Ch != 0); MaxCount--) {
        if (*Ch == '&' && !was) {
            Ch++;
            MaxCount++;
            was = 1;
            continue;
        } 
        p->Ch = (unsigned char) (*Ch++);
        if (was) {
            p->Attr = A1;
            was = 0;
        } else
            p->Attr = A0;
        p++;
    }
}

void MoveAttr(PCell B, int Pos, int Width, TAttr Attr, int Count) {
    PCell p = B;
    
    if (Pos < 0) {
        Count += Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + Count > Width) Count = Width - Pos;
    if (Count <= 0) return;
    for (p += Pos; Count > 0; Count--, p++)
        p->Attr = Attr;
}

void MoveBgAttr(PCell B, int Pos, int Width, TAttr Attr, int Count) {
    PCell p = B;
    
    if (Pos < 0) {
        Count += Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + Count > Width) Count = Width - Pos;
    if (Count <= 0) return;
    for (p += Pos; Count > 0; Count--) {
        p->Attr =
            ((unsigned char)(p->Attr & 0xf)) |
            ((unsigned char) Attr);
        p++;
    }
}

void MoveCell(PCell target, const PCell source, int Count) {
    memcpy(target, source, sizeof(TCell) * Count);
}
