/*    con_ncurses.cpp
 *
 *    Ncurses front-end to fte - Don Mahurin
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include <ncurses.h>
#include <unistd.h>

#include "sysdep.h"
#include "c_config.h"
#include "console.h"
#include "gui.h"

/* Escape sequence delay in milliseconds */
#define escDelay 10

/* translate fte colors to curses*/
static int fte_curses_colors[] = {
    COLOR_BLACK,
    COLOR_BLUE,
    COLOR_GREEN,
    COLOR_CYAN,
    COLOR_RED,
    COLOR_MAGENTA,
    COLOR_YELLOW,
    COLOR_WHITE,
};


static PCell *SavedScreen = 0;
static int SavedW = 0, SavedH = 0;
static int MaxSavedW = 0, MaxSavedH = 0;

/* Routine to allocate/reallocate and zero space for screen buffer,
   represented as a dynamic array of dynamic PCell lines */
static void SaveScreen() {
    int NewSavedW, NewSavedH;
    ConQuerySize(&NewSavedW, &NewSavedH);
    if (!SavedScreen) {
        SavedScreen = (PCell *) malloc(NewSavedH * sizeof(PCell));
        for (int j = 0 ; j < NewSavedH; j++) {
            SavedScreen[j] = (PCell)malloc(NewSavedW * sizeof(TCell));
            bzero(SavedScreen[j], sizeof(SavedScreen[j]));
        }
        MaxSavedW = SavedW = NewSavedW;
        MaxSavedH = SavedH = NewSavedH;
    } else {
        if (NewSavedW > MaxSavedW) { /* Expand maximum width if needed */
            for (int i = 0 ; i < MaxSavedH; i++) {
//    assert(sizeof(SavedScreen[i]) == MaxSavedH);
                SavedScreen[i] = (PCell)realloc(SavedScreen[i], NewSavedW * sizeof(TCell));
            }
            MaxSavedW = NewSavedW;
        }
        if (NewSavedW > SavedW) { /* Zero newly expanded screen */
            for (int i = 0 ; i < MaxSavedH; i++) {
                bzero(SavedScreen[i] + SavedW, NewSavedW - SavedW);
            }
        }
        if (NewSavedH > MaxSavedH) { /* Expand Maximum height if needed */
            SavedScreen = (PCell *)realloc(SavedScreen, NewSavedH * sizeof(PCell));
            for (int i = MaxSavedH ; i < NewSavedH; i++) {
                SavedScreen[i] = (PCell)malloc(MaxSavedW * sizeof(TCell));
            }
            MaxSavedH = NewSavedH;
        }
        if (NewSavedH > SavedH) { /* Zero newly expanded screen */
            for (int i = SavedH ; i < NewSavedH; i++) {
                bzero(SavedScreen[i], MaxSavedW);
            }
        }
        SavedW = NewSavedW;
        SavedH = NewSavedH;
    }
}

static void free_savedscreen() {
    if (! SavedScreen)
        return;
    for (int i = 0; i < MaxSavedH; i++) {
        if (SavedScreen[i]) {
            free(SavedScreen[i]);
            SavedScreen[i] = NULL;
        }
    }
    free(SavedScreen);
    SavedScreen = NULL;
}

static int fte_curses_attr[256];

static int key_sup = 0;
static int key_sdown = 0;

static int ConInitColors() {
    int c = 0;
    int colors = has_colors();

    if (colors) start_color();
    for (int bgb = 0 ; bgb < 2; bgb++) { /* bg bright bit */
        for (int bg = 0 ; bg < 8; bg++) {
            for (int fgb = 0; fgb < 2; fgb++) { /* fg bright bit */
                for (int fg = 0 ; fg < 8; fg++, c++) {
                    if (colors) {
                        int pair = bg * 8 + fg;
                        if (c != 0) init_pair(pair, fte_curses_colors[fg], fte_curses_colors[bg]);
                        fte_curses_attr[c] = (fgb ? A_BOLD : 0) | COLOR_PAIR(pair);
                    } else {
                        fte_curses_attr[c] = 0;
                        if (fgb || bgb) {
                            if (bg > fg) fte_curses_attr[c] |= (A_UNDERLINE | A_REVERSE);
                            else fte_curses_attr[c] |= A_BOLD;
                        } else if (bg > fg) fte_curses_attr[c] |= A_REVERSE;
                    }
                }
            }
        }
    }
    return colors;
}

int ConInit(int /*XSize */ , int /*YSize */) {
    int ch;
    const char *s;

    ESCDELAY = escDelay;
    initscr();
    ConInitColors();
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    /*    cbreak (); */
    raw();
    noecho();
    nonl();
    keypad(stdscr, 1);
    meta(stdscr, 1);
    SaveScreen();

    /* find shift up/down */
    for (ch = KEY_MAX + 1;;ch++) {
        s = keyname(ch);
        if (s == NULL) break;

        if (!strcmp(s, "kUP"))
            key_sup = ch;
        else if (!strcmp(s, "kDN"))
            key_sdown = ch;
        if (key_sup > 0 && key_sdown > 0) break;
    }
    return 0;
}


int ConDone(void) {
    keypad(stdscr, 0);
    endwin();
    free_savedscreen();
    return 0;
}

int ConSuspend(void) {
    return 0;
}
int ConContinue(void) {
    return 0;
}

int ConSetTitle(char * /*Title */ , char * /*STitle */) {
    return 0;
}

int ConGetTitle(char *Title, int /*MaxLen */
                , char * /*STitle */ , int /*SMaxLen */) {
    *Title = '\0';
    return 0;
}

int ConClear() { /* not used? */
    refresh();
    return 0;
}

static unsigned int  GetDch(int idx) {
    switch (idx) {
    case DCH_C1:
        return ACS_ULCORNER;
    case DCH_C2:
        return ACS_URCORNER;
    case DCH_C3:
        return ACS_LLCORNER;
    case DCH_C4:
        return ACS_LRCORNER;
    case DCH_H:
        return ACS_HLINE;
    case DCH_V:
        return ACS_VLINE;
    case DCH_M1:
        return ACS_HLINE;
    case DCH_M2:
        return ACS_LTEE;
    case DCH_M3:
        return ACS_RTEE;
    case DCH_M4 :
        return 'o';
        break;
    case DCH_X:
        return  'X';
        break;
    case DCH_RPTR:
        return ACS_RARROW;
        break;
    case DCH_EOL:
        return ACS_BULLET;
        break;
    case DCH_EOF:
        return ACS_DIAMOND;
        break;
    case DCH_END:
        return ACS_HLINE;
        break;
    case DCH_AUP:
        return ACS_UARROW;
        break;
    case DCH_ADOWN:
        return ACS_DARROW;
        break;
    case DCH_HFORE:
        return ACS_BLOCK;
        break;
    case DCH_HBACK:
        return ACS_CKBOARD;
        break;
    case DCH_ALEFT:
        return ACS_LARROW;
        break;
    case DCH_ARIGHT:
        return ACS_RARROW;
        break;
    default:
        return '@';
    }
}

static int last_attr = A_NORMAL;

// "Cell" is currently an integer type, but its contents is treated as
// a char data struct. So, create a struct to cast it that way.
typedef struct CellData {
    unsigned char ch;
    unsigned char attr;
} CellData;

int ConPutBox(int X, int Y, int W, int H, PCell Cell) {
    int CurX, CurY;
    getyx(stdscr, CurY, CurX);
    int yy = Y;

    if (Y + H > LINES)
        H = LINES - Y;

    for (int j = 0 ; j < H; j++) {
        memcpy(SavedScreen[Y+j] + X, Cell, W*sizeof(TCell)); // copy outputed line to saved screen
        move(yy++, X);
        for (int i = 0; i < W; i++) {
            CellData *celldata = (CellData *)Cell;
            unsigned char ch = celldata->ch;
            int attr = fte_curses_attr[celldata->attr];
            if (attr != last_attr) {
                wattrset(stdscr, attr);
                last_attr = attr;
            } else attr = 0;

            if (ch < 32) {
                if (ch <= 20) {
                    waddch(stdscr, GetDch(ch));
                } else
                    waddch(stdscr, '.');
            } else if (ch < 128 || ch >= 160) {
                waddch(stdscr, ch);
            }
            /*      else if(ch < 180)
             {
             waddch(stdscr,GetDch(ch-128));
             }
             */
            else {
                waddch(stdscr, '.');
            }
            Cell++;
        }
    }

    move(CurY, CurX);
    refresh();

    return 0;
}

int ConGetBox(int X, int Y, int W, int H, PCell Cell) {
    for (int j = 0 ; j < H; j++) {
        memcpy(Cell, SavedScreen[Y+j] + X, W*sizeof(TCell));
        Cell += W;
    }

    return 0;

}

int ConPutLine(int X, int Y, int W, int H, PCell Cell) {
    for (int j = 0 ; j < H; j++) {
        ConPutBox(X, Y + j, W, 1, Cell);
    }

    return 0;
}

int ConSetBox(int X, int Y, int W, int H, TCell Cell) {
    PCell line = (PCell) malloc(sizeof(TCell) * W);
    int i;

    for (i = 0; i < W; i++)
        line[i] = Cell;
    ConPutLine(X, Y++, W, H, line);
    free(line);
    return 0;
}



int ConScroll(int Way, int X, int Y, int W, int H, TAttr Fill, int Count) {
    PCell box;

    box = new TCell [W * H];

    TCell fill = (((unsigned) Fill) << 8) | ' ';

    ConGetBox(X, Y, W, H, box);

    if (Way == csUp) {
        ConPutBox(X, Y, W, H - Count, box + W * Count);
        ConSetBox(X, Y + H - Count, W, Count, fill);
    } else {
        ConPutBox(X, Y + Count, W, H - Count, box);
        ConSetBox(X, Y, W, Count, fill);
    }

    delete [](box);

    return 0;
}

int ConSetSize(int /*X */ , int /*Y */) {
    return -1;
}

static void ResizeWindow(int ww, int hh) {
    SaveScreen();
    if (frames) {
        frames->Resize(ww, hh);
        frames->Repaint();
    }
}


int ConQuerySize(int *X, int *Y) {
    *X = COLS;
    *Y = LINES;
    return 0;
}

int ConSetCursorPos(int X, int Y) {
    move(Y, X);
    refresh();
    return 0;
}

int ConQueryCursorPos(int *X, int *Y) {
    getyx(stdscr, *Y, *X);
    return 0;
}

static int CurVis = 1;

int ConShowCursor() {
    CurVis = 1;
    curs_set(1);
    return 0;
}
int ConHideCursor() {
    CurVis = 0;
    curs_set(0);
    return 0;
}
int ConCursorVisible() {
    return CurVis;
}

void ConSetInsertState(bool insert) {
}

int ConSetMousePos(int /*X */ , int /*Y */) {
    return -1;
}
int ConQueryMousePos(int *X, int *Y) {
    *X = 0;
    *Y = 0;
    return 0;
}

int ConShowMouse() {
    return -1;
}

int ConHideMouse() {
    return -1;
}

int ConMouseVisible() {
    return 0;
}

int ConQueryMouseButtons(int *ButtonCount) {
    *ButtonCount = 0;
    return 0;
}

static int ConGetMouseEvent(
    TEvent * Event) {
    MEVENT mevent;
    if (getmouse(&mevent) == ERR) {
        Event->What = evNone;
        return -1;
    }
    mmask_t bstate = mevent.bstate;

    Event->What = evNone;
    if (bstate & BUTTON1_PRESSED) {
        Event->What = Event->Mouse.What = evMouseDown;
        Event->Mouse.X = mevent.x;
        Event->Mouse.Y = mevent.y;
        Event->Mouse.Buttons = 1;
        Event->Mouse.Count = 1;
    } else if (bstate & BUTTON1_RELEASED) {
        Event->What = Event->Mouse.What = evMouseUp;
        Event->Mouse.X = mevent.x;
        Event->Mouse.Y = mevent.y;
        Event->Mouse.Buttons = 1;
        Event->Mouse.Count = (bstate & BUTTON1_DOUBLE_CLICKED) ? 2 : 1;
    } else if (bstate & BUTTON1_CLICKED) {
        Event->What = Event->Mouse.What = evMouseDown;
        Event->Mouse.X = mevent.x;
        Event->Mouse.Y = mevent.y;
        Event->Mouse.Buttons = 1;
        Event->Mouse.Count = 1;
        mevent.bstate ^= BUTTON1_CLICKED;
        mevent.bstate |= BUTTON1_RELEASED;
        ungetmouse(&mevent);
    } else if (bstate & BUTTON1_DOUBLE_CLICKED) {
        Event->Mouse.X = mevent.x;
        Event->Mouse.Y = mevent.y;
        Event->Mouse.Buttons = 1;
        Event->Mouse.Count = 2;
        mevent.bstate |= BUTTON1_RELEASED;
        ungetmouse(&mevent);
    }
    return 0;
}

static TEvent Prev = {evNone};

static int ConGetEscEvent(TEvent *Event) {
    int ch;

    TKeyEvent *KEvent = &(Event->Key);

    keypad(stdscr, 0);

    timeout(escDelay);

    ch = getch();
    if (ch == 033) {
        ch = getch();
        if (ch == '[' || ch == 'O') KEvent->Code |= kfAlt;

    }

    if (ch == ERR) {
        KEvent->Code |= kbEsc;
    } else if (ch == '[' || ch == 'O') {
        int ch1 = getch();
        int endch = '\0';
        int modch = '\0';

        if (ch1 == ERR) { /* translate to Alt-[ or Alt-O */
            KEvent->Code |= (kfAlt | ch);
        } else {
            if (ch1 >= '1' && ch1 <= '8') { // [n...
                endch = getch();
                if (endch == ERR) { // //[n, not valid
                    // TODO, should this be ALT-7 ?
                    endch = '\0';
                    ch1 = '\0';
                }
            } else { // [A
                endch = ch1;
                ch1 = '\0';
            }

            if (endch == ';') { // [n;mX
                modch = getch();
                endch = getch();
            } else if (ch1 != '\0' && endch != '~' && endch != '$') { // [mA
                modch = ch1;
                ch1 = '\0';
            }

            if (modch != '\0') {
                int ctAlSh = ch1 - '1';
                if (ctAlSh & 0x4) KEvent->Code |= kfCtrl;
                if (ctAlSh & 0x2) KEvent->Code |= kfAlt;
                if (ctAlSh & 0x1) KEvent->Code |= kfShift;
            }

            switch (endch) {

            case 'A':
                KEvent->Code |= kbUp;
                break;
            case 'B':
                KEvent->Code |= kbDown;
                break;
            case 'C':
                KEvent->Code |= kbRight;
                break;
            case 'D':
                KEvent->Code |= kbLeft;
                break;
            case 'F':
                KEvent->Code |= kbEnd;
                break;
            case 'H':
                KEvent->Code |= kbHome;
                break;

            case 'a':
                KEvent->Code |= (kfShift | kbUp);
                break;
            case 'b':
                KEvent->Code |= (kfShift | kbDown);
                break;
            case 'c':
                KEvent->Code |= (kfShift | kbRight);
                break;
            case 'd':
                KEvent->Code |= (kfShift | kbLeft);
                break;
            case '$':
                KEvent->Code |= kfShift;
            case '~':
                switch (ch1 - '0') {
                case 1:
                    KEvent->Code |= kbHome;
                    break;
                case 2:
                    KEvent->Code |= kbIns;
                    break;
                case 3:
                    KEvent->Code |= kbDel;
                    break;
                case 4:
                    KEvent->Code |= kbEnd;
                    break;
                case 5:
                    KEvent->Code |= kbPgUp;
                    break;
                case 6:
                    KEvent->Code |= kbPgDn;
                    break;
                case 7:
                    KEvent->Code |= kbHome;
                    break;
                case 8:
                    KEvent->Code |= kbEnd;
                    break;
                default:
                    Event->What |= evNone;
                    break;
                }
                break;
            default:
                Event->What = evNone;
                break;
            }
        }
    } else {
        KEvent->Code |= kfAlt;
        if (ch == '\r' || ch == '\n') {
            KEvent->Code |= kbEnter;
        } else if (ch == '\t') {
            KEvent->Code |= kbTab;
        } else if (ch < 32) {
            KEvent->Code |= (kfCtrl | (ch + 0100));
        } else {
            if (ch > 0x60 && ch < 0x7b) { /* Alt-A == Alt-a*/
                ch -= 0x20;
            }
            KEvent->Code |= ch;
        }
    }

    timeout(-1);
    keypad(stdscr, 1);

    return 1;
}

extern int WaitPipeEvent(TEvent *Event, int WaitTime, int *fds, int nfds);

int ConGetEvent(TEventMask /*EventMask */ ,
                TEvent * Event, int WaitTime, int Delete) {
    int sfd[1];
    int rtn;
    TKeyEvent *KEvent = &(Event->Key);
    if (WaitTime == 0) return -1;

    if (Prev.What != evNone) {
        *Event = Prev;
        if (Delete)
            Prev.What = evNone;
        return 1;
    }

    sfd[0] = STDIN_FILENO;

    if ((rtn = WaitPipeEvent(Event, -1, sfd, 1)) != 0) return rtn;

    int ch = wgetch(stdscr);
    Event->What = evKeyDown;
    KEvent->Code = 0;

    if (SevenBit && ch > 127 && ch < 256) {
        KEvent->Code |= kfAlt;
        ch -= 128;
        if (ch > 0x60 && ch < 0x7b) { /* Alt-A == Alt-a*/
            ch -= 0x20;
        }
    }

    if (ch < 0) {
        Event->What = evNone;
    } else if (ch == 27) {
        ConGetEscEvent(Event);
    } else if (ch == '\r' || ch == '\n') {
        KEvent->Code |= kbEnter;
    } else if (ch == '\t') {
        KEvent->Code |= kbTab;
    } else if (ch < 32) {
        KEvent->Code |= (kfCtrl | (ch + 0100));
    } else if (ch < 256) {
        KEvent->Code |= ch;
    } else { // > 255
        switch (ch) {
        case KEY_RESIZE:
            ResizeWindow(COLS, LINES);
            Event->What = evNone;
            break;
        case KEY_MOUSE:
            Event->What = evNone;
            ConGetMouseEvent(Event);
            break;
		case KEY_SF:
			KEvent->Code = kfShift | kbDown;
			break;
		case KEY_SR:
			KEvent->Code = kfShift | kbUp;
			break;
		case KEY_SRIGHT:
            KEvent->Code = kfShift | kbRight;
            break;
        case KEY_SLEFT:
            KEvent->Code = kfShift | kbLeft;
            break;
        case KEY_SDC:
            KEvent->Code = kfShift | kbDel;
            break;
        case KEY_SIC:
            KEvent->Code = kfShift | kbIns;
            break;
        case KEY_SHOME:
            KEvent->Code = kfShift | kbHome;
            break;
        case KEY_SEND:
            KEvent->Code = kfShift | kbEnd;
            break;
        case KEY_SNEXT:
            KEvent->Code = kfShift | kbPgDn;
            break;
        case KEY_SPREVIOUS:
            KEvent->Code = kfShift | kbPgUp;
            break;
        case KEY_UP:
            KEvent->Code = kbUp;
            break;
        case KEY_DOWN:
            KEvent->Code = kbDown;
            break;
        case KEY_RIGHT:
            KEvent->Code = kbRight;
            break;
        case KEY_LEFT:
            KEvent->Code = kbLeft;
            break;
        case KEY_DC:
            KEvent->Code = kbDel;
            break;
        case KEY_IC:
            KEvent->Code = kbIns;
            break;
        case KEY_BACKSPACE:
            KEvent->Code = kbBackSp;
            break;
        case KEY_HOME:
            KEvent->Code = kbHome;
            break;
        case KEY_END:
        case KEY_LL: // used in old termcap/infos
            KEvent->Code = kbEnd;
            break;
        case KEY_NPAGE:
            KEvent->Code = kbPgDn;
            break;
        case KEY_PPAGE:
            KEvent->Code = kbPgUp;
            break;
        case KEY_F(1):
                        KEvent->Code = kbF1;
            break;
        case KEY_F(2):
                        KEvent->Code = kbF2;
            break;
        case KEY_F(3):
                        KEvent->Code = kbF3;
            break;
        case KEY_F(4):
                        KEvent->Code = kbF4;
            break;
        case KEY_F(5):
                        KEvent->Code = kbF5;
            break;
        case KEY_F(6):
                        KEvent->Code = kbF6;
            break;
        case KEY_F(7):
                        KEvent->Code = kbF7;
            break;
        case KEY_F(8):
                        KEvent->Code = kbF8;
            break;
        case KEY_F(9):
                        KEvent->Code = kbF9;
            break;
        case KEY_F(10):
                        KEvent->Code = kbF10;
            break;
        case KEY_F(11):
                        KEvent->Code = kbF11;
            break;
        case KEY_F(12):
                        KEvent->Code = kbF12;
            break;
        case KEY_B2:
            KEvent->Code = kbCenter;
            break;
        case KEY_ENTER: /* shift enter */
            KEvent->Code |= kbEnter;
            break;
        default:
            if (key_sdown != 0 && ch == key_sdown)
                KEvent->Code = kfShift | kbDown;
            else if (key_sup != 0 && ch == key_sup)
                KEvent->Code = kfShift | kbUp;
            else {
                Event->What = evNone;
                // fprintf(stderr, "Unknown 0x%x %d\n", ch, ch);
            }
            break;
        }
    }

    if (!Delete)
        Prev = *Event;

    return 1;
}

char ConGetDrawChar(int idx) {
    //    return 128+idx;
    return idx;
}

int ConPutEvent(TEvent Event) {
    Prev = Event;
    return 0;
}

GUI::GUI(int &argc, char **argv, int XSize, int YSize) {
    fArgc = argc;
    fArgv = argv;
    ::ConInit(-1, -1);
    ::ConSetSize(XSize, YSize);
    gui = this;
}

GUI::~GUI() {
    ::ConDone();
    gui = 0;
}

int GUI::ConSuspend(void) {
    return::ConSuspend();
}

int GUI::ConContinue(void) {
    return::ConContinue();
}

int GUI::ShowEntryScreen() {
    return 1;
}

int GUI::RunProgram(int /*mode */ , char *Command) {
    int rc, W, H, W1, H1;

    ConQuerySize(&W, &H);
    ConHideMouse();
    ConSuspend();

    if (*Command == 0)  // empty string = shell
        Command = getenv("SHELL");

    rc = system(Command);

    ConContinue();
    ConShowMouse();
    ConQuerySize(&W1, &H1);

    if (W != W1 || H != H1) {
        frames->Resize(W1, H1);
    }
    frames->Repaint();
    return rc;
}
