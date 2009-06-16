/*    con_coca.cpp
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *    Copyright (c) 2008, Lauri Nurmi
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/* This file duplicates (way too much) code from con_x11.cpp. */

#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>
#include "sysdep.h"
#include "c_config.h"
#include "console.h"
#include "gui.h"

#include "con_i18n.h"
#include "s_files.h"
#include "s_util.h"
#include "s_string.h"

i18n_context_t* i18n_ctx = NULL;

#ifdef CAST_FD_SET_INT
#define FD_SET_CAST() (int *)
#else
#define FD_SET_CAST()
#endif

#define MIN_SCRWIDTH 20
#define MIN_SCRHEIGHT 6

#define MAX_PIPES 40
//#define PIPE_BUFLEN 4096

#define SELECTION_INCR_LIMIT 0x1000
#define SELECTION_XFER_LIMIT 0x1000
#define SELECTION_MAX_AGE 10

typedef struct {
    int used;
    int id;
    int fd;
    int pid;
    int stopped;
    EModel *notify;
} GPipe;

static GPipe Pipes[MAX_PIPES] = {
    { 0 },
};

static const long MouseAutoDelay = 40;
static const long MouseAutoRepeat = 200;
static const long MouseMultiClick = 300;

static int setUserPosition = 0;
static int initX = 0, initY = 0;
/*static*/
unsigned int ScreenCols = 80;
/*static*/
unsigned int ScreenRows = 25;
static unsigned int CursorX = 0;
static unsigned int CursorY = 0;
static int CursorVisible = 1;
static bool insertState = 1;
static int CursorStart, CursorEnd;
static unsigned long CursorLastTime;
// Cursor flashing interval, in msecs
static unsigned CursorFlashInterval = 300;
unsigned char *ScreenBuffer = NULL;
static int Refresh = 0;

static int useI18n = 1;
static int FontCX = 8, FontCY = 18;
static char winTitle[256] = "eFTE";
static char winSTitle[256] = "eFTE";

typedef struct _IncrementalSelectionInfo {
    struct _IncrementalSelectionInfo *next;
    unsigned char *data;
    int len;
    int pos;
    Atom requestor;
    Atom property;
    Atom type;
    time_t lastUse;
} IncrementalSelectionInfo;
IncrementalSelectionInfo *incrementalSelections = NULL;

static Bool gotXError;

static void SendSelection(XEvent *notify, Atom property, Atom type, unsigned char *data, int len, Bool privateData);

static int ErrorHandler(Display *, XErrorEvent *ee) {
    gotXError = True;
    return 1;
}


static int AllocBuffer() {
    unsigned char *p;
    unsigned int i;

    ScreenBuffer = (unsigned char *)malloc(2 * ScreenCols * ScreenRows);
    if (ScreenBuffer == NULL) return -1;
    for (i = 0, p = ScreenBuffer; i < ScreenCols * ScreenRows; i++) {
        *p++ = 32;
        *p++ = 0x07;
    }
    return 0;
}

static struct {
    int r, g, b;
} dcolors[] = {
    {   0,   0,   0 },  //     black
    {   0,   0, 160 },  // darkBlue
    {   0, 160,   0 },  // darkGreen
    {   0, 160, 160 },  // darkCyan
    { 160,   0,   0 },  // darkRed
    { 160,   0, 160 },  // darkMagenta
    { 160, 160,   0 },  // darkYellow
    { 204, 204, 204 },  // paleGray
    { 160, 160, 160 },  // darkGray
    {   0,   0, 255 },  //     blue
    {   0, 255,   0 },  //     green
    {   0, 255, 255 },  //     cyan
    { 255,   0,   0 },  //     red
    { 255,   0, 255 },  //     magenta
    { 255, 255,   0 },  //     yellow
    { 255, 255, 255 },  //     white
};

#include "cocoa.h"

int ConInit(int XSize, int YSize) {
    if (XSize != -1)
        ScreenCols = XSize;
    if (YSize != -1)
        ScreenRows = YSize;
    if (AllocBuffer() == -1) return -1;
#ifndef NO_SIGNALS
    signal(SIGALRM, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
#endif
    return 0;
}

int ConDone(void) {
    free(ScreenBuffer);
    return 0;
}

int ConSuspend(void) {
    return 0;
}

int ConContinue(void) {
    return 0;
}

int ConClear(void) {
    TDrawBuffer B;
    MoveCh(B, ' ', 0x07, ScreenCols);
    return ConPutLine(0, 0, ScreenCols, ScreenRows, B);
}

int ConSetTitle(char *Title, char *STitle) {
}

int ConGetTitle(char *Title, int MaxLen, char *STitle, int SMaxLen) {
    strlcpy(Title, winTitle, MaxLen);
    strlcpy(STitle, winSTitle, SMaxLen);
    return 0;
}

#define InRange(x,a,y) (((x) <= (a)) && ((a) < (y)))
#define CursorXYPos(x,y) (ScreenBuffer + ((x) + ((y) * ScreenCols)) * 2)

void DrawCursor(int Show) {
#if 0
    if (CursorVisible) {
        unsigned char *p = CursorXYPos(CursorX, CursorY);
    }
#endif
}

int ConPutBox(int X, int Y, int W, int H, PCell Cell) {
    unsigned int i;
    unsigned char temp[256], attr;
    unsigned char *p, *ps, *c, *ops;
    unsigned int len, x, l, ox, olen, skip;


    if (X >= (int) ScreenCols || Y >= (int) ScreenRows ||
            X + W > (int) ScreenCols || Y + H > (int) ScreenRows) {
        //fprintf(stderr, "%d %d  %d %d %d %d\n", ScreenCols, ScreenRows, X, Y, W, H);
        return -1;
    }
    //XClearArea(display, win, X, Y, W * FontCX, H * FontCY, False);

    //fprintf(stderr, "%d %d  %d %d %d %d\n", ScreenCols, ScreenRows, X, Y, W, H);
    for (i = 0; i < (unsigned int)H; i++) {
        len = W;
        p = CursorXYPos(X, Y + i);
        ps = (unsigned char *) Cell;
        x = X;
        while (len > 0) {
            if (!Refresh) {
                c = CursorXYPos(x, Y + i);
                skip = 0;
                ops = ps;
                ox = x;
                olen = len;
                while ((len > 0) && c[0] == ps[0] && c[1] == ps[1]) {
                    ps += 2;
                    c += 2;
                    x++;
                    len--;
                    skip++;
                }
                if (len <= 0) break;
                if (skip <= 4) {
                    ps = ops;
                    x = ox;
                    len = olen;
                }
            }
            p = ps;
            l = 1;
            temp[0] = *ps++;
            attr = *ps++;
            while ((l < len) && ((unsigned char)(ps[1]) == attr)) {
                temp[l++] = *ps++;
                ps++;
            }
            //temp[l] = 0; printf("%s\n", temp);
            len -= l;
            x += l;
        }
        /* if (x < ScreenCols - 1) {
             printf("XX %d   %d   %d\n", X, x, W);
             XFillRectangle(display, win, GCs[15 * 16 + 7],
              x * FontCX, (Y + i) * FontCY,
              (ScreenCols - x - 1) * FontCX, FontCY);
         }
        */
        p = CursorXYPos(X, Y + i);
        memcpy(p, Cell, W * 2);
        if (i + Y == CursorY)
            DrawCursor(1);
        Cell += W;
    }
    return 0;
}

int ConGetBox(int X, int Y, int W, int H, PCell Cell) {
    int i;

    for (i = 0; i < H; i++) {
        memcpy(Cell, CursorXYPos(X, Y + i), 2 * W);
        Cell += W;
    }
    return 0;
}

int ConPutLine(int X, int Y, int W, int H, PCell Cell) {
    int i;
    for (i = 0; i < H; i++) {
        if (ConPutBox(X, Y + i, W, 1, Cell) != 0) return -1;
    }
    return 0;
}

int ConSetBox(int X, int Y, int W, int H, TCell Cell) {
    TDrawBuffer B;
    int i;

    for (i = 0; i < W; i++)
        B[i] = Cell;
    ConPutLine(X, Y, W, H, B);
    return 0;
}

int ConScroll(int Way, int X, int Y, int W, int H, TAttr Fill, int Count) {
    fprintf(stderr, "%s\n", __FUNCTION__);
}

int ConSetSize(int X, int Y) {
    fprintf(stderr, "ConSetSize from (%d, %d) to (%d, %d)\n", ScreenCols, ScreenRows, X, Y);
    unsigned char *NewBuffer;
    unsigned char *p;
    int i;
    int MX, MY;

    p = NewBuffer = (unsigned char *) malloc(X * Y * 2);
    if (NewBuffer == NULL) return -1;
    for (i = 0; i < X * Y; i++) {
        *p++ = ' ';
        *p++ = 0x07;
    }
    MX = ScreenCols;
    if (X < MX)
        MX = X;
    MY = ScreenRows;
    if (Y < MY)
        MY = Y;
    p = NewBuffer;
    for (i = 0; i < MY; i++) {
        memcpy(p, CursorXYPos(0, i), MX * 2);
        p += X * 2;
    }
    free(ScreenBuffer);
    ScreenBuffer = NewBuffer;
    ScreenCols = X;
    ScreenRows = Y;
    //ConPutBox(0, 0, ScreenCols, ScreenRows, (PCell) ScreenBuffer);
    //if (Refresh == 0)
    //    XResizeWindow(display, win, ScreenCols * FontCX, ScreenRows * FontCY);
    return 0;
}

int ConQuerySize(int *X, int *Y) {
    *X = ScreenCols;
    *Y = ScreenRows;
    return 0;
}

int ConSetCursorPos(int X, int Y) {
    DrawCursor(0);
    CursorX = X;
    CursorY = Y;
    DrawCursor(1);
    return 0;
}

int ConQueryCursorPos(int *X, int *Y) {
    *X = CursorX;
    *Y = CursorY;
    return 0;
}

int ConShowCursor(void) {
    CursorVisible = 1;
    DrawCursor(1);
    return 0;
}

int ConHideCursor(void) {
    DrawCursor(0);
    CursorVisible = 0;
    return 0;
}

int ConCursorVisible(void) {
    return 1;
}

void ConSetInsertState(bool insert) {
    insertState = insert;
}

int ConSetCursorSize(int Start, int End) {
    CursorStart = Start;
    CursorEnd = End;
    DrawCursor(CursorVisible);
    return 1;
}

int ConSetMousePos(int /*X*/, int /*Y*/) {
    return 0;
}

static int LastMouseX = -1, LastMouseY = -1;

int ConQueryMousePos(int *X, int *Y) {
    if (X) *X = LastMouseX;
    if (Y) *Y = LastMouseY;
    return 0;
}

int ConShowMouse(void) {
    printf("Show\n");
    return 0;
}

int ConHideMouse(void) {
    printf("Hide\n");
    return 0;
}

int ConMouseVisible(void) {
    return 1;
}

int ConQueryMouseButtons(int *ButtonCount) {
    *ButtonCount = 3;
    return 0;
}

void UpdateWindow(int xx, int yy, int ww, int hh) {
    PCell p;
    int i;

    /* show redrawn area */
    /*
    XFillRectangle(display, win, GCs[14], xx, yy, ww, hh);
    XFlush(display);
    i = XEventsQueued(display, QueuedAfterReading);
    while (i-- > 0) {
    XEvent e;
    XNextEvent(display, &e);
    }
    // sleep(1);*/

    ww /= FontCX;
    ww += 2;
    hh /= FontCY;
    xx /= FontCX;
    yy /= FontCY;

    /*
     * OK for this moment I suggest this method - it works somehow
     * But I suppose the correct solution would meant general rewrite
     * of some basic behavior of FTE editor
     * THIS IS TEMPORAL FIX AND SHOULD BE SOLVED IN GENERAL WAY !
     */
    hh *= 3;
    yy -= hh;
    hh += hh + 2;
    if (yy < 0)
        yy = 0;
    if (xx + ww > (int)ScreenCols) ww = ScreenCols - xx;
    if (yy + hh > (int)ScreenRows) hh = ScreenRows - yy;
    Refresh = 1;
    //frames->Repaint();
    //frames->Update();
    p = (PCell) CursorXYPos(xx, yy);
    for (i = 0; i < hh; i++) {
        ConPutBox(xx, yy + i, ww, 1, p);
        p += ScreenCols;
    }
    //fprintf(stderr, "UPDATE\tx:%3d  y:%3d  w:%3d  h:%3d\n", xx, yy, ww, hh);
    //XFlush(display);
    Refresh = 0;
}

void ResizeWindow(int ww, int hh) {
    int ox = ScreenCols;
    int oy = ScreenRows;
    ww /= FontCX;
    if (ww <= 4) ww = 4;
    hh /= FontCY;
    if (hh <= 2) hh = 2;
    if ((int)ScreenCols != ww || (int)ScreenRows != hh) {
        Refresh = 0;
        ConSetSize(ww, hh);
        Refresh = 1;
        if (ox < (int)ScreenCols)
            UpdateWindow(ox * FontCX, 0,
                         (ScreenCols - ox) * FontCX, ScreenRows * FontCY);
        if (oy < (int)ScreenRows)
            UpdateWindow(0, oy * FontCY,
                         ScreenCols * FontCX, (ScreenRows - oy) * FontCY);
        Refresh = 0;
    }
}

void ConvertKeyToEvent(KeySym key, KeySym key1, char */*keyname*/, char */*keyname1*/, int etype, int state, TEvent *Event) {
}


static TEvent LastMouseEvent = { evNone };

#define TM_DIFF(x,y) ((long)(((long)(x) < (long)(y)) ? ((long)(y) - (long)(x)) : ((long)(x) - (long)(y))))

void ConvertClickToEvent(int type, int xx, int yy, int button, int state, TEvent *Event, Time mtime) {
}

static void FlashCursor() {
    struct timeval tv;
    if (!CursorBlink || gettimeofday(&tv, NULL) != 0)
        return;

    unsigned long OldTime = CursorLastTime;
    CursorLastTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    if (OldTime / CursorFlashInterval != CursorLastTime / CursorFlashInterval)
        DrawCursor(CursorVisible);
}

static TEvent Pending = { evNone };

int ConGetEvent(TEventMask EventMask, TEvent *Event, int WaitTime, int Delete) {
    fprintf(stderr, "%s\n", __FUNCTION__);
    //Event->What = evCommand; Event->Msg.Command = cmResize;

    return 0;
}

int ConPutEvent(TEvent Event) {
    fprintf(stderr, "%s\n", __FUNCTION__);
    Pending = Event;
    return 0;
}

int ConFlush(void) {
    fprintf(stderr, "%s\n", __FUNCTION__);
}

int ConGrabEvents(TEventMask /*EventMask*/) {
    return 0;
}

GUI::GUI(int &argc, char **argv, int XSize, int YSize) {
    if (::ConInit(XSize, YSize) == 0)
        gui = this;
    else
        gui = NULL;
    fArgc = argc;
    fArgv = argv;
}

GUI::~GUI() {
    ::ConDone();
}

int GUI::ConSuspend(void) {
    return ::ConSuspend();
}

int GUI::ConContinue(void) {
    return ::ConContinue();
}

int GUI::ShowEntryScreen() {
    return 1;
}

extern GFrame *frames;
extern void *theGlobalGUI;

int GUI::Run() {
    if (Start(fArgc, fArgv) == 0) {
        doLoop = 1;
        theGlobalGUI = this;
        cacao();
        Stop();
        return 0;
    }
    return 1;
}

int GUI::OpenPipe(char *Command, EModel *notify) {
#ifndef NO_PIPES
    int i;

    for (i = 0; i < MAX_PIPES; i++) {
        if (Pipes[i].used == 0) {
            int pfd[2];

            Pipes[i].id = i;
            Pipes[i].notify = notify;
            Pipes[i].stopped = 1;

            if (pipe((int *)pfd) == -1)
                return -1;

            switch (Pipes[i].pid = fork()) {
            case - 1: /* fail */
                return -1;
            case 0: /* child */
                signal(SIGPIPE, SIG_DFL);
                close(pfd[0]);
                close(0);
                assert(open("/dev/null", O_RDONLY) == 0);
                dup2(pfd[1], 1);
                dup2(pfd[1], 2);
                close(pfd[1]);
                exit(system(Command));
            default:
                close(pfd[1]);
                fcntl(pfd[0], F_SETFL, O_NONBLOCK);
                Pipes[i].fd = pfd[0];
            }
            Pipes[i].used = 1;
            //fprintf(stderr, "Pipe Open: %d\n", i);
            return i;
        }
    }
    return -1;
#else
    return 0;
#endif
}

int GUI::SetPipeView(int id, EModel *notify) {
#ifndef NO_PIPES
    if (id < 0 || id > MAX_PIPES)
        return -1;
    if (Pipes[id].used == 0)
        return -1;
    //fprintf(stderr, "Pipe View: %d %08X\n", id, notify);
    Pipes[id].notify = notify;
#endif
    return 0;
}

int GUI::ReadPipe(int id, void *buffer, int len) {
#ifndef NO_PIPES
    int r;

    if (id < 0 || id > MAX_PIPES)
        return -1;
    if (Pipes[id].used == 0)
        return -1;
    //fprintf(stderr, "Pipe Read: Get %d %d\n", id, len);

    r = read(Pipes[id].fd, buffer, len);
    //fprintf(stderr, "Pipe Read: Got %d %d\n", id, len);
    if (r == 0) {
        close(Pipes[id].fd);
        Pipes[id].fd = -1;
        return -1;
    }
    if (r == -1) {
        Pipes[id].stopped = 1;
        return 0;
    }
    return r;
#else
    return 0;
#endif
}

int GUI::ClosePipe(int id) {
#ifndef NO_PIPES
    int status;

    if (id < 0 || id > MAX_PIPES)
        return -1;
    if (Pipes[id].used == 0)
        return -1;
    if (Pipes[id].fd != -1)
        close(Pipes[id].fd);
    kill(Pipes[id].pid, SIGHUP);
    alarm(2);
    waitpid(Pipes[id].pid, &status, 0);
    alarm(0);
    //fprintf(stderr, "Pipe Close: %d\n", id);
    Pipes[id].used = 0;
    return WEXITSTATUS(status);
#else
    return 0;
#endif
}

int GUI::RunProgram(int mode, char *Command) {
    char Cmd[1024];

    strlcpy(Cmd, XShellCommand, sizeof(Cmd));

    if (*Command == 0)  // empty string = shell
        strlcat(Cmd, " -ls &", sizeof(Cmd));
    else {
        strlcat(Cmd, " -e ", sizeof(Cmd));
        strlcat(Cmd, Command, sizeof(Cmd));
        if (mode == RUN_ASYNC)
            strlcat(Cmd, " &", sizeof(Cmd));
    }
    return system(Cmd);
}

char ConGetDrawChar(int idx) {
    static const char *tab = NULL;

    if (!tab) {
        tab = GetGUICharacters("X11", "\x0D\x0C\x0E\x0B\x12\x19____+>\x1F\x01\x12 ");
    }
    assert(idx >= 0 && idx < (int) strlen(tab));

    return tab[idx];
}

extern TEvent NextEvent;

extern "C" void MyDispatchEvent() {
    GUI *g = (GUI*)theGlobalGUI;
    g->ProcessEvent();

    /* NextEvent.What = evKeyDown;
     NextEvent.Msg.View = frames->Active;
     NextEvent.Key.Code = '\n';
     g->DispatchEvent(frames, NextEvent.Msg.View, NextEvent);*/
}

extern "C" void MyDispatchKeyEvent(char c) {
    GUI *g = (GUI*)theGlobalGUI;
    NextEvent.What = evKeyDown;
    NextEvent.Msg.View = frames->Active;
    NextEvent.Key.Code = c;
    g->ProcessEvent();

}

extern "C" void MyResizeWindow(int x, int y) {
    ResizeWindow(x, y);
    MyDispatchEvent();
}
