/*    c_mode.cpp
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

EBufferFlags DefaultBufferFlags = {
    {
        1,                  // AutoIndent
        1,                  // InsertOn
        0,                  // DrawOn
        1,                  // HilitOn
        1,                  // ExpandTabs
        0,                  // Trim
        8,                  // TabSize
        HILIT_PLAIN,        // HilitMode
        INDENT_PLAIN,       // IndentMode
        0,                  // ShowTab
        10,                 // LineChar
#if !defined(UNIX)
        13,                 // StripChar
#else
        -1,
#endif
        1,                  // AddLine
#if !defined(UNIX)
        1,                  // AddStrip
#else
        0,
#endif
        0,                  // ForceNewLine
        0,                  // HardMode
        1,                  // UndoRedo
        0,                  // ReadOnly
        0,                  // AutoSave
        1,                  // KeepBackups
        -1,                 // LoadMargin
        256,                // Max Undo/Redo Commands
        1,                  // MatchCase
        0,                  // BackKillTab
        0,                  // DelKillTab
        1,                  // BackSpUnindent
        0,                  // SpaceTabs
        1,                  // IndentWTabs
        1,                  // Wrap.LeftMargin
        72,                 // Wrap.RightMargin
        0,                  // See Thru Sel
        0,                  // WordWrap
        0,                  // ShowMarkers
        1,                  // CursorThroughTabs
        0,                  // Save Folds
        0,                  // MultiLineHilit
        0,                  // AutoHilitParen
        0,                  // Abbreviations
        0,                  // BackSpKillBlock
        0,                  // DeleteKillBlock
        1,                  // PersistentBlocks
        0,                  // InsertKillBlock
        0,                  // EventMap
        0,                  // UndoMoves
#ifdef UNIX
        0,                  // DetectLineSep
#else
        1,
#endif
        0,                  // trim on save
        0,                  // save bookmarks
        1,                  // HilitTags
        0,                  // ShowBookmarks
        1                   // MakeBackups
    },
    {
        0,                  // Routine Regexp
        0,                  // DefFindOpt
        0,                  // DefFindReplaceOpt
        0,                  // comment start (folds)
        0,                  // comment end (folds)
        0,                  // filename rx
        0,                  // firstline rx
        0                   // compile command
    }
};

EMode *GetModeForName(const char *FileName) {
    //    char ext[10];
    //    char *p;
    int l, i;
    EMode *m;
    RxMatchRes RM;
    char buf[81];
    int fd;

    m = Modes;
    while (m) {
        if (m->MatchNameRx)
            if (RxExec(m->MatchNameRx,
                       FileName, strlen(FileName), FileName,
                       &RM, RX_CASE) == 1)
                return m;
        if (m->fNext == 0) break;
        m = m->fNext;
    }

    fd = open(FileName, O_RDONLY);
    if (fd != -1) {
        l = read(fd, buf, 80);
        close(fd);
        if (l > 0) {
            buf[l] = 0;
            for (i = 0; i < l; i++) {
                if (buf[i] == '\n') {
                    buf[i] = 0;
                    l = i;
                    break;
                }
            }
            m = Modes;
            while (m) {
                if (m->MatchLineRx)
                    if (RxExec(m->MatchLineRx, buf, l, buf, &RM, RX_CASE) == 1)
                        return m;
                if (m->fNext == 0) break;
                m = m->fNext;
            }
        }
    }

    if ((m = FindMode(DefaultModeName)) != 0) return m;

    m = Modes;
    while (m && m->fNext) m = m->fNext;
    return m;
}
