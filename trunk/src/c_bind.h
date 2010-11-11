/*    c_bind.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef BIND_H_
#define BIND_H_


#include "console.h"
#include "o_model.h"
#include "c_mode.h" // EM_MENUS
#include "e_regex.h"

#define ABBREV_HASH      16

class EMode;
class EEventMap;
class EKeyMap;
class EKey;
class EAbbrev;
class EView;
class EColorize;

typedef struct {
    TKeyCode Mask;
    TKeyCode Key;
} KeySel;

typedef struct {
    RxNode *regex;
    int look_line;
    int affect_line;
    int indent;
    int flags;
} IndentRx;

class EMode {
public:
    EMode *fNext;
    char *fName;
    char *MatchName;
    char *MatchLine;
    RxNode *MatchNameRx;
    RxNode *MatchLineRx;
    EBufferFlags Flags;
    EEventMap *fEventMap;
    EMode *fParent;
    EColorize *fColorize;
    IndentRx indents[25];
    int indent_count;
    char filename[256];

    EMode(EMode *aMode, EEventMap *Map, const char *aName);
    ~EMode();
    EAbbrev *FindAbbrev(const char *string);
    void AddIndentRx(int look_line, int affect_line, int indent, const char *regex, int flags);
};

class EKeyMap {
public:
    EKeyMap *fParent;
    EKey *fKeys;

    EKeyMap();
    ~EKeyMap();

    void AddKey(EKey *aKey);
    EKey *FindKey(TKeyCode aKey);
};

class EEventMap {
public:
    EEventMap *Next;
    EEventMap *Parent;
    char *Name;

    EKeyMap *KeyMap;
    char *Menu[EM_MENUS]; // main + local

    EAbbrev *abbrev[ABBREV_HASH];

    EEventMap(const char *AName, EEventMap *AParent);
    ~EEventMap();
    void SetMenu(int which, const char *What);
    char *GetMenu(int which);
    int AddAbbrev(EAbbrev *ab);
};

#define CT_COMMAND  0
#define CT_NUMBER   1
#define CT_STRING   2
#define CT_VARIABLE 3
#define CT_CONCAT   4 /* concatenate strings */

typedef struct {
    int type;
    short repeat;
    short ign;
    union {
        long num;
        char *string;
    } u;
} CommandType;

typedef struct {
    char *Name;
    int Count;
    CommandType *cmds;
} ExMacro;

class EKey {
public:
    KeySel fKey;
    int Cmd;
    EKeyMap *fKeyMap;
    EKey *fNext;

    EKey(char *aKey);
    EKey(char *aKey, EKeyMap *aKeyMap);
    ~EKey();
};

class EAbbrev {
public:
    EAbbrev *next;
    int Cmd;
    char *Match;
    char *Replace;

    EAbbrev(const char *aMatch, const char *aReplace);
    EAbbrev(const char *aMatch, int aCmd);
    ~EAbbrev();
};

class ExState { // state of macro execution
public:
    int Macro;
    int Pos;

    int GetStrParam(EView *view, char *str, int buflen);
    int GetIntParam(EView *view, int *value);
};

extern EMode *Modes;
extern EEventMap *EventMaps;

extern int CMacros;
extern ExMacro *Macros;

int GetCharFromEvent(TEvent &E, char *Ch);

const char *GetCommandName(int Command);
EMode *FindMode(const char *Name);
EEventMap *FindEventMap(const char *Name);
EEventMap *FindActiveMap(EMode *Mode);
EMode *GetModeForName(const char *FileName);
int CmdNum(const char *Cmd);
void ExecKey(EKey *Key);
EKey *SetKey(EEventMap *aMap, const char *Key);
int ParseKey(const char *Key, KeySel &ks);
int GetKeyName(char *Key, int KeySize, KeySel &ks);

void DefineWord(const char *s);
int NewCommand(const char *Name);
int RunCommand(int Command);
int AddCommand(int no, int cmd, int count, int ign);
int AddString(int no, const char *Command);
int AddNumber(int no, long number);
int AddVariable(int no, int number);
int AddConcat(int no);
int HashStr(const char *str, int maxim);
void SetWordChars(char *w, const char *s);

#endif
