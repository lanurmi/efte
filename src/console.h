/*    console.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_

/* don't change these, used as index */
#define DCH_C1 0
#define DCH_C2 1
#define DCH_C3 2
#define DCH_C4 3
#define DCH_H  4
#define DCH_V  5
#define DCH_M1 6
#define DCH_M2 7
#define DCH_M3 8
#define DCH_M4 9
#define DCH_X  10
#define DCH_RPTR 11
#define DCH_EOL 12
#define DCH_EOF 13
#define DCH_END 14
#define DCH_AUP 15
#define DCH_ADOWN 16
#define DCH_HFORE 17
#define DCH_HBACK 18
#define DCH_ALEFT 19
#define DCH_ARIGHT 20

#define ConMaxCols 256
#define ConMaxRows 128

#define csUp       0
#define csDown     1
#define csLeft     2
#define csRight    3

#define evNone             0
#define evKeyDown     0x0001
#define evKeyUp       0x0002
#define evMouseDown   0x0010
#define evMouseUp     0x0020
#define evMouseMove   0x0040
#define evMouseAuto   0x0080
#define evCommand     0x0100
#define evBroadcast   0x0200
#define evNotify      0x0400

#define evKeyboard    (evKeyDown | evKeyUp)
#define evMouse       (evMouseDown | evMouseUp | evMouseMove | evMouseAuto)
#define evMessage     (evCommand | evBroadcast)

#include "conkbd.h"

#define cmRefresh   1
#define cmResize    2
#define cmClose     3
#define cmPipeRead  4
#define cmMainMenu  5
#define cmPopupMenu 6

/* vertical scroll */

#define cmVScrollUp     10
#define cmVScrollDown   11
#define cmVScrollPgUp   12
#define cmVScrollPgDn   13
#define cmVScrollMove   14

/* horizontal scroll */

#define cmHScrollLeft   15
#define cmHScrollRight  16
#define cmHScrollPgLt   17
#define cmHScrollPgRt   18
#define cmHScrollMove   19

#define cmDroppedFile   30
#define cmRenameFile    31   /* TODO: in-place editing of titlebar */

typedef unsigned char TAttr;
typedef TAttr *PAttr;

#ifdef NTCONSOLE
typedef unsigned long TCell;
#else
typedef unsigned short TCell;
#endif

typedef TCell *PCell;
typedef TCell TDrawBuffer[ConMaxCols];
typedef TDrawBuffer *PDrawBuffer;
typedef unsigned long TEventMask;
typedef unsigned long TKeyCode;
typedef unsigned long TCommand;

class EModel; // forward
class GView;

typedef struct {
    TEventMask What;
    GView* View;
    TKeyCode Code;
} TKeyEvent;

typedef struct {
    TEventMask What;
    GView* View;
    long X;
    long Y;
    unsigned short Buttons;
    unsigned short Count;
    TKeyCode KeyMask;
} TMouseEvent;

typedef struct {
    TEventMask What;
    GView *View;
    EModel *Model;
    TCommand Command;
    long Param1;
    void *Param2;
} TMsgEvent;

typedef union {
    TEventMask What;
    TKeyEvent Key;
    TMouseEvent Mouse;
    TMsgEvent Msg;
    char fill[32];
} TEvent;

#define SUBMENU_NORMAL      (-1)
#define SUBMENU_CONDITIONAL (-2)

typedef struct _mItem {
    char *Name;
    char *Arg;
    int SubMenu;
    int Cmd;
} mItem;

typedef struct _mMenu {
    char *Name;
    int Count;
    mItem *Items;
} mMenu;

extern int MenuCount;
extern mMenu *Menus;

int ConInit(int XSize, int YSize);
int ConDone();
int ConSuspend();
int ConContinue();
int ConSetTitle(char *Title, char *STitle);
int ConGetTitle(char *Title, int MaxLen, char *STitle, int SMaxLen);

int ConClear();
int ConPutBox(int X, int Y, int W, int H, PCell Cell);
int ConGetBox(int X, int Y, int W, int H, PCell Cell);
int ConPutLine(int X, int Y, int W, int H, PCell Cell);
int ConSetBox(int X, int Y, int W, int H, TCell Cell);
int ConScroll(int Way, int X, int Y, int W, int H, TAttr Fill, int Count);

int ConSetSize(int X, int Y);
int ConQuerySize(int *X, int *Y);

int ConSetCursorPos(int X, int Y);
int ConQueryCursorPos(int *X, int *Y);
int ConShowCursor();
int ConHideCursor();
int ConCursorVisible();
void ConSetInsertState(bool insert);

int ConSetMousePos(int X, int Y);
int ConQueryMousePos(int *X, int *Y);
int ConShowMouse();
int ConHideMouse();
int ConMouseVisible();
int ConQueryMouseButtons(int *ButtonCount);

int ConGetEvent(TEventMask EventMask, TEvent *Event, int WaitTime, int Delete);
int ConPutEvent(TEvent Event);

void MoveCh(PCell B, char Ch, TAttr Attr, int Count);
void MoveChar(PCell B, int Pos, int Width, const char Ch, TAttr Attr, int Count);
void MoveMem(PCell B, int Pos, int Width, const char* Ch, TAttr Attr, int Count);
void MoveStr(PCell B, int Pos, int Width, const char* Ch, TAttr Attr, int MaxCount);
void MoveCStr(PCell B, int Pos, int Width, const  char* Ch, TAttr A0, TAttr A1, int MaxCount);
void MoveAttr(PCell B, int Pos, int Width, TAttr Attr, int Count);
void MoveBgAttr(PCell B, int Pos, int Width, TAttr Attr, int Count);

int CStrLen(const char *s);

int NewMenu(const char *Name);
int NewItem(int menu, const char *Name);
int NewSubMenu(int menu, const char *Name, int submenu, int type);
int GetMenuId(const char *Name);

char ConGetDrawChar(int index);

extern char WindowFont[64];

typedef struct {
    unsigned char r, g, b;
} TRGBColor;
extern TRGBColor RGBColor[16];
extern bool RGBColorValid[16];

#endif
