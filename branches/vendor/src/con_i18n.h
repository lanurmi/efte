#ifndef __CONI18N_H__
#define __CONI18N_H__

#include <X11/Xlib.h>
#include <X11/Xlocale.h>

/*
 * For now the only supported input style is root !!!
 * in future this should be read from resources
 */
#define XIM_INPUT_STYLE "Root"

struct remapKey {
    KeySym key_english;
    KeySym key_remap;
};

struct keyboardRec {
    struct remapKey *tab;
    KeySym deadkey;
    short next;
};

/*
 * prototypes for I18N functions
 */
extern void I18NFocusOut(XIC);
extern void I18NFocusIn(XIC);
extern int I18NLookupString(XKeyEvent *, char *, int, KeySym *, XIC);
extern XIC I18NInit(Display *, Window, unsigned long *);

#endif
