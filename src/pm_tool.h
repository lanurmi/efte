#ifndef __TOOLBAR_H
#define __TOOLBAR_H

#define tiBITMAP     1
#define tiSEPARATOR  2

#define tfDISABLED   1
#define tfDEPRESSED  0x8000

#define WC_MTOOLBAR  "MToolBar"

typedef struct {
    ULONG ulType;
    ULONG ulId;
    ULONG ulCommand;
    ULONG ulFlags;
    HBITMAP hBitmap;
} ToolBarItem;

typedef struct {
    USHORT cb;
    LONG ulCount;
    ToolBarItem *pItems;
    LONG ulDepressed;
} ToolBarData;

MRESULT EXPENTRY ToolBarProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
void RegisterToolBarClass(HAB hab);
HWND CreateToolBar(HWND parent,
                   HWND owner,
                   ULONG id,
                   ULONG count,
                   ToolBarItem *items);

#endif
