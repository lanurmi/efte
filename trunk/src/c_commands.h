/*    c_commands.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

typedef enum {
    ErFAIL = 0,
    ErOK = 1
} ExResult;

typedef enum {
    //!Non macro commands
    // { decoded in executor loop
    ExNop,
    //    { accessing macro internals one way or another
    ExUnconditionalBranch,
    /// unconditional branch, offset from command repeat count
    ExConditionalBranch,
    /// conditional branch, offset from command repeat count
    ExDoRuntime,
    /// macro run time code associate with DO
    ExLoopRuntime,
    /// macro run time code associate with LOOP
    ExPlusLoopRuntime,
    /// macro run time code associate with PLUSLOOP
    ExMinLoopRuntime,
    /// macro run time code associate with MINLOOP
    ExLeaveRuntime,
    /// macro run time code for breaking out of a loop
    ExTimes,
    /// set repeat counter of next command to top of stack
    ExOld,
    /// excuted by "converted" data structure accessor, to publish data location
    ExNew,
    /// converts a sub to a data structure accessor
    //    }
    // }
    //*** START

    ExDoes,
    /// seperates instance creation time code from instance run time code, signaling "new" where to find it. used in classes

    //!Stack Operations
    ExDepth,
    /// Depth of param stack
    ExExit,
    /// Exit the macro.
    ExPlus,
    /// Add top two stack items.
    ExMinus,
    /// Subtract [[Tos]] from [[Nos]].
    ExMul,
    /// Multiply top two stack items.
    ExDiv,
    /// Divide top two stack items.

    ExRandom,
    /// generate random number.
    ExMillisecs,
    /// time stamp, returning milliseconds
    ExMicrosecs,
    /// time stamp, returning microseconds

    ExAnd,
    /// And
    ExOr,
    /// Or
    ExXor,
    /// Xor
    ExShift,
    /// logical shift either direction.
    ExEquals,
    /// Compare top two stack items for equality.
    ExLess,
    /// Compare top two stack items for less than.
    ExFlag,
    /// Reflect condition register in [[Tos]].
    ExFail,
    /// Return last condition from register, causing macro interpreter
    /// to terminate macro execution if condition was a failure.
    ExDup,
    /// Duplicate top stack item.
    ExDrop,
    /// Remove top stack item.
    ExSwap,
    /// Swap top two stack items.
    ExOver,
    /// Push second from top item to stack.
    ExRot,
    /// Rotate third stack item to top.
    // -------------------
    ExToR,
    /// Move one item from param stack to control stack
    ExRFrom,
    /// Move one item from control stack to param stack
    ExRFetch,
    /// Duplicate top control stack item to param stack
    ExI,
    /// place loop index of innermost loop on stack
    ExJ,
    /// place loop index of 2nd loop on stack
    // -------------------
    ExDiag,
    /// Print stack top and conditions to stderr for debugging.
    ExStore,
    /// Store tos into into memory
    ExFetch,
    /// Push specified location in memory onto the stack
    ExMemEnd,
    /// Push the memory end to the stack
    ExHere,
    /// Push the dictonary pointer to stack (though we don't have a dictionary where the pointer points to...)
    ExAllot,
    /// Reserve an amount of memory, by bumping dictionary pointer
    ExDump,
    /// Dump memory contents
    ExToggleConditionDisplay,
    /// Toggle displaying the condition code in the status bar.

    //!String Stack Commands
    ExDiagStr,
    /// Print string stack info
    ExDupStr,
    /// Duplicate StrTos
    ExDropStr,
    /// Drop top from string stack
    ExSwapStr,
    /// Swap StrTos and StrNos
    ExCompareStr,
    /// Compare StrTos and StrNos
    ExOverStr,
    /// Push a copy of StrNos
    ExPickStr,
    /// Pick a copy of the string and push
    ExDepthStr,
    /// Push the number of strings on the stack
    ExSubSearchStr,
    /// Substring search through stringstack
    ExSplitStr,
    /// Split a string into two on the stack at the given location
    ExMergeStr,
    /// Merge StrTos and StrNos by appending StrTos onto StrNos
    ExRotStr,
    /// Rotate third stack item to top
    ExLenStr,
    /// Length of string
    ExMidStr,
    /// Get a substring of a string
    ExGetString,
    /// Get a string from the user and place onto the string stack

    //!Cursor Commands
    ExCursorLeft,
    /// Move the cursor left.
    ExCursorRight,
    /// Move the cursor right.
    ExCursorUp,
    /// Simple move up.
    ExCursorDown,
    /// Simple move down.
    ExMoveDown,
    /// Move cursor to next line.
    ///
    /// See variable: CursorWithinEOL
    ExMoveUp,
    /// Move cursor to previous line
    //
    // See variable: CursorWithinEOL
    ExMoveLeft,
    /// Move cursor to previous column.
    ///
    /// See variable: CursorWithinEOL
    ExMoveRight,
    /// Move cursor to next column.
    ///
    /// See variable: CursorWithinEOL
    ExMovePrev,
    /// Move cursor to previous character or if the cursor is at the beginning of the line,
    /// move to the end of the previous line.
    ExMoveNext,
    /// Move cursor to next character or if the cursor is at the end of the line, move to
    /// the beginning of the next line.
    ExMoveWordLeft,
    /// Move cursor to the beginning of the word on the left.
    ExMoveWordRight,
    /// Move cursor to the beginning of the word on the right.
    ExMoveWordPrev,
    /// Move cursor to the beginning of the previous word.
    ExMoveWordNext,
    /// Move cursor to the beginning of the next word.
    ExMoveWordEndLeft,
    /// Move cursor to the end of the previous word.
    ExMoveWordEndRight,
    /// Move cursor to the end of the word on the right.
    ExMoveWordEndPrev,
    /// Move cursor to the end of the previous word.
    ExMoveWordEndNext,
    /// Move cursor to the end of the next word.
    ExMoveWordOrCapLeft,
    /// Move cursor to the beginning of the word or capital letter on the right.
    ExMoveWordOrCapRight,
    /// Move cursor to the beginning of the word or capital letter on the left.
    ExMoveWordOrCapPrev,
    /// Move cursor to the beginning of the previous word or to previous
    /// capital letter.
    ExMoveWordOrCapNext,
    /// Move cursor to the beginning of the next word or to next capital letter.
    ExMoveWordOrCapEndLeft,
    /// Move cursor to the end of the word or capitals on the left.
    ExMoveWordOrCapEndRight,
    /// Move cursor to the end of the word or capitals on the right.
    ExMoveWordOrCapEndPrev,
    /// Move cursor to the end of the previous word or capitals.
    ExMoveWordOrCapEndNext,
    /// Move cursor to the end of the next word or capitals.
    ExMoveLineStart,
    /// Move cursor to the beginning of line.
    ExMoveLineEnd,
    /// Move cursor to the end of line.
    ExMovePageStart,
    /// Move cursor to the first line on current page.
    ExMovePageEnd,
    /// Move cursor to the last line on currently page.
    ExMovePageUp,
    /// Display previous page.
    ExMovePageDown,
    /// Display next page.
    ExMoveFileStart,
    /// Move cursor to the beginning of file.
    ExMoveFileEnd,
    /// Move cursor to the end of file.
    ExMovePageLeft,
    /// Scroll horizontally to display page on the left.
    ExMovePageRight,
    /// Scroll horizontally to display page on the right.
    ExMoveBlockStart,
    /// Move cursor to the beginning of block.
    ExMoveBlockEnd,
    /// Move cursor to end beginning of block.
    ExMoveFirstNonWhite,
    /// Move cursor to the first non-blank character on line.
    ExMoveLastNonWhite,
    /// Move cursor to the last non-blank character on line.
    ExMovePrevEqualIndent,
    /// Move cursor to the previous line with equal indentation.
    ExMoveNextEqualIndent,
    /// Move cursor to the next line with equal indentation.
    ExMovePrevTab,
    /// Move cursor to the previous tab position.
    ExMoveNextTab,
    /// Move cursor to the next tab position.
    ExMoveTabStart,
    /// When cursor is on the tab characters, moves it to the beginning
    /// of the tab.
    ExMoveTabEnd,
    /// When cursor is on the tab characters, moves it to the end
    /// of the tab.
    ExMoveLineTop,
    /// Scroll the file to make the current line appear on the top of the window.
    ExMoveLineCenter,
    /// Scroll the file to make the current line appear on the center of the window.
    ExMoveLineBottom,
    /// Scroll the file to make the current line appear on the bottom of the window.
    ExScrollLeft,
    /// Scroll screen left.
    ExScrollRight,
    /// Scroll screen right.
    ExScrollDown,
    /// Scroll screen down.
    ExScrollUp,
    /// Scroll screen up.
    ExMoveFoldTop,
    /// Move to the beginning of current fold.
    ExMoveFoldPrev,
    /// Move to the beginning of previous fold.
    ExMoveFoldNext,
    /// Move to the beginning of next fold.
    ExMoveBeginOrNonWhite,
    /// Move to beginning of line, or to first non blank character.
    ExMoveBeginLinePageFile,
    /// Move to the beginning of line, if there already, move to the beginning
    /// page, if there already, move to the beginning of file.
    ExMoveEndLinePageFile,
    /// Move to the end of line, if there already, move to the end
    /// page, if there already, move to the end of file.
    ExMoveToLine,
    /// Move to line number given as argument.
    ExMoveToColumn,
    /// Move to column given as argument.
    ExMoveSavedPosCol,
    /// Move to column from saved position.
    ExMoveSavedPosRow,
    /// Move to line from saved position.
    ExMoveSavedPos,
    /// Move to saved position.
    ExSavePos,
    /// Save current cursor position.
    ExMovePrevPos,
    /// Move to last cursor position.
    // ExCursorPush,
    // ExCursorPop,

    //!Deleting Commands
    ExKillLine,
    /// Delete current line. If the line is the last line in the file,
    /// only the text is deleted.
    ExKillChar,
    /// Delete character under (after) cursor.
    ExKillCharPrev,
    /// Delete character before cursor.
    ExKillWord,
    /// Delete the word after cursor.
    ExKillWordPrev,
    /// Delete the word before cursor.
    ExKillWordOrCap,
    /// Delete word or capitals after cursor.
    ExKillWordOrCapPrev,
    /// Delete word or capitals before cursor.
    ExKillToLineStart,
    /// Delete characters to the beginning of line.
    ExKillToLineEnd,
    /// Delete characters to the end of line.
    ExKillBlock,
    /// Delete block.
    ExKillBlockOrChar,
    /// If block is marked, delete it, otherwise delete character under cursor.
    ExKillBlockOrCharPrev,
    /// If block is marked, delete it, otherwise delete character before cursor.
    ExDelete,
    /// Delete character under (after) cursor.
    ///
    /// See variable: DeleteKillTab and DeleteKillBlock
    ExBackSpace,
    /// Delete character before cursor.
    ///
    /// See variable: BackSpKillTab and BackSpKillBlock

    //!Line Commands
    ExLineInsert,
    /// Insert a new line before the current one.
    ExLineAdd,
    /// Add a new line after the current one.
    ExLineSplit,
    /// Split current line after cursor position.
    ExLineJoin,
    /// Join current line with next one. If cursor is positioned beyond
    /// the end of line, the current line is first padded with whitespace.
    ExLineNew,
    /// Append a new line and move to the beginning of new line.
    ExLineIndent,
    /// Reindent current line.
    ExLineTrim,
    /// Trim whitespace at the end of current line.
    ExLineDuplicate,
    /// Duplicate the current line.
    ExLineCenter,
    /// Center the current line.

    //!Block Commands
    ExSelectionStr,
    /// Push the selection onto the string stack
    ExBlockBegin,
    /// Set block beginning to current position.
    ExBlockEnd,
    /// Set block end to current position.
    ExBlockUnmark,
    /// Unmark block.
    ExBlockCut,
    /// Cut selected block to clipboard.
    ExBlockCopy,
    /// Copy selected block to clipboard.
    ExBlockCutAppend,
    /// Cut selected block and append it to clipboard.
    ExBlockCopyAppend,
    /// Append selected block to clipboard.
    ExBlockClear,
    /// Clear selected block.
    ExBlockPaste,
    /// Paste clipboard to current position.
    ExBlockKill,
    /// Delete selected text.
    ExBlockIndent,
    /// Indent block by 1 character.
    ExBlockUnindent,
    /// Unindent block by 1 character.
    ExBlockMarkStream,
    /// Start/stop marking stream block.
    ExBlockMarkLine,
    /// Start/stop marking line block.
    ExBlockMarkColumn,
    /// Start/stop marking column block.
    ExBlockExtendBegin,
    /// Start extending selected block.
    ExBlockExtendEnd,
    /// Stop extending selected block.
    ExBlockReIndent,
    /// Reindent entire block, if SmartIndent is supported by the current mode.
    ExBlockSelectWord,
    /// Select word under cursor as block.
    ExBlockSelectLine,
    /// Select current line as block.
    ExBlockSelectPara,
    /// Select current paragraph (delimited by blank lines) as block.
    ExBlockPasteStream,
    /// Paste clipboard to current position as stream block.
    ExBlockPasteLine,
    /// Paste clipboard to current position as line block.
    ExBlockPasteColumn,
    /// Paste clipboard to current position as column block.
    ExBlockPrint,
    /// Print a block to configured device.
    ExBlockRead,
    /// Read block from file.
    ExBlockReadStream,
    /// Read block from file as stream block.
    ExBlockReadLine,
    /// Read block from file as line block.
    ExBlockReadColumn,
    /// Read block from file as column block.
    ExBlockWrite,
    /// Write marked block to file.
    ExBlockSort,
    /// Sorts the marked block in ascending order.
    ///
    /// If mode setting MatchCase is set, characters will be compared case
    /// sensitively.
    ///
    /// When block is marked in BlockMarkStream or BlockMarkLine mode,
    /// the entire lines in marked block will be compared.
    ///
    /// When block is marked in BlockMarkColumn mode, only characters within marked
    /// columns will be compared.
    ExBlockSortReverse,
    /// Sorts the marked block in descending order.
    ///
    /// See also: BlockSort
    ExBlockUnTab,
    /// Remove tabs from marked lines.
    ExBlockEnTab,
    /// Generate and optimize tabs in marked lines.
    ExBlockMarkFunction,
    /// Mark current function as block.
    ExBlockTrim,
    /// Trim end-of-line whitespace.

    //!Text Editing Commands
    ExUndo,
    /// Undo last operation.
    ExRedo,
    /// Redo last undone operation.

    //!Folding Commands
    ExFoldCreate,
    /// Create fold.
    ExFoldCreateByRegexp,
    /// Create folds at lines matching a regular expression.
    ExFoldCreateAtRoutines,
    /// Create folds at lines matching RoutineRx.
    ExFoldDestroy,
    /// Destroy fold at current line.
    ExFoldDestroyAll,
    /// Destroy all folds in the file.
    ExFoldPromote,
    /// Promote fold to outer level.
    ExFoldDemote,
    /// Demote fold to inner level.
    ExFoldOpen,
    /// Open fold at current line.
    ExFoldOpenNested,
    /// Open fold and nested folds.
    ExFoldClose,
    /// Close current fold.
    ExFoldOpenAll,
    /// Open all folds in the file.
    ExFoldCloseAll,
    /// Close all folds in the file.
    ExFoldToggleOpenClose,
    /// Toggle open/close current fold.

    //!Bookmark Commands
    ExPlaceBookmark,
    /// Place a file-local bookmark.
    ExRemoveBookmark,
    /// Place a file-local bookmark.
    ExGotoBookmark,
    /// Go to file-local bookmark location.
    ExPlaceGlobalBookmark,
    /// Place global (persistent) bookmark.
    ExRemoveGlobalBookmark,
    /// Remove global bookmark.
    ExGotoGlobalBookmark,
    /// Go to global bookmark location.
    ExPushGlobalBookmark,
    /// Push global bookmark (named as #<num>) to bookmark stack.
    ExPopGlobalBookmark,
    /// Pop global bookmark from bookmark stack.

    //!Character Translation Commands
    ExCharCaseUp,
    /// Convert current character to uppercase.
    ExCharCaseDown,
    /// Convert current character to lowercase.
    ExCharCaseToggle,
    /// Toggle case of current character.
    ExCharTrans,
    /// Translate current character (like perl/sed).
    ExLineCaseUp,
    /// Convert current line to uppercase.
    ExLineCaseDown,
    /// Convert current line to lowercase.
    ExLineCaseToggle,
    /// Toggle case of current line.
    ExLineTrans,
    /// Translate characters on current line.
    ExBlockCaseUp,
    /// Convert characters in selected block to uppercase.
    ExBlockCaseDown,
    /// Convert characters in selected block to lowercase.
    ExBlockCaseToggle,
    /// Toggle case of characters in selected block.
    ExBlockTrans,
    /// Translate characters in selected block.
    ExInsertString,
    /// Insert argument string at cursor position.
    ExInsertSpace,
    /// Insert space
    ExInsertChar,
    /// Insert character argument at cursor position.
    ExTypeChar,
    /// Insert character at cursor position (expanding any abbreviations).
    ExInsertTab,
    /// Insert tab character at cursor position.
    ExInsertSpacesToTab,
    /// Insert appropriate number of spaces to simulate a tab.
    ExSelfInsert,
    /// Insert typed character.
    ExGetChar,
    /// Get a character from the user and push it onto the stack.
    ExWrapPara,
    /// Wrap current paragraph.
    ExInsPrevLineChar,
    /// Insert character in previous line above cursor.
    ExInsPrevLineToEol,
    /// Insert previous line from cursor to end of line.
    ExCompleteWord,
    /// Complete current word to last word starting with the
    /// same prefix.


    //!File Commands
    ExFilePrev,
    /// Switch to previous file in ring.
    ExFileNext,
    /// Switch to next file in ring.
    ExFileLast,
    /// Exchange last two files in ring.
    ExSwitchTo,
    /// Switch to numbered buffer given as argument.
    ExFileOpen,
    /// Open a file.
    ExFileOpenInMode,
    /// Open a file in specified mode.
    ExFileReload,
    /// Reload the current file.
    ExFileSave,
    /// Save the current file.
    ExFileSaveAll,
    /// Save all modified files.
    ExFileSaveAs,
    /// Save the current file to a different name.
    ///
    /// See also: FileWriteTo
    ExFileWriteTo,
    /// Write the current file into another file.
    //
    // See also: FileSaveAs
    ExFilePrint,
    /// Print the current file.
    ExFileClose,
    /// Close the current file.
    ExFileCloseAll,
    /// Close all open files.
    ExFileTrim,
    /// Trim all end-of-line whitespace.

    //!Directory Commands
    ExDirOpen,
    /// Open the directory browser.
    ExDirGoUp,
    /// Change to the parent directory.
    ExDirGoDown,
    /// Change to the currently selected directory.
    ExDirGoRoot,
    /// Change to the root directory.
    ExDirGoto,
    /// Change to the directory given as argument.
    ExDirSearchCancel,
    /// Cancel the search.
    ExDirSearchNext,
    /// Find the next matching file.
    ExDirSearchPrev,
    /// Find the previous matching file.

    //!Search And Replace Commands
    ExIncrementalSearch,
    /// Incremental search
    ExFind,
    /// Find
    ExFindReplace,
    /// Find and replace
    ExFindRepeat,
    /// Repeat last find/replace operation.
    ExFindRepeatOnce,
    /// Repeat last find/replace operation only once.
    ExFindRepeatReverse,
    /// Repeat last find/replace operation in reverse.
    ExMatchBracket,
    /// Find matching bracket ([{<>}]).
    ExHilitWord,
    /// Highlight current word everywhere in the file.
    ExSearchWordPrev,
    /// Search for previous occurence of word under the cursor.
    ExSearchWordNext,
    /// Search for next occurence of the word under the cursor.
    ExHilitMatchBracket,
    /// Highlight matching bracket.
    ExSearch,
    ///
    ExSearchB,
    ///
    ExSearchRx,
    ///
    ExSearchAgain,
    ///
    ExSearchAgainB,
    ///
    ExSearchReplace,
    ///
    ExSearchReplaceB,
    ///
    ExSearchReplaceRx,
    ///

    //!Window Commands
    ExWinHSplit,
    /// Split the window horizontally.
    ExWinNext,
    /// Switch to the next (bottom) window.
    ExWinPrev,
    /// Switcn to the previous (top) window.
    ExWinClose,
    /// Close the current window.
    ExWinZoom,
    /// Delete all windows except the current one.
    ExWinResize,
    /// Resize current window (+n,-n given as argument).
    ExViewBuffers,
    /// View all open buffers.
    ExListRoutines,
    /// Display routines in the current source file.
    ExExitEditor,
    /// Exit FTE.
    ExShowEntryScreen,
    /// View external program output if available.
    ExMessage,
    /// Display a message in the status bar area
    ExGetChoice,
    /// Display a choice dialog and push selection index onto the parameter stack

    //!Compiler Commands
    ExAskCompiler,
    /// Ask for compile command and run compiler.
    ///
    /// See also: RunCompiler
    ExRunCompiler,
    /// Run configured compile command.
    ///
    /// See also: [[Compile]]
    ExViewMessages,
    /// View the compiler output.
    ExCompileNextError,
    /// Switch to the next compiler error.
    ExCompilePrevError,
    /// Switch to the previous compiler error.
    ExRunProgram,
    /// Run an external program.

    //!Cvs Commands
    ExCvs,
    /// Ask for CVS options and run CVS.
    ExRunCvs,
    /// Run configured CVS command.
    ExViewCvs,
    /// View CVS output.
    ExClearCvsMessages,
    /// Clear CVS messages.
    ExCvsDiff,
    /// Ask for CVS diff options and run CVS.
    ///
    /// See also: RunCvsDiff
    ExRunCvsDiff,
    /// Run configured CVS diff command.
    ///
    /// See also: CvsDiff
    ExViewCvsDiff,
    /// View CVS diff output.
    ExCvsCommit,
    /// Ask for CVS commit options and run CVS.
    ///
    /// See also: RunCvsCommit
    ExRunCvsCommit,
    /// Run configured CVS commit command.
    ///
    /// See also: CvsCommit
    ExViewCvsLog,
    /// View CVS log.

    //!Svn Commands
    ExSvn,
    /// Ask for SVN options and run SVN.
    ExRunSvn,
    /// Run configured SVN command.
    ExViewSvn,
    /// View SVN output.
    ExClearSvnMessages,
    /// Clear SVN messages.
    ExSvnDiff,
    /// Ask for SVN diff options and run SVN.
    ///
    /// See also: RunSvnDiff
    ExRunSvnDiff,
    /// Run configured SVN diff command.
    ///
    /// See also: SvnDiff
    ExViewSvnDiff,
    /// View SVN diff output.
    ExSvnCommit,
    /// Ask for SVN commit options and run SVN.
    ExRunSvnCommit,
    /// Run configured SVN commit command.
    ExViewSvnLog,
    /// View SVN log.

    //!Tag Commands
    ExTagFind,
    /// Find word argumen in tag files.
    ExTagFindWord,
    /// Find word under cursor in tag files.
    ExTagNext,
    /// Switch to next occurance of tag.
    ExTagPrev,
    /// Switch to previous occurance of tag.
    ExTagPop,
    /// Pop saved position from tag stack.
    ExTagLoad,
    /// Load tag file and merge with current tags.
    ExTagClear,
    /// Clear loaded tags.
    ExTagGoto,
    ///

    //!Option Commands
    ExToggleAutoIndent,
    ///
    ExToggleInsert,
    ///
    ExToggleExpandTabs,
    ///
    ExToggleShowTabs,
    ///
    ExToggleUndo,
    ///
    ExToggleReadOnly,
    ///
    ExToggleKeepBackups,
    ///
    ExToggleMatchCase,
    ///
    ExToggleBackSpKillTab,
    ///
    ExToggleDeleteKillTab,
    ///
    ExToggleSpaceTabs,
    ///
    ExToggleIndentWithTabs,
    ///
    ExToggleBackSpUnindents,
    ///
    ExToggleWordWrap,
    ///
    ExToggleTrim,
    ///
    ExToggleShowMarkers,
    ///
    ExToggleHilitTags,
    ///
    ExToggleShowBookmarks,
    ///
    ExToggleMakeBackups,
    ///
    ExSetLeftMargin,
    ///
    ExSetRightMargin,
    ///
    ExToggleSysClipboard,
    ///
    ExSetPrintDevice,
    ///
    ExChangeTabSize,
    ///
    ExChangeLeftMargin,
    ///
    ExChangeRightMargin,
    ///


    //!Other Commands
    ExShowPosition,
    /// Show internal position information on status line.
    ExShowVersion,
    /// Show eFTE version information.
    ExShowKey,
    /// Wait for a keypress and display modifiers+key pressed.
    ExWinRefresh,
    /// Refresh the display.
    ExMainMenu,
    /// Activate the main menu.
    ExShowMenu,
    /// Popup the menu specified as the argument.
    ExLocalMenu,
    /// Popup the context menu.
    ///
    /// See variable: LocalMenu
    ExChangeMode,
    /// Change active mode for the current buffer.
    ExChangeKeys,
    /// Change the keybindings for current buffer.
    ExChangeFlags,
    /// Change the option flags for current buffer.
    ExCancel,
    ///
    ExActivate,
    ///
    ExRescan,
    ///
    ExCloseActivate,
    ///
    ExActivateInOtherWindow,
    ///
    ExDeleteFile,
    /// Remove a file while in the directory browser
    ExRenameFile,
    /// Rename a file while in the directory browser
    ExMakeDirectory,
    /// Make a directory while in the directory browser
    ExASCIITable,
    /// Display ASCII selector in status line.
    ExDesktopSave,
    /// Save the desktop.
    ExClipClear,
    /// Clear the clipboard.
    ExDesktopSaveAs,
    /// Save the desktop under a new name.
    ExDesktopLoad,
    /// Load the desktop from a file.
    ExChildClose,
    ///
    ExBufListFileSave,
    /// Save the currently selected file in the buffer list.
    ExBufListFileClose,
    /// Close the currently selected file in the buffer list.
    ExBufListSearchCancel,
    /// Cancel the search.
    ExBufListSearchNext,
    /// Goto the next match in the search.
    ExBufListSearchPrev,
    /// Goto the previous match in the search.
    ExViewModeMap,
    /// View the current mode's keybindings.
    ExClearMessages,
    /// Clear the compiler messages.
    ExIndentFunction,
    /// Indent current the function (if SmartIndent is available for the current mode).
    ExMoveFunctionPrev,
    /// Move the cursor to the previous function.
    ExMoveFunctionNext,
    /// Move the cursor to the next function.
    ExInsertDate,
    /// Insert date at the cursor.
    ExInsertUid,
    /// Insert user name at the cursor.
    ExFrameNew,
    /// Create a new frame (only supported on certain platforms).
    ExFrameClose,
    /// Close the current frame (only supported on certain platforms).
    ExFrameNext,
    /// Goto the next frame (only supported on certain platforms).
    ExFramePrev,
    /// Goto the previous frame (only supported on certain platforms).
    ExBufferViewNext,
    /// Goto the next buffer.
    ExBufferViewPrev,
    /// Goto the previous buffer.
    ExShowHelpWord,
    /// Show context help on keyword.
    ExShowHelp,
    /// Show help for eFTE.
    ExSetCIndentStyle,
    /// Set C indentation style parameters.
    ///
    /// Has the following parameters:
    ///
    /// * [[C_Indent]] = 4;
    /// * [[C_BraceOfs]] = 0;
    /// * [[C_ParenDelta]] = -1;
    /// * [[C_CaseOfs]] = 0;
    /// * [[C_CaseDelta]] = 4;
    /// * [[C_ClassOfs]] = 0;
    /// * [[C_ClassDelta]] = 4;
    /// * [[C_ColonOfs]] = -4;
    /// * [[C_CommentOfs]] = 0;
    /// * [[C_CommentDelta]] = 1;
    /// * [[C_FirstLevelWidth]] = -1;
    /// * [[C_FirstLevelIndent]] = 4;
    /// * [[C_Continuation]] = 4;
    ExSetIndentWithTabs,
    /// Set the value of the indent-with-tabs to the argument.
    ExRunProgramAsync,
    ///
    ExListMark,
    /// Mark a single line in the list.
    ExListUnmark,
    /// Unmark the selected line line in the list.
    ExListToggleMark,
    /// Toggle the marking of the selected line in the list.
    ExListMarkAll,
    /// Mark all the lines in the list.
    ExListUnmarkAll,
    /// Unmark all the lines in the list.
    ExListToggleMarkAll,
    /// Toggle the marking of all lines in the list.
    ExBlockPasteOver,
    /// Delete the content's of selection and paste the clipboard contents to the
    /// current position
    ExPrint,
    /// Print a string to the console
    ExExecuteCommand,
    /// Prompt user for a command to execute
    ExExecute,
    /// Execute the command on tos
    ExTick,
    /// Look up the command name on tos$

    //!Info To Stack Commands
    ExPushFileName,
    /// Full filename including path of current file
    ExPushCurDir,
    /// Current working directory
    ExQuestionAt,
    /// Where is the cursor?
    ExPushCurChar,
    /// Current character
    ExPushCurWord,
    /// Current word
    ExPushCurLine,
    /// Current line
    ExPushSelection,
    /// Selected text
    ExPushEfteVersion,
    /// eFTE version number
    ExAsc,
    /// Move a character from paramstack to string stack 65 -> "A"
    ExChar
    /// Move a character from string stack to paramstack  "A" -> 65

    //*** END
} ExCommands;

#endif
