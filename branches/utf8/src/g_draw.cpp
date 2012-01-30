/*    g_draw.cpp
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "console.h"

#ifdef  NTCONSOLE
#   define  WIN32_LEAN_AND_MEAN 1
#   include <windows.h>
#endif

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

#ifndef NTCONSOLE

void MoveCh(PCell B, unichar_t CCh, TAttr Attr, int Count) {
    PCell p = B;
    while (Count > 0) {
        p->chr = CCh;
        p->attr = Attr;
        p++;
        Count--;
    }
}

void MoveChar(PCell B, int Pos, int Width, const unichar_t CCh, TAttr Attr, int Count) {
    PCell p = B;
    if (Pos < 0) {
        Count += Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + Count > Width) Count = Width - Pos;
    if (Count <= 0) return;
    for (p += Pos; Count > 0; Count--) {
        p->chr = CCh;
        p->attr = Attr;
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
        p->chr = *Ch++;
        p->attr = Attr;
        p++;
    }
}

#ifdef UNICODE_ENABLED
void MoveMem(PCell B, int Pos, int Width, const unichar_t* Ch, TAttr Attr, int Count) {
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
        p->chr = *Ch++;
        p->attr = Attr;
        p++;
    }
}
#endif

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
        p->chr = *Ch++;
        p->attr = Attr;
        p++;
    }
}

#ifdef UNICODE_ENABLED
void MoveStr(PCell B, int Pos, int Width, const unichar_t* Ch, TAttr Attr, int MaxCount) {
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
        p->chr = *Ch++;
        p->attr = Attr;
        p++;
    }
}
#endif

void MoveCStr(PCell B, int Pos, int Width, const char* Ch, TAttr A0, TAttr A1, int MaxCount) {
    PCell p = B;

    char was = 0;
    if (Pos < 0) {
        MaxCount += Pos;
        Ch -= Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + MaxCount > Width) MaxCount = Width - Pos;
    if (MaxCount <= 0) return;
    for (p += Pos; MaxCount > 0 && (*Ch != 0); MaxCount--) {
        if (*Ch == '&' && !was) {
            Ch++;
            MaxCount++;
            was = 1;
            continue;
        }
        p->chr = *Ch++;
        if (was) {
            p->attr = A1;
            was = 0;
        } else
            p->attr = A0;
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
    for (p += Pos; Count > 0; Count--) {
        p->attr = Attr;
        p++;
    }
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
        p->attr = ((unsigned char)(p->attr & 0x0F)) | ((unsigned char) Attr);
        p++;
    }
}

#else

void MoveCh(PCell B, unichar_t Ch, TAttr Attr, int Count) {
    PCHAR_INFO p = (PCHAR_INFO) B;
    while (Count > 0) {
        p->Char.AsciiChar = Ch;
        p->Attributes = Attr;
        p++;
        Count--;
    }
}

void MoveChar(PCell B, int Pos, int Width, const unichar_t Ch, TAttr Attr, int Count) {
    PCHAR_INFO p = (PCHAR_INFO) B;
    if (Pos < 0) {
        Count += Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + Count > Width) Count = Width - Pos;
    if (Count <= 0) return;
    for (p += Pos; Count > 0; Count--) {
        p->Char.AsciiChar = Ch;
        p->Attributes = Attr;
        p++;
    }
}

void MoveMem(PCell B, int Pos, int Width, const char* Ch, TAttr Attr, int Count) {
    PCHAR_INFO p = (PCHAR_INFO) B;

    if (Pos < 0) {
        Count += Pos;
        Ch -= Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + Count > Width) Count = Width - Pos;
    if (Count <= 0) return;
    for (p += Pos; Count > 0; Count--) {
        p->Char.AsciiChar = *Ch++;
        p->Attributes = Attr;
        p++;
    }
}

void MoveStr(PCell B, int Pos, int Width, const char* Ch, TAttr Attr, int MaxCount) {
    PCHAR_INFO p = (PCHAR_INFO) B;

    if (Pos < 0) {
        MaxCount += Pos;
        Ch -= Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + MaxCount > Width) MaxCount = Width - Pos;
    if (MaxCount <= 0) return;
    for (p += Pos; MaxCount > 0 && (*Ch != 0); MaxCount--) {
        p->Char.AsciiChar = *Ch++;
        p->Attributes = Attr;
        p++;
    }
}

void MoveCStr(PCell B, int Pos, int Width, const char* Ch, TAttr A0, TAttr A1, int MaxCount) {
    PCHAR_INFO p = (PCHAR_INFO) B;
    char was;
    //TAttr A;

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
        p->Char.AsciiChar = (unsigned char)(*Ch++);
        if (was) {
            p->Attributes = A1;
            was = 0;
        } else
            p->Attributes = A0;
        p++;
    }
}

void MoveAttr(PCell B, int Pos, int Width, TAttr Attr, int Count) {
    PCHAR_INFO p = (PCHAR_INFO) B;

    if (Pos < 0) {
        Count += Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + Count > Width) Count = Width - Pos;
    if (Count <= 0) return;
    for (p += Pos; Count > 0; Count--, p++)
        p->Attributes = Attr;
}

void MoveBgAttr(PCell B, int Pos, int Width, TAttr Attr, int Count) {
    PCHAR_INFO p = (PCHAR_INFO) B;

    if (Pos < 0) {
        Count += Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + Count > Width) Count = Width - Pos;
    if (Count <= 0) return;
    for (p += Pos; Count > 0; Count--) {
        p->Attributes =
            ((unsigned char)(p->Attributes & 0xf)) |
            ((unsigned char) Attr);
        p++;
    }
}

#endif
