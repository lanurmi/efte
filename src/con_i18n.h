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

typedef struct {
    XIC xic;
#if XlibSpecificationRelease >= 6
    XIM xim;
    XIMStyles* xim_styles;
    XIMStyle input_style;
#endif
} i18n_context_t;

/*
 * prototypes for I18N functions
 */
void i18n_focus_out(i18n_context_t*);
void i18n_focus_in(i18n_context_t*);
int i18n_lookup_sym(XKeyEvent *, char *, int, KeySym *, XIC);
i18n_context_t* i18n_open(Display *, Window, unsigned long *);
void i18n_destroy(i18n_context_t*);

#endif
