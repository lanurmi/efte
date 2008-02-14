/*    o_modemap.cpp
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"
#include "o_modemap.h"

EventMapView *TheEventMapView = 0;

void EventMapView::AddLine(const char *Line) {
    if (BList) {
        BCount++;
        BList = (char **)realloc((void *)BList, sizeof(char *) * BCount);
    } else {
        BCount = 1;
        BList = (char **)malloc(sizeof(char *));
    }
    assert(BList);
    BList[BCount - 1] = strdup(Line);
}

void EventMapView::DumpKey(const char *aPrefix, EKey *Key) {
    char KeyName[128] = "";
    char Entry[2048] = "";
    char *p;
    int id;

    if (aPrefix) {
        strcpy(KeyName, aPrefix);
        strcat(KeyName, "_");
    }
    GetKeyName(KeyName + strlen(KeyName), sizeof(KeyName) - strlen(KeyName), Key->fKey);
    sprintf(Entry, "%13s   ", KeyName);
    id = Key->Cmd;
    for (int i = 0; i < Macros[id].Count; i++) {
        p = Entry + strlen(Entry);
        if (Macros[id].cmds[i].type == CT_COMMAND) {
            if (Macros[id].cmds[i].repeat > 1)
                sprintf(p, "%d:%s ", Macros[id].cmds[i].repeat, GetCommandName(Macros[id].cmds[i].u.num));
            else
                sprintf(p, "%s ", GetCommandName(Macros[id].cmds[i].u.num));
        } else if (Macros[id].cmds[i].type == CT_NUMBER) {
            sprintf(p, "%ld ", Macros[id].cmds[i].u.num);
        } else if (Macros[id].cmds[i].type == CT_STRING) {
            sprintf(p, "'%s' ", Macros[id].cmds[i].u.string);
        } else if (Macros[id].cmds[i].type == CT_CONCAT) {
            strcat(p, ". ");
        } else if (Macros[id].cmds[i].type == CT_VARIABLE) {
            sprintf(p, "$(%ld) ", Macros[id].cmds[i].u.num);
        }
        if (strlen(Entry) > 70) {
            if (i != Macros[id].Count - 1) {
                // not the last entry
                AddLine(Entry);
                sprintf(Entry, "%13s   ", "");
            }
        }
    }
    AddLine(Entry);
}

void EventMapView::DumpMap(const char *aPrefix, EKeyMap *aKeyMap) {
    EKey *Key;

    Key = aKeyMap->fKeys;
    while (Key) {
        if (Key->fKeyMap) {
            char Prefix[32] = "";

            if (aPrefix) {
                strcpy(Prefix, aPrefix);
                strcat(Prefix, "_");
            }
            GetKeyName(Prefix + strlen(Prefix), sizeof(Prefix) - strlen(Prefix), Key->fKey);
            DumpMap(Prefix, Key->fKeyMap);
        } else {
            DumpKey(aPrefix, Key);
        }
        Key = Key->fNext;
    }
}

void EventMapView::DumpEventMap(EEventMap *aEventMap) {
    char name[256];

    while (aEventMap) {
        strcpy(name, aEventMap->Name);
        if (aEventMap->Parent) {
            strcat(name, ": ");
            strcat(name, aEventMap->Parent->Name);
        }
        AddLine(name);
        if (aEventMap->KeyMap)
            DumpMap(0, aEventMap->KeyMap);
        aEventMap = aEventMap->Parent;
        if (aEventMap != 0)
            AddLine("");
    }
}

EventMapView::EventMapView(int createFlags, EModel **ARoot, EEventMap *Map): EList(createFlags, ARoot, "Event Map") {
    BCount = 0;
    BList = 0;
    DumpEventMap(EMap = Map);
    TheEventMapView = this;
}

EventMapView::~EventMapView() {
    FreeView();
    TheEventMapView = 0;
}

void EventMapView::FreeView() {
    if (BList) {
        for (int i = 0; i < BCount; i++)
            if (BList[i])
                free(BList[i]);
        free(BList);
    }
    BCount = 0;
    BList = 0;
}

void EventMapView::ViewMap(EEventMap *Map) {
    FreeView();
    DumpEventMap(EMap = Map);
}

EEventMap *EventMapView::GetEventMap() {
    return FindEventMap("EVENTMAPVIEW");
}

int EventMapView::ExecCommand(int Command, ExState &State) {
    return EList::ExecCommand(Command, State);
}

int EventMapView::GetContext() {
    return CONTEXT_MAPVIEW;
}

void EventMapView::DrawLine(PCell B, int Line, int Col, ChColor color, int Width) {
    if (Line < BCount)
        if (Col < int(strlen(BList[Line])))
            MoveStr(B, 0, Width, BList[Line] + Col, color, Width);
}

char *EventMapView::FormatLine(int Line) {
    return strdup(BList[Line]);
}

void EventMapView::UpdateList() {
    Count = BCount;
    EList::UpdateList();
}

int EventMapView::CanActivate(int /*Line*/) {
    return 0;
}

void EventMapView::GetName(char *AName, int MaxLen) {
    strncpy(AName, "EventMapView", MaxLen);
}

void EventMapView::GetInfo(char *AInfo, int /*MaxLen*/) {
    sprintf(AInfo,
            "%2d %04d/%03d EventMapView (%s)",
            ModelNo,
            Row + 1, Count,
            EMap->Name);
}

void EventMapView::GetTitle(char *ATitle, int /*MaxLen*/, char *ASTitle, int SMaxLen) {
    sprintf(ATitle, "EventMapView: %s", EMap->Name);
    strncpy(ASTitle, "EventMapView", SMaxLen);
    ASTitle[SMaxLen - 1] = 0;
}

