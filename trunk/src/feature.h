/*    feature.h
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/* some stuff does not yet work */

#ifndef __FEATURE_H__
#define __FEATURE_H__

#undef CONFIG_EMULATE_VI // todo

#define CONFIG_CONFIGURABLE
#define CONFIG_MOUSE
#define CONFIG_CLIPBOARD
#define CONFIG_SHELL

#define CONFIG_MFRAMES
#define CONFIG_MWINDOWS
#define CONFIG_MBUFFERS
#define CONFIG_MENUS
#define CONFIG_SCROLLBARS
#define CONFIG_I_SEARCH
#define CONFIG_I_ASCII
#define CONFIG_HISTORY
#define CONFIG_DESKTOP

#define CONFIG_BLOCK_STREAM
#define CONFIG_BLOCK_COLUMN
#define CONFIG_BLOCK_LINE
#define CONFIG_IOBLOCKS
#define CONFIG_PRINTING
#define CONFIG_BOOKMARKS
#define CONFIG_WORDWRAP
#define CONFIG_ABBREV
#define CONFIG_TAGS

#define CONFIG_UNDOREDO
#define CONFIG_REGEXPS
#define CONFIG_FOLDS

#undef CONFIG_OBJ_HEXEDIT            // todo
#undef CONFIG_OBJ_VIEWER             // todo
#define CONFIG_OBJ_LIST
#define CONFIG_OBJ_FILE

#ifdef CONFIG_OBJ_LIST
#define CONFIG_OBJ_DIRECTORY
#define CONFIG_OBJ_ROUTINE
#define CONFIG_OBJ_BUFFERS
#define CONFIG_OBJ_MESSAGES
#define CONFIG_OBJ_CVS
#endif

#define CONFIG_SYNTAX_HILIT
#define CONFIG_WORD_HILIT

#ifdef CONFIG_SYNTAX_HILIT
#define CONFIG_HILIT_C
#define CONFIG_HILIT_REXX
#define CONFIG_HILIT_PERL
#define CONFIG_HILIT_ADA
#define CONFIG_HILIT_MAKE
#define CONFIG_HILIT_IPF
#define CONFIG_HILIT_MSG
#define CONFIG_HILIT_SH
#define CONFIG_HILIT_PASCAL
#define CONFIG_HILIT_TEX
#define CONFIG_HILIT_FTE
#define CONFIG_HILIT_CATBS
#define CONFIG_HILIT_SIMPLE
#endif

#if defined(CONFIG_HILIT_C)
#define CONFIG_INDENT_C
#endif

#if defined(CONFIG_HILIT_SIMPLE)
#define CONFIG_INDENT_SIMPLE
#endif

#if defined(CONFIG_HILIT_REXX)
#define CONFIG_INDENT_REXX
#endif

#define CONFIG_I_COMPLETE
#endif
