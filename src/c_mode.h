/*    c_mode.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef EMODE_H_
#define EMODE_H_

#define CMD_EXT 0x1000  // max 4096 internal commands, check cfte.cpp

#define CONTEXT_NONE      0
#define CONTEXT_FILE      1
#define CONTEXT_DIRECTORY 2
#define CONTEXT_MESSAGES  3
#define CONTEXT_SHELL     4
#define CONTEXT_INPUT     5
#define CONTEXT_CHOICE    6
#define CONTEXT_LIST      7
#define CONTEXT_CHAR      8
#define CONTEXT_BUFFERS   9
#define CONTEXT_ROUTINES 10
#define CONTEXT_MAPVIEW  11
#define CONTEXT_CVSBASE  12
#define CONTEXT_CVS      13
#define CONTEXT_CVSDIFF  14
#define CONTEXT_SVNBASE  15
#define CONTEXT_SVN      16
#define CONTEXT_SVNDIFF  17

//typedef unsigned char ChColor;
typedef int ChColor;

#define HILIT_PLAIN   0
#define HILIT_C       1
#define HILIT_HTML    2
#define HILIT_MAKE    3
#define HILIT_REXX    4
#define HILIT_DIFF    5
#define HILIT_IPF     6
#define HILIT_PERL    7
#define HILIT_MERGE   8
#define HILIT_ADA     9
#define HILIT_MSG    10
#define HILIT_SH     11
#define HILIT_PASCAL 12
#define HILIT_TEX    13
#define HILIT_FTE    14
#define HILIT_CATBS  15
#define HILIT_SIMPLE 16

#define INDENT_PLAIN  0
#define INDENT_C      1
#define INDENT_REXX   2
#define INDENT_SIMPLE 3

#define BFI_AutoIndent          0
#define BFI_Insert              1
#define BFI_DrawOn              2
#define BFI_HilitOn             3
#define BFI_ExpandTabs          4
#define BFI_Trim                5
#define BFI_TabSize             6
#define BFI_ShowTabs            9
#define BFI_HardMode           15
#define BFI_Undo               16
#define BFI_ReadOnly           17
#define BFI_AutoSave           18
#define BFI_KeepBackups        19
#define BFI_MatchCase          22
#define BFI_BackSpKillTab      23
#define BFI_DeleteKillTab      24
#define BFI_BackSpUnindents    25
#define BFI_SpaceTabs          26
#define BFI_IndentWithTabs     27
#define BFI_SeeThruSel         30
#define BFI_ShowMarkers        32
#define BFI_CursorThroughTabs  33
#define BFI_MultiLineHilit     35

#define BFI_WordWrap           31
#define BFI_LeftMargin         28
#define BFI_RightMargin        29

#define BFI_Colorizer           7
#define BFI_IndentMode          8

#define BFI_LineChar           10
#define BFI_StripChar          11
#define BFI_AddLF              12
#define BFI_AddCR              13
#define BFI_ForceNewLine       14
#define BFI_LoadMargin         20
#define BFI_SaveFolds          34

#define BFI_UndoLimit          21

#define BFI_AutoHilitParen     36
#define BFI_Abbreviations      37
#define BFI_BackSpKillBlock    38
#define BFI_DeleteKillBlock    39
#define BFI_PersistentBlocks   40
#define BFI_InsertKillBlock    41
#define BFI_EventMap           42
#define BFI_UndoMoves          43
#define BFI_DetectLineSep      44
#define BFI_TrimOnSave         45
#define BFI_SaveBookmarks      46
#define BFI_HilitTags          47
#define BFI_ShowBookmarks      48
#define BFI_MakeBackups        49

#define BFI_COUNT              50

#define BFS_RoutineRegexp       (0 | 256)
#define BFS_DefFindOpt          (1 | 256)
#define BFS_DefFindReplaceOpt   (2 | 256)
#define BFS_CommentStart        (3 | 256)
#define BFS_CommentEnd          (4 | 256)
#define BFS_FileNameRx          (5 | 256)
#define BFS_FirstLineRx         (6 | 256)
#define BFS_CompileCommand      (7 | 256)

#define BFS_COUNT               8

#define BFS_WordChars           (100 | 256) // ext
#define BFS_CapitalChars        (101 | 256)

#define BFI(y,x) ((y)->Flags.num[(x) & 0xFF])
#define BFI_SET(y,x,v) ((y)->Flags.num[(x) & 0xFF]=(v))
#define BFS(y,x) ((y)->Flags.str[(x) & 0xFF])

#define WSETBIT(x,y,z) \
    ((x)[(unsigned char)(y) >> 3] = char((z) ? \
    ((x)[(unsigned char)(y) >> 3] |  (1 << ((unsigned char)(y) & 0x7))) : \
    ((x)[(unsigned char)(y) >> 3] & ~(1 << ((unsigned char)(y) & 0x7)))))

#define WGETBIT(x,y) \
    (((x)[(unsigned char)(y) / 8] &  (1 << ((unsigned char)(y) % 8))) ? 1 : 0)

typedef struct {
    int num[BFI_COUNT];
    char *str[BFS_COUNT];
    char WordChars[32];
    char CapitalChars[32];
} EBufferFlags;

extern EBufferFlags DefaultBufferFlags;

/* globals */
#define FLAG_C_Indent            1
#define FLAG_C_BraceOfs          2
#define FLAG_REXX_Indent         3
#define FLAG_ScreenSizeX         6
#define FLAG_ScreenSizeY         7
#define FLAG_SysClipboard       12
#define FLAG_ShowHScroll        13
#define FLAG_ShowVScroll        14
#define FLAG_ScrollBarWidth     15
#define FLAG_SelectPathname     16
#define FLAG_C_CaseOfs          18
#define FLAG_DefaultModeName    19
#define FLAG_CompletionFilter   20
#define FLAG_ShowMenuBar        22
#define FLAG_C_CaseDelta        23
#define FLAG_C_ClassOfs         24
#define FLAG_C_ClassDelta       25
#define FLAG_C_ColonOfs         26
#define FLAG_C_CommentOfs       27
#define FLAG_C_CommentDelta     28
#define FLAG_OpenAfterClose     30
#define FLAG_PrintDevice        31
#define FLAG_CompileCommand     32
#define FLAG_REXX_Do_Offset     33
#define FLAG_KeepHistory        34
#define FLAG_LoadDesktopOnEntry 35
#define FLAG_SaveDesktopOnExit  36
#define FLAG_WindowFont         37
#define FLAG_KeepMessages       38
#define FLAG_ScrollBorderX      39
#define FLAG_ScrollBorderY      40
#define FLAG_ScrollJumpX        41
#define FLAG_ScrollJumpY        42
#define FLAG_ShowToolBar        43
#define FLAG_GUIDialogs         44
#define FLAG_PMDisableAccel     45
#define FLAG_SevenBit           46
#define FLAG_WeirdScroll        47
#define FLAG_LoadDesktopMode    48
#define FLAG_HelpCommand        49
#define FLAG_C_FirstLevelIndent 50
#define FLAG_C_FirstLevelWidth  51
#define FLAG_C_Continuation     52
#define FLAG_C_ParenDelta       53
#define FLAG_FunctionUsesContinuation 54
#define FLAG_IgnoreBufferList   55
#define FLAG_GUICharacters      56
#define FLAG_CvsCommand         57
#define FLAG_CvsLogMode         58
#define FLAG_ReassignModelIds   59
#define FLAG_RecheckReadOnly    60
#define FLAG_XShellCommand      61
#define FLAG_RGBColor  62
#define FLAG_CursorBlink        63
#define FLAG_SvnCommand         64
#define FLAG_SvnLogMode         65
#define FLAG_CursorWithinEOL    66
#define FLAG_CursorInsertMask   67
#define FLAG_CursorOverMask     68
#define FLAG_BackupDirectory    69

#define EM_MENUS 2
#define EM_MainMenu 0
#define EM_LocalMenu 1

#define COL_SyntaxParser 1

#define CLR_Normal         0
#define CLR_Keyword        1
#define CLR_String         2
#define CLR_Comment        3
#define CLR_CPreprocessor  4
#define CLR_Regexp         5
#define CLR_Header         6
#define CLR_Quotes         7
#define CLR_Number         8
#define CLR_HexNumber      9
#define CLR_OctalNumber   10
#define CLR_FloatNumber   11
#define CLR_Function      12
#define CLR_Command       13
#define CLR_Tag           14
#define CLR_Punctuation   15
#define CLR_New           16
#define CLR_Old           17
#define CLR_Changed       18
#define CLR_Control       19
#define CLR_Separator     20
#define CLR_Variable      21
#define CLR_Symbol        22
#define CLR_Directive     23
#define CLR_Label         24
#define CLR_Special       25
#define CLR_QuoteDelim    26
#define CLR_RegexpDelim   27

#define COUNT_CLR         28

#define MATCH_MUST_BOL     0x0001
#define MATCH_MUST_BOLW    0x0002
#define MATCH_MUST_EOL     0x0004
#define MATCH_MUST_EOLW    0x0008
#define MATCH_NO_CASE      0x0010
#define MATCH_SET          0x0020
#define MATCH_NOTSET       0x0040
#define MATCH_QUOTECH      0x0100
#define MATCH_QUOTEEOL     0x0200
#define MATCH_NOGRAB       0x0400
#define MATCH_NEGATE       0x0800
#define MATCH_TAGASNEXT    0x1000
#define MATCH_REGEXP       0x2000

#define ACTION_NXSTATE     0x0001

#define STATE_NOCASE       0x0001
#define STATE_TAGASNEXT    0x0002
#define STATE_NOGRAB       0x0004

typedef enum {
    mvFilePath = 1,  /* directory + name + extension */
    mvFileName,      /* name + extension */
    mvFileDirectory, /* directory + '/' */
    mvFileBaseName,  /* without the last extension */
    mvFileExtension, /* the last one */
    mvCurDirectory,
    mvCurRow,
    mvCurCol,
    mvChar,
    mvWord,
    mvLine,
    mvFTEVer,
    mvGet0,
    mvGet1,
    mvGet2,
    mvGet3,
    mvGet4,
    mvGet5,
    mvGet6,
    mvGet7,
    mvGet8,
    mvGet9
} MacroVariable;

#endif
