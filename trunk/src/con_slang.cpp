/*    con_slang.cpp
 *
 *    Copyright (c) 1998, István Váradi
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

static int use_esc_hack = 0;

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

#include <slang.h>

#include "sysdep.h"
#include "c_config.h"
#include "console.h"
// #include "slangkbd.h"
#include "gui.h"

#define MAX_PIPES 4
//#define PIPE_BUFLEN 4096

typedef struct {
    int used;
    int id;
    int fd;
    int pid;
    int stopped;
    EModel *notify;
} GPipe;

static GPipe Pipes[MAX_PIPES] =
{
    {0},
    {0},
    {0},
    {0}
};

/* These characters cannot appear on a console, so we can detect
 * them in the output routine.
 */
#define DCH_SLANG_C1         128
#define DCH_SLANG_C2         129
#define DCH_SLANG_C3         130
#define DCH_SLANG_C4         131
#define DCH_SLANG_H          132
#define DCH_SLANG_V          133
#define DCH_SLANG_M1         134
#define DCH_SLANG_M2         135
#define DCH_SLANG_M3         136
#define DCH_SLANG_M4         137
#define DCH_SLANG_X          138
#define DCH_SLANG_RPTR       139
#define DCH_SLANG_EOL        140
#define DCH_SLANG_EOF        141
#define DCH_SLANG_END        142
#define DCH_SLANG_AUP        143
#define DCH_SLANG_ADOWN      144
#define DCH_SLANG_HFORE      145
#define DCH_SLANG_HBACK      146
#define DCH_SLANG_ALEFT      147
#define DCH_SLANG_ARIGHT     148

static char slang_dchs[] =
{
    'l',
    'k',
    'm',
    'j',
    'q',
    'x',
    'f',
    'f',
    'f',
    'f',
    'f',
    '+',
    '~',
    '`',
    'q',
    '-',
    '.',
    ' ',
    'a',
    ',',
    '+'
};

static char raw_dchs[sizeof(slang_dchs)];

static unsigned char ftesl_get_dch(char raw)
{
    for (int i = 0; i < (int) sizeof(slang_dchs); i++)
	if (raw_dchs[i] == raw)
	    return DCH_SLANG_C1 + i;
    return DCH_SLANG_EOL;
}

static const char *const slang_colors[] =
{
    "black",
    "blue",
    "green",
    "cyan",
    "red",
    "magenta",
    "brown",
    "lightgray",
    "gray",
    "brightblue",
    "brightgreen",
    "brightcyan",
    "brightred",
    "brightmagenta",
    "yellow",
    "white",
};

/*
 * Definitions for keyboard handling under SLang.
 */

#define FTESL_KEY            0x00001000		// A key defined by me
#define FTESL_KEY_SHIFT      0x00002000		// Key with Shift
#define FTESL_KEY_CTRL       0x00004000		// Key with Ctrl
#define FTESL_KEY_ALT        0x00008000		// Key with Alt
#define FTESL_KEY_GRAY       0x00010000		// Gray Key

#define FTESL_KEY_ENTER      13
#define FTESL_KEY_TAB         9
#define FTESL_KEY_ESC        27
#define FTESL_KEY_BACKSP      8

#define FTESL_KEY_CTRLAND(x)    (x+1-'a')

static const TKeyCode speckeys[] =
{
    kbF1,
    kbF2,
    kbF3,
    kbF4,
    kbF5,
    kbF6,
    kbF7,
    kbF8,
    kbF9,
    kbF10,
    kbF11,
    kbF12,
    kbHome,
    kbEnd,
    kbPgUp,
    kbPgDn,
    kbIns,
    kbDel,
    kbUp,
    kbDown,
    kbLeft,
    kbRight,
    kbEnter,
    kbEsc,
    kbBackSp,
    kbSpace,
    kbTab,
    kbCenter,
};

/*
static int ftesl_getkeysym(TKeyCode keycode)
{
    unsigned key = keyCode(keycode);
    int ksym = -1;

    for (unsigned i = 0; i < sizeof(speckeys) / sizeof(TKeyCode); i++) {
	if (key == speckeys[i]) {
	    ksym = (int) i;
	    break;
	}
    }

    if (ksym < 0 && key < 256) {
	ksym = (int) key;
    }

    if (ksym < 0)
	return ksym;

    if (keycode & kfAlt)
	ksym |= FTESL_KEY_ALT;
    if (keycode & kfCtrl)
	ksym |= FTESL_KEY_CTRL;
    if (keycode & kfShift)
	ksym |= FTESL_KEY_SHIFT;
    if (keycode & kfGray)
	ksym |= FTESL_KEY_GRAY;

    ksym |= FTESL_KEY;
    return ksym;
}
*/

int ConInit(int /*XSize */ , int /*YSize */ )
{
    unsigned i;
    unsigned short linebuf[sizeof(slang_dchs)];

    SLtt_get_terminfo();

    if (SLkp_init() == -1) {
	return -1;
    }
    if (SLang_init_tty(0, 1, 1) == -1) {
	return -1;
    }

    if (SLsmg_init_smg() == -1) {
	SLang_reset_tty();
	return -1;
    }

    SLang_set_abort_signal(NULL);

    SLtty_set_suspend_state(0);

    for (i = 0; i < 128; i++) {
	SLtt_set_color(i, NULL, (char *) slang_colors[i & 0x0f],
		       (char *) slang_colors[((i >> 4) + 0) & 0x07]);
    }

    SLsmg_gotorc(0, 0);
    SLsmg_set_char_set(1);

    SLsmg_write_nchars(slang_dchs, sizeof(slang_dchs));

    SLsmg_gotorc(0, 0);
    SLsmg_read_raw(linebuf, sizeof(slang_dchs));
    for (i = 0; i < sizeof(slang_dchs); i++)
	raw_dchs[i] = (linebuf[i]) & 0xff;

    SLsmg_set_char_set(0);

    use_esc_hack = (getenv("FTESL_ESC_HACK") != NULL);

    return 0;
}
int ConDone(void)
{
    SLsmg_reset_smg();
    SLang_reset_tty();
    return 0;
}

int ConSuspend(void)
{
    SLsmg_suspend_smg();
    SLang_reset_tty();
    return 0;
}
int ConContinue(void)
{
    SLang_init_tty(-1, 0, 1);
    SLsmg_resume_smg();
    return 0;
}

int ConSetTitle(char * /*Title */ , char * /*STitle */ )
{
    return 0;
}

int ConGetTitle(char *Title, int /*MaxLen */
		, char * /*STitle */ , int /*SMaxLen */ )
{
    *Title = '\0';
    return 0;
}

int ConClear()
{
    SLsmg_cls();
    SLsmg_refresh();
    return 0;
}

static void fte_write_color_chars(PCell Cell, int W)
{
    int i = 0;
    int chset = 0, chsetprev = 0;
    unsigned char ch, col = 0x70, colprev = 0x70;
    char buf[256];

    SLsmg_set_color(colprev);
    while (W > 0) {
	for (i = 0; i < W && i < (int) sizeof(buf); i++) {
	    ch = Cell[i] & 0xff;
	    col = (Cell[i] >> 8) & 0x7f;
	    if (ch <= 127 || ch >= 0xa0) {
		if (ch < 32)
		    buf[i] = '.';
		else
		    buf[i] = ch;
		chset = 0;
	    } else {
		buf[i] = slang_dchs[ch - 128];
		chset = 1;
	    }



	    if (col != colprev || chset != chsetprev)
		break;

	}

	if (i > 0) {
	    SLsmg_write_nchars(buf, i);
	    W -= i;
	    Cell += i;
	}

	if (col != colprev) {
	    SLsmg_set_color(col);
	    colprev = col;
	}

	if (chset != chsetprev) {
	    SLsmg_set_char_set(chset);
	    chsetprev = chset;
	}
    }
    SLsmg_set_char_set(0);
}

int ConPutBox(int X, int Y, int W, int H, PCell Cell)
{
    int CurX, CurY;

    ConQueryCursorPos(&CurX, &CurY);
    while (H > 0) {
	SLsmg_gotorc(Y++, X);
	fte_write_color_chars(Cell, W);
	Cell += W;
	H--;
    }
    ConSetCursorPos(CurX, CurY);
    SLsmg_refresh();

    return 0;
}

static int ConPutBoxRaw(int X, int Y, int W, int H, unsigned short *box)
{
    int CurX, CurY;

    ConQueryCursorPos(&CurX, &CurY);
    while (H > 0) {
	SLsmg_gotorc(Y++, X);
	SLsmg_write_raw(box, W);
	box += W;
	H--;
    }
    ConSetCursorPos(CurX, CurY);

    return 0;

}

int ConGetBox(int X, int Y, int W, int H, PCell Cell)
{
    int CurX, CurY, i;
    char ch;

    ConQueryCursorPos(&CurX, &CurY);
    while (H > 0) {
	SLsmg_gotorc(Y++, X);
	SLsmg_read_raw(Cell, W);
	for (i = 0; i < W; i++)
	    if (Cell[i] & 0x8000) {
		ch = Cell[i] & 0xff;
		Cell[i] &= 0x7f00;
		Cell[i] |= ftesl_get_dch(ch);
	    }
	Cell += W;
	H--;
    }
    ConSetCursorPos(CurX, CurY);

    return 0;

}

static int ConGetBoxRaw(int X, int Y, int W, int H, unsigned short *box)
{
    int CurX, CurY;

    ConQueryCursorPos(&CurX, &CurY);
    while (H > 0) {
	SLsmg_gotorc(Y++, X);
	SLsmg_read_raw(box, W);
	box += W;
	H--;
    }
    ConSetCursorPos(CurX, CurY);

    return 0;

}

int ConPutLine(int X, int Y, int W, int H, PCell Cell)
{
    int CurX, CurY;

    ConQueryCursorPos(&CurX, &CurY);
    while (H > 0) {
	SLsmg_gotorc(Y, X);
	fte_write_color_chars(Cell, W);
	H--;
    }
    ConSetCursorPos(CurX, CurY);
    SLsmg_refresh();

    return 0;
}

int ConSetBox(int X, int Y, int W, int H, TCell Cell)
{
    PCell line = (PCell) malloc(sizeof(TCell) * W);
    int i;

    for (i = 0; i < W; i++)
	line[i] = Cell;
    ConPutLine(X, Y++, W, H, line);
    free(line);
    return 0;
}



int ConScroll(int Way, int X, int Y, int W, int H, TAttr Fill, int Count)
{
    unsigned short *box;

    box = new unsigned short [W * H];

    TCell fill = (((unsigned) Fill) << 8) | ' ';

    ConGetBoxRaw(X, Y, W, H, box);

    if (Way == csUp) {
	ConPutBoxRaw(X, Y, W, H - Count, box + W * Count);
	ConSetBox(X, Y + H - Count, W, Count, fill);
    } else {
	ConPutBoxRaw(X, Y + Count, W, H - Count, box);
	ConSetBox(X, Y, W, Count, fill);
    }

    delete [] (box);

    return 0;
}

int ConSetSize(int /*X */ , int /*Y */ )
{
    return -1;
}

int ConQuerySize(int *X, int *Y)
{
    *X = SLtt_Screen_Cols;
    *Y = SLtt_Screen_Rows;
    return 0;
}

int ConSetCursorPos(int X, int Y)
{
    SLsmg_gotorc(Y, X);
    SLsmg_refresh();
    return 0;
}

int ConQueryCursorPos(int *X, int *Y)
{
    *X = SLsmg_get_column();
    *Y = SLsmg_get_row();
    return 0;
}

static int CurVis = 1;

int ConShowCursor()
{
    CurVis = 1;
    SLtt_set_cursor_visibility(1);
    return 0;
}
int ConHideCursor()
{
    CurVis = 0;
    SLtt_set_cursor_visibility(0);
    return 0;
}
int ConCursorVisible()
{
    return CurVis;
}

int ConSetCursorSize(int /*Start */ , int /*End */ )
{
    return 0;
}

int ConSetMousePos(int /*X */ , int /*Y */ )
{
    return -1;
}
int ConQueryMousePos(int *X, int *Y)
{
    *X = 0;
    *Y = 0;
    return 0;
}

int ConShowMouse()
{
    return -1;
}

int ConHideMouse()
{
    return -1;
}

int ConMouseVisible()
{
    return 0;
}

int ConQueryMouseButtons(int *ButtonCount)
{
    *ButtonCount = 0;
    return 0;
}

static TEvent Prev =
{evNone};

static const TKeyCode keys_ctrlhack[] =
{
    kfAlt,
    kbHome,
    kfCtrl,
    kbDown,
    kbEnd,

    kbF1,
    kfCtrl | 'G',
    kbBackSp,
    kbTab,
    kfCtrl | 'J',

    kfCtrl | 'K',
    kbLeft,
    kbEnter,
    kbPgDn,
    kfCtrl | 'O',

    kbPgUp,
    kbIns,
    kbRight,
    kfShift,
    kfCtrl | 'T',

    kbUp,
    kfCtrl | 'V',
    kfCtrl | 'W',
    kbCenter,
    kfCtrl | 'Y',

    kbDel,
    kbEsc,
    kbCtrl | '\\',
    kbCtrl | ']',
    kbCtrl | '^',
    kbCtrl | '_'
};

static TKeyCode ftesl_getftekey(unsigned char key)
{
    if (key < 32)
	return speckeys[key];
    else
	return (TKeyCode) key;
}

/*
 * Keyboard handling with SLang.
 */
static TKeyCode ftesl_process_key(int key, int ctrlhack = 0)
{
    TKeyCode kcode;

    //fprintf(stderr, "KEY  %03d \n", key);
    if (key < 256 && key >= 32) {
	return (TKeyCode) key;
    } else if (key >= 1 && key <= 31 && key != 13 && key != 9 && key != 8
	       && key != 27) {
	if (!ctrlhack)
	    return ((key + 'A' - 1) & 0xff) | kfCtrl;
	else
	    return keys_ctrlhack[key - 1];
    } else if (key & FTESL_KEY) {
	kcode = ftesl_getftekey(key & 0x00ff);
	if (key & FTESL_KEY_SHIFT)
	    kcode |= kfShift;
	if (key & FTESL_KEY_CTRL)
	    kcode |= kfCtrl;
	if (key & FTESL_KEY_ALT)
	    kcode |= kfAlt;
	if (key & FTESL_KEY_GRAY)
	    kcode |= kfGray;
	return kcode;
    } else
	switch (key) {
	case SL_KEY_UP:
	    return kbUp;
	case SL_KEY_DOWN:
	    return kbDown;
	case SL_KEY_LEFT:
	    return kbLeft;
	case SL_KEY_RIGHT:
	    return kbRight;
	case SL_KEY_PPAGE:
	    return kbPgUp;
	case SL_KEY_NPAGE:
	    return kbPgDn;
	case SL_KEY_HOME:
	    return kbHome;
	case SL_KEY_END:
	    return kbEnd;
	case SL_KEY_BACKSPACE:
	case FTESL_KEY_BACKSP:
	    return kbBackSp;
	case SL_KEY_ENTER:
	case FTESL_KEY_ENTER:
	    return kbEnter;
	case SL_KEY_IC:
	    return kbIns;
	case SL_KEY_DELETE:
	    return kbDel;
	case SL_KEY_F(1):
	    return kbF1;
	case SL_KEY_F(2):
	    return kbF2;
	case SL_KEY_F(3):
	    return kbF3;
	case SL_KEY_F(4):
	    return kbF4;
	case SL_KEY_F(5):
	    return kbF5;
	case SL_KEY_F(6):
	    return kbF6;
	case SL_KEY_F(7):
	    return kbF7;
	case SL_KEY_F(8):
	    return kbF8;
	case SL_KEY_F(9):
	    return kbF9;
	case SL_KEY_F(10):
	    return kbF10;
	case SL_KEY_F(11):
	    return kbF11;
	case SL_KEY_F(12):
	    return kbF12;
	case FTESL_KEY_TAB:
	    return kbTab;
	case FTESL_KEY_ESC:
	case SL_KEY_ERR:
	    return kbEsc;
	default:
	    return '?';
	}
}


int ConGetEvent(TEventMask /*EventMask */ ,
		TEvent * Event, int WaitTime, int Delete)
{
    int key;
    TKeyEvent *KEvent = &(Event->Key);
    fd_set readfds;
    struct timeval timeout;

    if (Prev.What != evNone) {
	*Event = Prev;
	if (Delete)
	    Prev.What = evNone;
	return 1;
    }

    Event->What = evNone;

    WaitTime = (WaitTime >= 0) ? WaitTime / 100 : 36000;

    FD_ZERO(&readfds);

    FD_SET(0, &readfds);
    for (int p = 0; p < MAX_PIPES; p++)
	if (Pipes[p].used && Pipes[p].fd != -1)
	    FD_SET(Pipes[p].fd, &readfds);

    if (WaitTime == -1) {
	if (select(sizeof(fd_set) * 8, &readfds, NULL, NULL, NULL) < 0)
	    return -1;
    } else {
	timeout.tv_sec = WaitTime / 1000;
	timeout.tv_usec = (WaitTime % 1000) * 1000;
	if (select(sizeof(fd_set) * 8, &readfds, NULL, NULL, &timeout) < 0)
	    return -1;
    }

    if (SLang_input_pending(0) > 0) {
	TKeyCode kcode = 0, kcode1;

	key = SLang_getkey();
	int escfirst = 1;

	if (key == 27)
	    while (1) {
		if (use_esc_hack) {
		    if (SLang_input_pending(1) == 0) {
			kcode = kbEsc;
			break;
		    }
		}

		key = SLang_getkey();
		if (key == 3) {
		    SLang_ungetkey(key);
		    SLkp_getkey();
		}
		if (key >= 'a' && key <= 'z')
		    key -= 'a' - 'A';
		if (key == 27) {
		    kcode = kbEsc;
		    break;
		} else if (key == '[' && escfirst) {
		    unsigned char kbuf[2];

		    kbuf[0] = 27;
		    kbuf[1] = (char) key;
		    SLang_ungetkey_string(kbuf, 2);
		    key = SLkp_getkey();
		    if (key == 0xFFFF) {
			if (SLang_input_pending(0) == 0) {
			    /*
			     * SLang got an unknown key and ate it.
			     * beep and bonk out.
			     */
			    SLtt_beep();
			    return -1;
			}
			/*
			 * SLang encountered an unknown key sequence, so we
			 * try to parse the sequence one by one and thus
			 * enable the user to configure a binding for it
			 */
			key = SLang_getkey();
			if (key != 27) {
			    SLtt_beep();
			    SLang_flush_input();
			    return -1;
			}
		    }
		    kcode = ftesl_process_key(key, 0);
		    break;
		} else {
		    kcode1 = ftesl_process_key(key, 1);
		    if (keyCode(kcode1) == kbF1) {
			key = SLang_getkey();
			switch (key) {
			case '0':
			    kcode |= kbF10;
			    break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			    kcode |= kbF1 + key - '1';
			    break;
			case 'a':
			case 'b':
			    kcode |= kbF11 + key - 'a';
			    break;
			}
		    } else
			kcode |= kcode1;

		    if (keyCode(kcode) != 0) {
			if (escfirst)
			    kcode |= kfAlt;
			break;
		    }
		}
		escfirst = 0;
	} else {
	    SLang_ungetkey(key);
	    key = SLkp_getkey();
	    kcode = ftesl_process_key(key, 0);
	}

	Event->What = evKeyDown;
	KEvent->Code = kcode;

	if (!Delete)
	    Prev = *Event;

	return 1;
    } else {
	for (int pp = 0; pp < MAX_PIPES; pp++) {
	    if (Pipes[pp].used && Pipes[pp].fd != -1 &&
		FD_ISSET(Pipes[pp].fd, &readfds) &&
		Pipes[pp].notify) {
		Event->What = evNotify;
		Event->Msg.View = 0;
		Event->Msg.Model = Pipes[pp].notify;
		Event->Msg.Command = cmPipeRead;
		Event->Msg.Param1 = pp;
		Pipes[pp].stopped = 0;
		return 0;
	    }
	    //fprintf(stderr, "Pipe %d\n", Pipes[pp].fd);
	}
    }

    return -1;
}

int ConPutEvent(TEvent Event)
{
    Prev = Event;
    return 0;
}

GUI::GUI(int &argc, char **argv, int XSize, int YSize)
{
    fArgc = argc;
    fArgv = argv;
    ::ConInit(-1, -1);
    ::ConSetSize(XSize, YSize);
    gui = this;
}

GUI::~GUI()
{
    ::ConDone();
    gui = 0;
}

int GUI::ConSuspend(void)
{
    return::ConSuspend();
}

int GUI::ConContinue(void)
{
    return::ConContinue();
}

int GUI::ShowEntryScreen()
{
    TEvent E;

    ConHideMouse();
    do {
	gui->ConGetEvent(evKeyDown, &E, -1, 1, 0);
    } while (E.What != evKeyDown);
    ConShowMouse();
    if (frames)
	frames->Repaint();
    return 1;
}

int GUI::OpenPipe(char *Command, EModel * notify)
{
    int i;

    for (i = 0; i < MAX_PIPES; i++) {
	if (Pipes[i].used == 0) {
	    int pfd[2];

	    Pipes[i].id = i;
	    Pipes[i].notify = notify;
	    Pipes[i].stopped = 1;

	    if (pipe((int *) pfd) == -1)
		return -1;

	    switch (Pipes[i].pid = fork()) {
	    case -1:		/* fail */
		return -1;
	    case 0:		/* child */
		signal(SIGPIPE, SIG_DFL);
		close(pfd[0]);
		close(0);
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
	    return i;
	}
    }
    return -1;
}

int GUI::SetPipeView(int id, EModel * notify)
{
    if (id < 0 || id > MAX_PIPES)
	return -1;
    if (Pipes[id].used == 0)
	return -1;

    Pipes[id].notify = notify;
    return 0;
}

int GUI::ReadPipe(int id, void *buffer, int len)
{
    int rc;

    if (id < 0 || id > MAX_PIPES)
	return -1;
    if (Pipes[id].used == 0)
	return -1;

    rc = read(Pipes[id].fd, buffer, len);
    if (rc == 0) {
	close(Pipes[id].fd);
	Pipes[id].fd = -1;
	return -1;
    }
    if (rc == -1) {
	Pipes[id].stopped = 1;
	return 0;
    }
    return rc;
}

int GUI::ClosePipe(int id)
{
    int status = -1;

    if (id < 0 || id > MAX_PIPES)
	return -1;
    if (Pipes[id].used == 0)
	return -1;
    if (Pipes[id].fd != -1)
	close(Pipes[id].fd);

    kill(Pipes[id].pid, SIGHUP);
    alarm(1);
    waitpid(Pipes[id].pid, &status, 0);
    alarm(0);
    Pipes[id].used = 0;
    return WEXITSTATUS(status);
}

int GUI::RunProgram(int /*mode */ , char *Command)
{
    int rc, W, H, W1, H1;

    ConQuerySize(&W, &H);
    ConHideMouse();
    ConSuspend();

    if (*Command == 0)		// empty string = shell
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

char ConGetDrawChar(int idx)
{
    static const char * use_tab = NULL;
    static int use_tab_size = 0;

    static const char tab[] =
    {
	DCH_SLANG_C1,
	DCH_SLANG_C2,
	DCH_SLANG_C3,
	DCH_SLANG_C4,
	DCH_SLANG_H,
	DCH_SLANG_V,
	DCH_SLANG_M1,
	DCH_SLANG_M2,
	DCH_SLANG_M3,
	DCH_SLANG_M4,
	DCH_SLANG_X,
	DCH_SLANG_RPTR,
	DCH_SLANG_EOL,
	DCH_SLANG_EOF,
	DCH_SLANG_END,
	DCH_SLANG_AUP,
	DCH_SLANG_ADOWN,
	DCH_SLANG_HFORE,
	DCH_SLANG_HBACK,
	DCH_SLANG_ALEFT,
	DCH_SLANG_ARIGHT
    };

    static const char tab_linux[] =
    {
	DCH_SLANG_C1,
	DCH_SLANG_C2,
	DCH_SLANG_C3,
	DCH_SLANG_C4,
	DCH_SLANG_H,
	DCH_SLANG_V,
	DCH_SLANG_M1,
	DCH_SLANG_M2,
	DCH_SLANG_M3,
	DCH_SLANG_M4,
	DCH_SLANG_X,
	' ',
	'.',
	DCH_SLANG_EOF,
	DCH_SLANG_END,
	DCH_SLANG_AUP,
	DCH_SLANG_ADOWN,
	DCH_SLANG_HFORE,
	DCH_SLANG_HBACK,
	DCH_SLANG_ALEFT,
	DCH_SLANG_ARIGHT
    };
    //static const char tab_linux1[] =
    //{
    //    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    //    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'j', 'k',
    //    'l', 'm', 'n', 'o', 'p', 'q'
    //};

    if (use_tab == NULL) {
	char *c = getenv("TERM");
        use_tab = ((c == NULL) || strcmp(c, "linux") != 0) ? tab : tab_linux;
        use_tab=GetGUICharacters ("Slang",use_tab);
	use_tab_size = strlen(use_tab);
    }

    assert(idx >= 0 && idx < use_tab_size);

    return use_tab[idx];
}
