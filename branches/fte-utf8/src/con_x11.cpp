/*    con_x11.cpp
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 *    I18N & XMB support added by kabi@fi.muni.cz
 */

#include <string.h>
#include <assert.h>
#include <stdarg.h>
#ifdef WINNT
#include <winsock.h>
#define NO_PIPES
#define NO_SIGNALS
#else
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#endif

#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>
#if defined(AIX)
#include <strings.h>
#include <sys/select.h>
#endif
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xos.h>
#ifdef USE_XTINIT
#include <X11/Intrinsic.h>
#endif
#ifdef HPUX
#include </usr/include/X11R5/X11/HPkeysym.h>
#endif
#include "sysdep.h"
#include "c_config.h"
#include "console.h"
#include "gui.h"

#include "con_i18n.h"
#include "s_files.h"
#include "s_util.h"
#include "s_string.h"

i18n_context_t* i18n_ctx = NULL;

#ifdef WINHCLX
#include <X11/XlibXtra.h>    /* HCL - HCLXlibInit */
#endif

#ifdef CAST_FD_SET_INT
#define FD_SET_CAST() (int *)
#else
#define FD_SET_CAST()
#endif

#define MIN_SCRWIDTH 20
#define MIN_SCRHEIGHT 6

#define MAX_PIPES 40
//#define PIPE_BUFLEN 4096

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
static unsigned int ScreenCols = 80;
static unsigned int ScreenRows = 40;
static unsigned int CursorX = 0;
static unsigned int CursorY = 0;
static int CursorVisible = 1;
static unsigned char *ScreenBuffer = NULL;
static int Refresh = 0;

// res_name can be set with -name switch
static char res_name[20] = "fte";
static char res_class[] = "Fte";

static Display *display;
static Colormap colormap;
static Atom wm_protocols;
static Atom wm_delete_window;
static Atom targets;
static Atom XA_CLIPBOARD = 0;
static Window win;
static Atom selection_buffer;
static XSizeHints sizeHints;
// program now contains both modes if available
// some older Xservers don't like XmbDraw...
static XFontStruct *fontStruct;
#ifdef USE_XMB
static int useXMB = 1; // default is yes
static XFontSet fontSet;
static int FontCYD;
#else
static int useXMB = 0;
#endif
static int FontCX, FontCY;
static XColor Colors[16];
static GC GCs[256];
//static int rc;
static char winTitle[256] = "FTE";
static char winSTitle[256] = "FTE";

static unsigned char* CurSelectionData[3] = {NULL,NULL,NULL};
static int CurSelectionLen[3] = {0,0,0};
static int CurSelectionOwn[3] = {0,0,0};
static Time now;

static Atom GetXClip (int clipboard) {
    if (clipboard==1) {
        return XA_PRIMARY;
    }
    if (clipboard==2) {
        return XA_SECONDARY;
    }
    return XA_CLIPBOARD;
}


static int GetFTEClip (Atom clip) {
    if (clip==XA_CLIPBOARD) {
        return 0;
    }
    if (clip==XA_PRIMARY) {
        return 1;
    }
    if (clip==XA_SECONDARY) {
        return 2;
    }
    return -1;
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
} dcolors[] =
{
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

static void SetColor(int i) {
    assert (0 <= i && i <= 15);
    Colors[i].blue  = (dcolors[i].b << 8) | dcolors[i].b;
    Colors[i].green = (dcolors[i].g << 8) | dcolors[i].g;
    Colors[i].red   = (dcolors[i].r << 8) | dcolors[i].r;
    Colors[i].flags = DoRed | DoGreen | DoBlue;
}

static int InitXColors() {
    int i, j;
    long d = 0x7FFFFFFF, d1;
    XColor clr;
    unsigned long pix;
    int num;
    long d_red, d_green, d_blue;
    long u_red, u_green, u_blue;

    for (i = 0; i < 16; i++) {
        SetColor(i);
        if (XAllocColor(display, colormap, &Colors[i]) == 0) {
            SetColor(i);
            pix = 0xFFFFFFFF;
            num = DisplayCells(display, DefaultScreen(display));
            for (j = 0; j < num; j++) {
                clr.pixel = j;
                XQueryColor(display, colormap, &clr);

                d_red = (clr.red - Colors[i].red) >> 3;
                d_green = (clr.green - Colors[i].green) >> 3;
                d_blue = (clr.blue - Colors[i].blue) >> 3;

                //fprintf(stderr, "%d:%d dr:%d, dg:%d, db:%d\n", i, j, d_red, d_green, d_blue);

                u_red = d_red / 100 * d_red * 3;
                u_green = d_green / 100 * d_green * 4;
                u_blue = d_blue / 100 * d_blue * 2;

                //fprintf(stderr, "%d:%d dr:%u, dg:%u, db:%u\n", i, j, u_red, u_green, u_blue);

                d1 = u_red + u_blue + u_green;

                if (d1 < 0)
                    d1 = -d1;
                if (pix == ~0UL || d1 < d) {
                    pix = j;
                    d = d1;
                }
            }
            if (pix == 0xFFFFFFFF) {
                fprintf(stderr, "Color search failed for #%04X%04X%04X\n",
                        Colors[i].red,
                        Colors[i].green,
                        Colors[i].blue);
            }
            clr.pixel = pix;
            XQueryColor(display, colormap, &clr);
            Colors[i] = clr;
            if (XAllocColor(display, colormap, &Colors[i]) == 0) {
                fprintf(stderr, "Color alloc failed for #%04X%04X%04X\n",
                        Colors[i].red,
                        Colors[i].green,
                        Colors[i].blue);
            }
            /*colormap = XCreateColormap(display, win, DefaultVisual(display, screen), AllocNone);
             for (i = 0; i < 16; i++) {
             SetColor(i);
             XAllocColor(display, colormap, &Colors[i]);
             }
             XSetWindowColormap(display, win, colormap);
             return 0;*/
        }
    }
    return 0;
}

static int InitXGCs()
{
    unsigned int i;
    unsigned long mask = GCForeground | GCBackground;
    XGCValues gcv;

    if (!useXMB) {
        gcv.font = fontStruct->fid;
        mask |= GCFont;
    }

    for (i = 0; i < 256; i++) {
        gcv.foreground = Colors[i % 16].pixel;
        gcv.background = Colors[(i / 16)].pixel;
        GCs[i] = XCreateGC(display, win, mask, &gcv);
    }

    return 0;
}

#ifdef USE_XMB
static void try_fontset_load(const char *fs)
{
    char *def = NULL;
    char **miss = NULL;
    int nMiss = 0;

    if (fontSet)
	return;

    if (!fs || !*fs)
	return;

    fontSet = XCreateFontSet(display, fs, &miss, &nMiss,
			     &def);

    if (fontSet == NULL) {
	fprintf(stderr, "XFTE Warning: unable to open font \"%s\":\n"
		" Missing count: %d\n", fs, nMiss);
	for(int i = 0; i < nMiss; i++)
	    fprintf(stderr, "  %s\n", miss[i]);
	if (def != NULL)
	    fprintf(stderr, " def_ret: %s\n", def);
    }
}
#endif

static int InitXFonts(void)
{
    char *fs;

    fs = getenv("VIOFONT");
    if (fs == NULL && WindowFont[0] != 0)
        fs = WindowFont;

    if (!useXMB) {

        fontStruct = NULL;

	if (fs != NULL) {
	    char *s = 0;

	    s = strchr(fs, ',');
	    if (s != NULL)
		*s = 0;
	    fontStruct = XLoadQueryFont(display, fs);
	}
        if (fontStruct == NULL)
            fontStruct = XLoadQueryFont(display, "8x13");
        if (fontStruct == NULL)
            fontStruct = XLoadQueryFont(display, "fixed");
        if (fontStruct == NULL)
            return -1;
        FontCX = fontStruct->max_bounds.width;
        FontCY = fontStruct->max_bounds.ascent + fontStruct->max_bounds.descent;
    }
#ifdef USE_XMB
    else {
	try_fontset_load(getenv("VIOFONT"));
	try_fontset_load(WindowFont);
	try_fontset_load("-misc-*-r-normal-*");
	try_fontset_load("*fixed*");

	if (fontSet == NULL)
	    return -1;

        XFontSetExtents *xE = XExtentsOfFontSet(fontSet);

        FontCX = xE->max_logical_extent.width;
        FontCY = xE->max_logical_extent.height;
        // handle descending (comes in negative form)
        FontCYD = -(xE->max_logical_extent.y);
        // printf("Font X:%d\tY:%d\tD:%d\n", FontCX, FontCY, FontCYD);
    }
#endif
    return 0;
}

static int SetupXWindow(int argc, char **argv)
{
    unsigned long mask;
    XSetWindowAttributes setWindowAttributes;

#ifdef WINHCLX
    HCLXlibInit(); /* HCL - Initialize the X DLL */
#endif

#ifdef USE_XTINIT
    XtAppContext  	app_context;
    XtToolkitInitialize();
    app_context = XtCreateApplicationContext();
    if (( display = XtOpenDisplay(app_context, NULL, argv[0], "xfte",
                            NULL, 0, &argc, argv)) == NULL)
       DieError(1, "%s:  Can't open display\n", argv[0]);
#else
    char *ds;
    if ((ds = getenv("DISPLAY")) == NULL)
	DieError(1, "$DISPLAY not set? This version of fte must be run under X11.");
    if ((display = XOpenDisplay(ds)) == NULL)
	DieError(1, "XFTE Fatal: could not open display: %s!", ds);
#endif

    colormap = DefaultColormap(display, DefaultScreen(display));

    setWindowAttributes.bit_gravity =
        sizeHints.win_gravity = NorthWestGravity;

    // this is correct behavior
    if (initX < 0)
        initX = DisplayWidth(display, DefaultScreen(display)) + initX;
    if (initY < 0)
        initY = DisplayHeight(display, DefaultScreen(display)) + initY;
    win = XCreateWindow(display,
                        DefaultRootWindow(display),
                        initX, initY,
                        // ScreenCols * FontCX, ScreenRows * FontCY, 0,
                        // at this moment we don't know the exact size
                        // but we need to open a window - so pick up 1 x 1
                        1, 1, 0,
                        CopyFromParent, InputOutput, CopyFromParent,
                        CWBitGravity, &setWindowAttributes);

    i18n_ctx = i18n_open(display, win, &mask);

    if (InitXFonts() != 0)
	DieError(1, "XFTE Fatal: could not open any font!");

    /* >KeyReleaseMask shouldn't be set for correct key mapping */
    /* we set it anyway, but not pass to XmbLookupString -- mark */
    mask |= ExposureMask | StructureNotifyMask | VisibilityChangeMask |
        FocusChangeMask | KeyPressMask | KeyReleaseMask |
        ButtonPressMask | ButtonReleaseMask | ButtonMotionMask;
    XSelectInput(display, win, mask);

    wm_protocols = XInternAtom(display, "WM_PROTOCOLS", False);
    assert(wm_protocols != None);
    wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
    assert(wm_delete_window != None);
    selection_buffer = XInternAtom(display, "fte_clip", False);//??? needed
    assert(selection_buffer != None);
    targets = XInternAtom(display, "TARGETS", False);
    assert(targets != None);
    XA_CLIPBOARD = XInternAtom(display, "CLIPBOARD", False);
    assert(XA_CLIPBOARD != None);

    sizeHints.flags = PResizeInc | PMinSize | PBaseSize | PWinGravity;
    sizeHints.width_inc = FontCX;
    sizeHints.height_inc = FontCY;
    sizeHints.min_width = MIN_SCRWIDTH * FontCX;
    sizeHints.min_height = MIN_SCRHEIGHT * FontCY;
    sizeHints.base_width = 0;
    sizeHints.base_height = 0;
    if (setUserPosition)
        sizeHints.flags |= USPosition;

    XClassHint classHints;
    classHints.res_name = res_name;
    classHints.res_class = res_class;
    XSetClassHint(display, win, &classHints);

    XSetStandardProperties(display, win, winTitle, winTitle, 0, NULL, 0, 0);
    XSetWMNormalHints(display, win, &sizeHints);
    XSetWMProtocols(display, win, &wm_delete_window, 1);

    if (InitXColors() != 0) return -1;
    if (InitXGCs() != 0) return -1;

    XResizeWindow(display, win, ScreenCols * FontCX, ScreenRows * FontCY);
    XMapRaised(display, win);
    //    XClearWindow(display, win); /// !!! why?
    return 0;
}

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
    char buf[sizeof(winTitle)] = {0};
    JustFileName(Title, buf, sizeof(buf));
    if (buf[0] == '\0') // if there is no filename, try the directory name.
        JustLastDirectory(Title, buf, sizeof(buf));

    strncpy(winTitle, "FTE - ", sizeof(winTitle) - 1);
    if (buf[0] != 0) // if there is a file/dir name, stick it in here.
    {
        strncat(winTitle, buf, sizeof(winTitle) - 1 - strlen(winTitle));
        strncat(winTitle, " - ", sizeof(winTitle) - 1 - strlen(winTitle));
    }
    strncat(winTitle, Title, sizeof(winTitle) - 1 - strlen(winTitle));
    winTitle[sizeof(winTitle) - 1] = 0;
    strncpy(winSTitle, STitle, sizeof(winSTitle) - 1);
    winSTitle[sizeof(winSTitle) - 1] = 0;
    XSetStandardProperties(display, win, winTitle, winSTitle, 0, NULL, 0, NULL);
    return 0;
}

int ConGetTitle(char *Title, int MaxLen, char *STitle, int SMaxLen) {
    strlcpy(Title, winTitle, MaxLen);
    strlcpy(STitle, winSTitle, SMaxLen);
    return 0;
}

#define InRange(x,a,y) (((x) <= (a)) && ((a) < (y)))
#define CursorXYPos(x,y) (ScreenBuffer + ((x) + ((y) * ScreenCols)) * 2)

void DrawCursor(int Show) {
    if (CursorVisible) {
        unsigned char *p = CursorXYPos(CursorX, CursorY), attr;
        attr = p[1];
        /*if (Show) attr = ((((attr << 4) & 0xF0)) | (attr >> 4)) ^ 0x77;*/
        if (Show)
            attr = (attr ^ 0x77);

        if (!useXMB)
            XDrawImageString(display, win, GCs[((unsigned)attr) & 0xFF],
                             CursorX * FontCX,
                             fontStruct->max_bounds.ascent + CursorY * FontCY,
                             (char *)p, 1);
#ifdef USE_XMB
        else
            XmbDrawImageString(display, win, fontSet,
                               GCs[((unsigned)attr) & 0xFF],
                               CursorX * FontCX, FontCYD + CursorY * FontCY,
                               (char *)p, 1);
#endif
    }
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
                while ((len > 0) && c[0] == ps[0] && c[1] == ps[1] )
                {
                    ps+=2;
                    c+=2;
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
            temp[0] = *ps++; attr = *ps++;
            while ((l < len) && ((unsigned char) (ps[1]) == attr)) {
                temp[l++] = *ps++;
                ps++;
	    }
	    if (!useXMB)
                XDrawImageString(display, win, GCs[((unsigned)attr) & 0xFF],
                                 x * FontCX, fontStruct->max_bounds.ascent +
                                 (Y + i) * FontCY,
                                 (char *)temp, l);
#ifdef USE_XMB
            else
                XmbDrawImageString(display, win, fontSet,
                                   GCs[((unsigned)attr) & 0xFF],
                                   x * FontCX, FontCYD + (Y + i) * FontCY,
                                   (char *)temp, l);
#endif
	    //temp[l] = 0; printf("%s\n", temp);
            len -= l;
            x += l;
	}
/*	if (x < ScreenCols - 1) {
	    printf("XX %d   %d   %d\n", X, x, W);
	    XFillRectangle(display, win, GCs[15 * 16 + 7],
			   x * FontCX, (Y + i) * FontCY,
			   (ScreenCols - x - 1) * FontCX, FontCY);
	}
*/        p = CursorXYPos(X, Y + i);
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
    TCell Cell;
    int l;

    MoveCh(&Cell, ' ', Fill, 1);
    DrawCursor(0);
    if (Way == csUp) {
	XCopyArea(display, win, win, GCs[0],
		  X * FontCX,
                  (Y + Count) * FontCY,
                  W * FontCX,
                  (H - Count) * FontCY,
		  X * FontCX,
                  Y * FontCY);
	for (l = 0; l < H - Count; l++)
	    memcpy(CursorXYPos(X, Y + l), CursorXYPos(X, Y + l + Count), 2 * W);

	if (ConSetBox(X, Y + l, W, Count, Cell) == -1)
	    return -1;
    } else if (Way == csDown) {
        XCopyArea(display, win, win, GCs[0],
                  X * FontCX,
                  Y * FontCY,
                  W * FontCX,
                  (H - Count) * FontCY,
                  X * FontCX,
                  (Y + Count) * FontCY);
	for (l = H - 1; l >= Count; l--)
            memcpy(CursorXYPos(X, Y + l), CursorXYPos(X, Y + l - Count), 2 * W);

	if (ConSetBox(X, Y, W, Count, Cell) == -1)
	    return -1;
    }
    DrawCursor(1);
    return 0;
}

int ConSetSize(int X, int Y) {
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
int ConSetCursorSize(int /*Start*/, int /*End*/) {
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

    ww /= FontCX; ww += 2;
    hh /= FontCY;
    xx /= FontCX;
    yy /= FontCY;

    /*
     * OK for this moment I suggest this method - it works somehow
     * But I suppose the correct solution would meant general rewrite
     * of some basic behavior of FTE editor
     * THIS IS TEMPORAL FIX AND SHOULD BE SOLVED IN GENERAL WAY !
     */
    hh *= 3; yy -= hh; hh += hh + 2;
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
    ww /= FontCX; if (ww <= 4) ww = 4;
    hh /= FontCY; if (hh <= 2) hh = 2;
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

static struct {
    long keysym;
    long keycode;
} key_table[] = {
    { XK_Escape,         kbEsc },
    { XK_Tab,            kbTab },
    { XK_Return,         kbEnter },
    { XK_Pause,          kbPause },
    { XK_BackSpace,      kbBackSp },
    { XK_Home,           kbHome },
    { XK_Up,             kbUp },
    { XK_Prior,          kbPgUp },
    { XK_Left,           kbLeft },
    { XK_Right,          kbRight },
    { XK_End,            kbEnd },
    { XK_Down,           kbDown },
    { XK_Next,           kbPgDn },
    { XK_Select,         kbEnd },
    { XK_KP_Enter,       kbEnter | kfGray },
    { XK_Insert,         kbIns | kfGray },
    { XK_Delete,         kbDel | kfGray },
    { XK_KP_Add,         '+' | kfGray },
    { XK_KP_Subtract,    '-' | kfGray },
    { XK_KP_Multiply,    '*' | kfGray },
    { XK_KP_Divide,      '/' | kfGray },
    { XK_KP_Begin,       kbPgUp | kfGray | kfCtrl },
    { XK_KP_Home,        kbHome | kfGray },
    { XK_KP_Up,          kbUp | kfGray },
    { XK_KP_Prior,       kbPgUp | kfGray },
    { XK_KP_Left,        kbLeft | kfGray },
    { XK_KP_Right,       kbRight | kfGray },
    { XK_KP_End,         kbEnd | kfGray },
    { XK_KP_Down,        kbDown | kfGray },
    { XK_KP_Next,        kbPgDn| kfGray },
    { XK_Num_Lock,       kbNumLock },
    { XK_Caps_Lock,      kbCapsLock },
    { XK_Print,          kbPrtScr },
    { XK_Shift_L,        kbShift },
    { XK_Shift_R,        kbShift | kfGray },
    { XK_Control_L,      kbCtrl },
    { XK_Control_R,      kbCtrl | kfGray },
    { XK_Alt_L,          kbAlt },
    { XK_Alt_R,          kbAlt | kfGray },
    { XK_Meta_L,         kbAlt },
    { XK_Meta_R,         kbAlt | kfGray },
    { XK_F1,             kbF1 },
    { XK_F2,             kbF2 },
    { XK_F3,             kbF3 },
    { XK_F4,             kbF4 },
    { XK_F5,             kbF5 },
    { XK_F6,             kbF6 },
    { XK_F7,             kbF7 },
    { XK_F8,             kbF8 },
    { XK_F9,             kbF9 },
    { XK_F10,            kbF10 },
    { XK_F11,            kbF11 },
    { XK_F12,            kbF12 },
    { XK_KP_0,           '0' | kfGray },
    { XK_KP_1,           '1' | kfGray },
    { XK_KP_2,           '2' | kfGray },
    { XK_KP_3,           '3' | kfGray },
    { XK_KP_4,           '4' | kfGray },
    { XK_KP_5,           '5' | kfGray },
    { XK_KP_6,           '6' | kfGray },
    { XK_KP_7,           '7' | kfGray },
    { XK_KP_8,           '8' | kfGray },
    { XK_KP_9,           '9' | kfGray },
    { XK_KP_Decimal,     '.' | kfGray },
    { 0x1000FF6F,        kbDel | kfShift | kfGray },
    { 0x1000FF70,        kbIns | kfCtrl | kfGray },
    { 0x1000FF71,        kbIns | kfShift | kfGray },
    { 0x1000FF72,        kbIns | kfGray },
    { 0x1000FF73,        kbDel | kfGray },
    { 0x1000FF74,        kbTab | kfShift },
    { 0x1000FF75,        kbTab | kfShift },
    { 0,                 0 }
};

void ConvertKeyToEvent(KeySym key, KeySym key1, char */*keyname*/, char */*keyname1*/, int etype, int state, TEvent *Event) {
    unsigned int myState = 0;

    Event->What = evNone;

    switch (etype) {
    case KeyPress:   Event->What = evKeyDown; break;
    case KeyRelease: Event->What = evKeyUp; break;
    default:
        return ;
    }

    if (state & ShiftMask) myState |= kfShift;
    if (state & ControlMask) myState |= kfCtrl;
    if (state & Mod1Mask) myState |= kfAlt;

    /* modified kabi@fi.muni.cz
     * for old method
     * if (!KeyAnalyze((etype == KeyPress), state, &key, &key1))
     *     return;
     */

    //printf("key: %d ; %d ; %d\n", key, key1, state);
    if (key < 256 || (key1 < 256 && (myState == kfAlt || myState == (kfAlt | kfShift)))) {
        if (myState & kfAlt)
            key = key1;
        if (myState == kfShift)
            myState = 0;
        if (myState & (kfAlt | kfCtrl))
            if ((key >= 'a') && (key < 'z' + 32))
                key &= ~0x20;
        if ((myState & kfCtrl) && key < 32)
            key += 64;
        Event->Key.Code = key | myState;
        return;
    } else {
        for (unsigned i = 0; i < (sizeof(key_table) / sizeof(key_table[0])); i++) {
            long k;

            if ((long) key1 == key_table[i].keysym) {
                k = key_table[i].keycode;
                if (k < 256)
                    if (myState == kfShift)
                        myState = 0;
                Event->Key.Code = k | myState;
                return;
            }
        }
    }
    //printf("Unknown key: %ld %s %d %d\n", key, keyname, etype, state);
    Event->What = evNone;
}


static TEvent LastMouseEvent = { evNone };

#define TM_DIFF(x,y) ((long)(((long)(x) < (long)(y)) ? ((long)(y) - (long)(x)) : ((long)(x) - (long)(y))))

void ConvertClickToEvent(int type, int xx, int yy, int button, int state, TEvent *Event, Time mtime) {
    unsigned int myState = 0;
    static unsigned long LastClickTime = 0;
    static short LastClickCount = 0;
    static unsigned long LastClick = 0;
    unsigned long CurTime = mtime;

    if (type == MotionNotify) Event->What = evMouseMove;
    else if (type == ButtonPress) Event->What = evMouseDown;
    else Event->What = evMouseUp;
    Event->Mouse.X = xx / FontCX;
    Event->Mouse.Y = yy / FontCY;
    if (Event->What == evMouseMove)
	if (LastMouseX == Event->Mouse.X
	    && LastMouseY == Event->Mouse.Y) {
            Event->What = evNone;
            return;
        }
    LastMouseX = Event->Mouse.X;
    LastMouseY = Event->Mouse.Y;
    Event->Mouse.Buttons = 0;
    if (type == MotionNotify) {
        if (state & Button1Mask) Event->Mouse.Buttons |= 1;
        if (state & Button2Mask) Event->Mouse.Buttons |= 4;
        if (state & Button3Mask) Event->Mouse.Buttons |= 2;
    } else {
        switch (button) {
        case Button1: Event->Mouse.Buttons |= 1; break;
        case Button2: Event->Mouse.Buttons |= 4; break;
        case Button3: Event->Mouse.Buttons |= 2; break;
        case Button4:
        case Button5:
            if (type == ButtonPress) {
                Event->What = evCommand;
                if (state & ShiftMask) {
                    Event->Msg.Param1 = 1;

                    if (button == Button4)
                        Event->Msg.Command = cmVScrollUp; // fix core to use count
                    else
                        Event->Msg.Command = cmVScrollDown;
                }
                else
                {
                    Event->Msg.Param1 = 3;
                    if (button == Button4)
                        Event->Msg.Command = cmVScrollUp;
                    else
                        Event->Msg.Command = cmVScrollDown;
                }
            }
            return ;
        }
    }
    Event->Mouse.Count = 1;
    if (state & ShiftMask) myState |= kfShift;
    if (state & ControlMask) myState |= kfCtrl;
    if (state & Mod1Mask) myState |= kfAlt;
//    if (state & Mod2Mask) myState |= kfAlt;
//    if (state & Mod3Mask) myState |= kfAlt;
//    if (state & Mod4Mask) myState |= kfAlt;
    Event->Mouse.KeyMask = myState;

    if (Event->What == evMouseDown) {
        if (LastClickCount) {
            if (LastClick == Event->Mouse.Buttons) {
                if (TM_DIFF(CurTime, LastClickTime) <= MouseMultiClick) {
                    Event->Mouse.Count = ++LastClickCount;
                } else {
                    LastClickCount = 0;
                }
            } else {
                LastClick = 0;
                LastClickCount = 0;
                LastClickTime = 0;
            }
        }

        LastClick = Event->Mouse.Buttons;
        if (LastClickCount == 0)
            LastClickCount = 1;
        LastClickTime = CurTime;
    }
    /*    if (Event->What == evMouseMove) {
     LastClick = 0;
     LastClickCount = 0;
     LastClickTime = 0;
     }
     */
    LastMouseEvent = *Event;
}

void ProcessXEvents(TEvent *Event) {
    XEvent event;
    XAnyEvent *anyEvent = (XAnyEvent *) &event;
    XExposeEvent *exposeEvent = (XExposeEvent *) &event;
    XButtonEvent *buttonEvent = (XButtonEvent *) &event;
    XKeyEvent *keyEvent = (XKeyEvent *) &event;
    XKeyEvent keyEvent1;
    XConfigureEvent *configureEvent = (XConfigureEvent *) &event;
    XGraphicsExposeEvent *gexposeEvent = (XGraphicsExposeEvent *) &event;
    XMotionEvent *motionEvent = (XMotionEvent *) &event;
    KeySym key, key1;
    int state;
    char keyName[32];
    char keyName1[32];

    memset((void *)&event, 0, sizeof(event));
    Event->What = evNone;
#ifdef WINNT
    //int rc = -1;
#else
    //int rc =
#endif
        XNextEvent(display, &event);

    if (XFilterEvent(&event, None))
        return;

    if (event.type == MappingNotify) {
        XRefreshKeyboardMapping(&event.xmapping);
        return;
    }

    if (anyEvent->window != win)
        return;

    switch (event.type) {
    case Expose:
	//fprintf(stderr, "EXPOSE\tx:%3d  y:%3d  w:%3d  h:%3d\n",
	//	exposeEvent->x, exposeEvent->y,
	//	exposeEvent->width, exposeEvent->height);
        UpdateWindow(exposeEvent->x,
                     exposeEvent->y,
                     exposeEvent->width,
                     exposeEvent->height);
        break;
    case GraphicsExpose:
        /* catch up same events to speed up this a bit */
	state = XEventsQueued(display, QueuedAfterReading);
        // printf("Event %d\n", state);
	while (state-- > 0) {
	    XEvent e;
	    XGraphicsExposeEvent *ge = (XGraphicsExposeEvent *) &e;

	    if (XCheckTypedWindowEvent(display, win, GraphicsExpose, &e)) {
		if (gexposeEvent->x == ge->x
		    && gexposeEvent->y == ge->y
		    && gexposeEvent->width == ge->width
		    && gexposeEvent->height == ge->height) {
		    // fprintf(stderr, "found the same gexpose event\n");
		    continue;
		} else {
		    // fprintf(stderr, "caught different gexpose event\n");
		    XPutBackEvent(display, &e);
		}
	    }
	    break;
	}
	//fprintf(stderr, "GEXPOSE\tx:%3d  y:%3d  w:%3d  h:%3d\n",
	//	gexposeEvent->x, gexposeEvent->y,
	//	gexposeEvent->width, gexposeEvent->height);
        UpdateWindow(gexposeEvent->x,
                     gexposeEvent->y,
                     gexposeEvent->width,
                     gexposeEvent->height);
        break;
    case ConfigureNotify:
        while ((XPending(display) > 0) &&
               XCheckTypedWindowEvent(display, win,
                                      ConfigureNotify, &event))
            XSync(display, 0);
        ResizeWindow(configureEvent->width, configureEvent->height);
        Event->What = evCommand;
        Event->Msg.Command = cmResize;
        break;
    case ButtonPress:
    case ButtonRelease:
        now = event.xbutton.time;
        ConvertClickToEvent(event.type, buttonEvent->x, buttonEvent->y, buttonEvent->button, buttonEvent->state, Event, motionEvent->time);
        break;
    case FocusIn:
        i18n_focus_in(i18n_ctx);
        break;
    case FocusOut:
        i18n_focus_out(i18n_ctx);
        break;
    case KeyPress:
        // case KeyRelease:
        now = event.xkey.time;
        state = keyEvent->state;
        keyEvent1 = *keyEvent;
        keyEvent1.state &= ~(ShiftMask | ControlMask | Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask);

        if (event.type == KeyRelease)
            XLookupString(keyEvent, keyName, sizeof(keyName), &key, 0);
        else {
	    i18n_lookup_sym(keyEvent, keyName, sizeof(keyName), &key, i18n_ctx->xic);
            if (!key)
                break;
        }
        XLookupString(&keyEvent1, keyName1, sizeof(keyName1), &key1, 0);
        //printf("keyEvent->state = %d %s %08X\n", keyEvent->state, keyName, key);
        //printf("keyEvent1.state = %d %s %08X\n", keyEvent1.state, keyName1, key1);
        //key1 = XLookupKeysym(keyEvent, 0);
        ConvertKeyToEvent(key, key1, keyName, keyName1, event.type, state, Event);
        break;
    case MotionNotify:
        now = event.xmotion.time;
        ConvertClickToEvent(event.type, motionEvent->x, motionEvent->y, 0, motionEvent->state, Event, motionEvent->time);
        break;
    case ClientMessage:
        if (event.xclient.message_type == wm_protocols
            && event.xclient.format == 32
            && (Atom)event.xclient.data.l[0] == wm_delete_window)
        {
            Event->What = evCommand;
            Event->Msg.Command = cmClose;
        }
        break;
    case SelectionClear:
        {
            for (int i=0; i<3; i++) {
                Window owner = XGetSelectionOwner(display, GetXClip(i));
                if (owner != win) {
                    if (CurSelectionData[i] != NULL)
                        free(CurSelectionData[i]);
                    CurSelectionData[i] = NULL;
                    CurSelectionLen[i] = 0;
                    CurSelectionOwn[i] = 0;
                }
            }
        }
        break;
    case SelectionRequest:
        {
            XEvent notify;
            unsigned int clip = GetFTEClip(event.xselectionrequest.selection);

            notify.type = SelectionNotify;
            notify.xselection.requestor = event.xselectionrequest.requestor;
            notify.xselection.selection = event.xselectionrequest.selection;
            notify.xselection.target = event.xselectionrequest.target;
            notify.xselection.time = event.xselectionrequest.time;

            if (clip<=2 && event.xselectionrequest.target == XA_STRING)
	    {
                static unsigned char empty[] = "";
                XChangeProperty(display,
                                event.xselectionrequest.requestor,
                                event.xselectionrequest.property,
                                event.xselectionrequest.target,
                                8, PropModeReplace,
                                (CurSelectionData[clip] ? CurSelectionData[clip] : empty),
                                CurSelectionLen[clip]);
                notify.xselection.property = event.xselectionrequest.property;
            } else if (clip<=2 && event.xselectionrequest.target == targets)
            {
                Atom type = XA_STRING;

                XChangeProperty(display,
                                event.xselectionrequest.requestor,
                                event.xselectionrequest.property,
                                event.xselectionrequest.target,
                                32, PropModeReplace,
                                (unsigned char *)&type, 1);
                notify.xselection.property = event.xselectionrequest.property;
            } else {
                /*fprintf(stderr,
                 "selection request %ld prop: %ld target: %ld requestor=%lX\n",
                 event.xselectionrequest.selection,
                 event.xselectionrequest.property,
                 event.xselectionrequest.target,
                 event.xselectionrequest.requestor);*/

                notify.xselection.property = None;
            }

            XSendEvent(display, notify.xselection.requestor, False, 0L, &notify);
        }
        break;
    }
}

static TEvent Pending = { evNone };

int ConGetEvent(TEventMask EventMask, TEvent *Event, int WaitTime, int Delete) {
    fd_set read_fds;
    struct timeval timeout;
    int rc;
    static TEvent Queued = { evNone };

    Event->What = evNone;
    if (Queued.What != evNone) {
        *Event = Queued;
        if (Delete) Queued.What = evNone;
        if (Event->What & EventMask) return 0;
        else Queued.What = evNone;
    }

    Event->What = evNone;
    if (Pending.What != evNone) {
        *Event = Pending;
        if (Delete) Pending.What = evNone;
        if (Event->What & EventMask) return 0;
        else
            Pending.What = evNone;
    }

    Event->What = evNone;
    while (Event->What == evNone) {
        Event->What = evNone;
        while (XPending(display) > 0) {
            ProcessXEvents(Event);
            if (Event->What != evNone) {
                while ((Event->What == evMouseMove) && (Queued.What == evNone)) {
                    while ((rc = XPending(display)) > 0) {
                        ProcessXEvents(&Queued);
                        if (Queued.What == evMouseMove) {
                            *Event = Queued;
                            Queued.What = evNone;
                        } else break;
                    }
                    if (rc <= 0) break;
                }
            }
            if (Delete == 0)
                Pending = *Event;
            if (Event->What & EventMask) return 0;
            else
                Pending.What = evNone;
            Event->What = evNone;
        }

        Event->What = evNone;
        FD_ZERO(&read_fds);
        FD_SET(ConnectionNumber(display), &read_fds);
        for (int p = 0; p < MAX_PIPES; p++)
            if (Pipes[p].used)
                if (Pipes[p].fd != -1)
                    FD_SET(Pipes[p].fd, &read_fds);

        if ((WaitTime == -1 || WaitTime > MouseAutoDelay) && (LastMouseEvent.What == evMouseAuto) && (EventMask & evMouse)) {
            timeout.tv_sec = 0;
            timeout.tv_usec = MouseAutoDelay * 1000;
            rc = select(sizeof(fd_set) * 8,
                        FD_SET_CAST() &read_fds, NULL, NULL,
                        &timeout);
            if (rc == 0) {
                *Event = LastMouseEvent;
                return 0;
            }
        } else if ((WaitTime == -1 || WaitTime > MouseAutoRepeat) && (LastMouseEvent.What == evMouseDown || LastMouseEvent.What == evMouseMove)
                   &&
                   (LastMouseEvent.Mouse.Buttons) && (EventMask & evMouse))
        {
            timeout.tv_sec = 0;
            timeout.tv_usec = MouseAutoRepeat * 1000;
            rc = select(sizeof(fd_set) * 8,
                        FD_SET_CAST() &read_fds, NULL, NULL,
                        &timeout);
            if (rc == 0) {
                LastMouseEvent.What = evMouseAuto;
                *Event = LastMouseEvent;
                return 0;
            }
        } else if (WaitTime == -1) {
            rc = select(sizeof(fd_set) * sizeof(char),
                        FD_SET_CAST() &read_fds, NULL, NULL,
                        NULL);
        } else {
            timeout.tv_sec = 0;
            timeout.tv_usec = WaitTime * 1000 + 1;
            rc = select(sizeof(fd_set) * sizeof(char),
                        FD_SET_CAST() &read_fds, NULL, NULL,
                        &timeout);
        }

        if (rc == 0 || rc == -1) {
            Event->What = evNone;
            return -1;
        }
        if (FD_ISSET(ConnectionNumber(display), &read_fds)) // X has priority
            continue;
        for (int pp = 0; pp < MAX_PIPES; pp++) {
            if (Pipes[pp].used)
                if (Pipes[pp].fd != -1)
                    if (FD_ISSET(Pipes[pp].fd, &read_fds)) {
                        if (Pipes[pp].notify) {
                            Event->What = evNotify;
                            Event->Msg.View = 0;
                            Event->Msg.Model = Pipes[pp].notify;
                            Event->Msg.Command = cmPipeRead;
                            Event->Msg.Param1 = pp;
                            Pipes[pp].stopped = 0;
                        }
                        //fprintf(stderr, "Pipe %d\n", Pipes[pp].fd);
                        return 0;
                    }
        }
    }
    return 0;
}

int ConPutEvent(TEvent Event) {
    Pending = Event;
    return 0;
}

int ConFlush(void) {
    XFlush(display);
    return 0;
}

int ConGrabEvents(TEventMask /*EventMask*/) {
    return 0;
}

int GetXSelection(int *len, char **data, int clipboard) {
    if (CurSelectionOwn[clipboard]) {
        *data = (char *) malloc(CurSelectionLen[clipboard]);
        if (*data == 0)
            return -1;
        memcpy(*data, CurSelectionData[clipboard], CurSelectionLen[clipboard]);
        *len = CurSelectionLen[clipboard];
        return 0;
    } else {
        Atom clip = GetXClip(clipboard);
        if (XGetSelectionOwner(display, clip) != None) {
            XEvent event;
            Atom type;
            long extra;
            int i;
            long l;
            time_t time_started;

            assert(selection_buffer != None);

            XConvertSelection(display, clip, XA_STRING,
                              selection_buffer, win, now);

            time_started = time(NULL);

            for (;;) {
                if (XCheckTypedWindowEvent(display,
                                           win,
                                           SelectionNotify,
                                           &event))
                    break;

                time_t tnow = time(NULL);

                if (time_started > tnow)
                    time_started = tnow;

                if (tnow - time_started > 5000)
                    return -1;
            }

            /*do
             {
             XNextEvent(display, &event);
             } while (event.type != SelectionNotify &&
             event.xselection.property != None &&
             event.type != ButtonPress);*/

	    if (event.type == SelectionNotify
		&& event.xselection.property != None) {
                XGetWindowProperty(display,
                                   event.xselection.requestor,
                                   event.xselection.property,
                                   0L, 0x10000, True,
                                   event.xselection.target, &type, &i,
                                   (unsigned long *)&l,
                                   (unsigned long *)&extra,
                                   (unsigned char **)data);

                *len = l;
                return 0;
            }
            return -1;
        }
    }
    *data = XFetchBytes(display, len);
    if (*data == 0)
        return -1;
    else
        return 0;
}

int SetXSelection(int len, char *data, int clipboard) {
    Atom clip = GetXClip(clipboard);
    if (CurSelectionData[clipboard] != NULL)
        free(CurSelectionData[clipboard]);

    CurSelectionData[clipboard] = (unsigned char *)malloc(len);
    if (CurSelectionData[clipboard] == NULL) {
        CurSelectionLen[clipboard] = 0;
        return -1;
    }
    CurSelectionLen[clipboard] = len;
    memcpy(CurSelectionData[clipboard], data, CurSelectionLen[clipboard]);
    if (CurSelectionLen[clipboard] < 64 * 1024) {
        XStoreBytes(display, data, len);
        XSetSelectionOwner(display, clip, win, CurrentTime);
        if (XGetSelectionOwner(display, clip) == win)
            CurSelectionOwn[clipboard] = 1;
    } else {
        XSetSelectionOwner(display, clip, None, CurrentTime);
    }
    return 0;
}

GUI::GUI(int &argc, char **argv, int XSize, int YSize) {
    int o = 1;

    for (int c = 1; c < argc; c++) {
        if (strcmp(argv[c], "-font") == 0) {
            if (c + 1 < argc) {
                strcpy(WindowFont, argv[++c]);
            }
        } else if (strcmp(argv[c], "-geometry") == 0) {
            if (c + 1 < argc) {

                XParseGeometry(argv[++c], &initX, &initY,
                               &ScreenCols, &ScreenRows);
                if (ScreenCols > 255)
                    ScreenCols = 255;
                else if (ScreenCols < MIN_SCRWIDTH)
                    ScreenCols = MIN_SCRWIDTH;
                if (ScreenRows > 255)
                    ScreenRows = 255;
                else if (ScreenRows < MIN_SCRHEIGHT)
                    ScreenRows = MIN_SCRHEIGHT;
                setUserPosition = 1;
            }
        } else if ((strcmp(argv[c], "-noxmb") == 0)
                   || (strcmp(argv[c], "--noxmb") == 0))
            useXMB = 0;
        else if (strcmp(argv[c], "-name") == 0) {
            if (c + 1 < argc) {
                strncpy (res_name, argv [++c], sizeof (res_name));
                res_name[sizeof(res_name)-1] = '\0'; // ensure termination
	    }
        } else
            argv[o++] = argv[c];
    }
    argc = o;
    argv[argc] = 0;

    if (::ConInit(XSize, YSize) == 0
        && SetupXWindow(argc, argv) == 0)
        gui = this;
    else
        gui = NULL;
    fArgc = argc;
    fArgv = argv;
}

GUI::~GUI() {

    for (int i = 0; i < 256; i++)
	XFreeGC(display, GCs[i]);

    XDestroyWindow(display, win);
    XCloseDisplay(display);

    for (int i=0; i<3; i++) {
        if (CurSelectionData[i] != NULL) {
            free(CurSelectionData[i]);
        }
    }

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
            case -1: /* fail */
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
    static const char *tab=NULL;

    if (!tab) {
        tab=GetGUICharacters ("X11","\x0D\x0C\x0E\x0B\x12\x19____+>\x1F\x01\x12 ");
    }
    assert(idx >= 0 && idx < (int) strlen(tab));

    return tab[idx];
}
