/*    g_menu.cpp
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <stdarg.h>
#if defined(MSVC)
#include <malloc.h>
#endif
#include "console.h"
#include "gui.h"

int MenuCount = 0;
mMenu *Menus = 0;

int NewMenu(const char *Name) {
    int n;

    Menus = (mMenu *) realloc((void *) Menus,
                              sizeof(mMenu) * (MenuCount + 1));
    n = MenuCount;

    Menus[n].Name = strdup(Name);
    Menus[n].Count = 0;
    Menus[n].Items = 0;

    MenuCount++;
    return n;
}

int NewItem(int menu, const char *Name) {
    int n;

    assert(menu < MenuCount);

    Menus[menu].Items = (mItem *) realloc(Menus[menu].Items,
                                          sizeof(mItem) * (Menus[menu].Count + 1));
    n = Menus[menu].Count;

    Menus[menu].Items[n].SubMenu = -1;
    Menus[menu].Items[n].Name = Name ? strdup(Name) : 0;
    Menus[menu].Items[n].Arg = 0;
    Menus[menu].Items[n].Cmd = -1;

    Menus[menu].Count++;
    return n;
}

int NewSubMenu(int menu, const char *Name, int submenu, int Type) {
    int n;

    assert(menu < MenuCount);

    Menus[menu].Items = (mItem *) realloc(Menus[menu].Items,
                                          sizeof(mItem) * (Menus[menu].Count + 1));
    n = Menus[menu].Count;

    Menus[menu].Items[n].SubMenu = submenu;
    Menus[menu].Items[n].Name = Name ? strdup(Name) : 0;
    Menus[menu].Items[n].Arg = 0;
    Menus[menu].Items[n].Cmd = Type;

    Menus[menu].Count++;
    return n;
}

int GetMenuId(const char *Name) {
    if (Name)
        for (int i = 0; i < MenuCount; i++)
            if (strcmp(Name, Menus[i].Name) == 0)
                return i;
    return -1;
}

