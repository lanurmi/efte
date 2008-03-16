/*    o_buffer.cpp
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

SearchReplaceOptions LSearch = { 0 };
int suspendLoads = 0;

EViewPort *EBuffer::CreateViewPort(EView *V) {
    V->Port = new EEditPort(this, V);
    AddView(V);

    if (Loaded == 0 && !suspendLoads) {
        Load();

        if (CompilerMsgs)
            CompilerMsgs->FindFileErrors(this);
        if (CvsDiffView)
            CvsDiffView->FindFileLines(this);
        if (SvnDiffView)
            SvnDiffView->FindFileLines(this);

        markIndex.retrieveForBuffer(this);

        int r, c;

        if (RetrieveFPos(FileName, r, c) == 1)
            SetNearPosR(c, r);
        //printf("setting to c:%d r:%d f:%s", c, r, FileName);
        V->Port->GetPos();
        V->Port->ReCenter = 1;

        if (BFI(this, BFI_SaveBookmarks) == 3)
            RetrieveBookmarks(this);
    }
    return V->Port;
}

EEditPort::EEditPort(EBuffer *B, EView *V): EViewPort(V) {
    Buffer = B;
    Rows = Cols = 0;
    OldTP.Row = -1;
    OldTP.Col = -1;

    GetPos();
    TP = B->TP;
    CP = B->CP;
    if (V && V->MView && V->MView->Win) {
        V->MView->ConQuerySize(&Cols, &Rows);
        Rows--;
    }
}

EEditPort::~EEditPort() {
    StorePos();
}

void EEditPort::Resize(int Width, int Height) {
    Cols = Width;
    Rows = Height - 1;
    RedrawAll();
}

int EEditPort::SetTop(int Col, int Line) {
    int A, B;

    if (Line >= Buffer->VCount) Line = Buffer->VCount - 1;
    if (Line < 0) Line = 0;

    A = Line;
    B = Line + Rows;

    TP.Row = Line;
    TP.Col = Col;

    if (A >= Buffer->VCount) A = Buffer->VCount - 1;
    if (B >= Buffer->VCount) {
        B = Buffer->VCount - 1;
    }
    Buffer->Draw(Buffer->VToR(A), -1);
    return 1;
}

void EEditPort::StorePos() {
    Buffer->CP = CP;
    Buffer->TP = TP;
}

void EEditPort::GetPos() {
    CP = Buffer->CP;
    TP = Buffer->TP;
}

void EEditPort::ScrollY(int Delta) {
    // optimization
    // no need to scroll (clear) entire window which we are about to redraw
    if (Delta >= Rows || -Delta >= Rows)
        return ;

    if (Delta < 0) {
        Delta = -Delta;
        if (Delta > Rows) return;
        View->MView->ConScroll(csDown, 0, 0, Cols, Rows, hcPlain_Background, Delta);
    } else {
        if (Delta > Rows) return;
        View->MView->ConScroll(csUp, 0, 0, Cols, Rows, hcPlain_Background, Delta);
    }
}

void EEditPort::DrawLine(int L, TDrawBuffer B) {
    if (L < TP.Row) return;
    if (L >= TP.Row + Rows) return;
    if (View->MView->Win->GetViewContext() == View->MView)
        View->MView->ConPutBox(0, L - TP.Row, Cols, 1, B);
    //    printf("%d %d (%d %d %d %d)\n", 0, L - TP.Row, view->sX, view->sY, view->sW, view->sH);
}

void EEditPort::RedrawAll() {
    Buffer->Draw(TP.Row, -1);
    ///    Redraw(0, 0, Cols, Rows);
}

int EBuffer::GetContext() {
    return CONTEXT_FILE;
}

void EEditPort::HandleEvent(TEvent &Event) {
    EViewPort::HandleEvent(Event);
    switch (Event.What) {
    case evKeyDown: {
        char Ch;
        if (GetCharFromEvent(Event, &Ch)) {
            if (Buffer->BeginMacro() == 0)
                return ;
            Buffer->TypeChar(Ch);
            Event.What = evNone;
        }
    }
    break;
    case evCommand:
        switch (Event.Msg.Command) {
        case cmVScrollUp:
            Buffer->ScrollDown(Event.Msg.Param1);
            Event.What = evNone;
            break;
        case cmVScrollDown:
            Buffer->ScrollUp(Event.Msg.Param1);
            Event.What = evNone;
            break;
        case cmVScrollPgUp:
            Buffer->ScrollDown(Rows);
            Event.What = evNone;
            break;
        case cmVScrollPgDn:
            Buffer->ScrollUp(Rows);
            Event.What = evNone;
            break;
        case cmVScrollMove: {
            int ypos;

//                fprintf(stderr, "Pos = %d\n\x7", Event.Msg.Param1);
            ypos = Buffer->CP.Row - TP.Row;
            Buffer->SetNearPos(Buffer->CP.Col, Event.Msg.Param1 + ypos);
            SetTop(TP.Col, Event.Msg.Param1);
            RedrawAll();
        }
        Event.What = evNone;
        break;
        case cmHScrollLeft:
            Buffer->ScrollRight(Event.Msg.Param1);
            Event.What = evNone;
            break;
        case cmHScrollRight:
            Buffer->ScrollLeft(Event.Msg.Param1);
            Event.What = evNone;
            break;
        case cmHScrollPgLt:
            Buffer->ScrollRight(Cols);
            Event.What = evNone;
            break;
        case cmHScrollPgRt:
            Buffer->ScrollLeft(Cols);
            Event.What = evNone;
            break;
        case cmHScrollMove: {
            int xpos;

            xpos = Buffer->CP.Col - TP.Col;
            Buffer->SetNearPos(Event.Msg.Param1 + xpos, Buffer->CP.Row);
            SetTop(Event.Msg.Param1, TP.Row);
            RedrawAll();
        }
        Event.What = evNone;
        break;
        }
        break;
    case evMouseDown:
    case evMouseMove:
    case evMouseAuto:
    case evMouseUp:
        HandleMouse(Event);
        break;
    }
}

void EEditPort::HandleMouse(TEvent &Event) {
    int x, y, xx, yy, W, H;

    View->MView->ConQuerySize(&W, &H);

    x = Event.Mouse.X;
    y = Event.Mouse.Y;

    if (Event.What != evMouseDown || y < H - 1) {
        xx = x + TP.Col;
        yy = y + TP.Row;
        if (yy >= Buffer->VCount) yy = Buffer->VCount - 1;
        if (yy < 0) yy = 0;
        if (xx < 0) xx = 0;

        switch (Event.What) {
        case evMouseDown:
            if (Event.Mouse.Y == H - 1)
                break;
            if (View->MView->Win->CaptureMouse(1))
                View->MView->MouseCaptured = 1;
            else
                break;

            View->MView->MouseMoved = 0;

            if (Event.Mouse.Buttons == 1) {
                // left mouse button down
                Buffer->SetNearPos(xx, yy);
                switch (Event.Mouse.Count % 5) {
                case 1:
                    break;
                case 2:
                    Buffer->BlockSelectWord();
                    break;
                case 3:
                    Buffer->BlockSelectLine();
                    break;
                case 4:
                    Buffer->BlockSelectPara();
                    break;
                }
                //            Window->Buffer->Redraw();
                if (SystemClipboard) {
                    // note: copy to second clipboard
                    Buffer->NextCommand();
                    Buffer->BlockCopy(0, 1);
                }
                Event.What = evNone;
            } else if (Event.Mouse.Buttons == 2) {
                // right mouse button down
                Buffer->SetNearPos(xx, yy);
            }
            break;
        case evMouseAuto:
        case evMouseMove:
            if (View->MView->MouseCaptured) {
                if (Event.Mouse.Buttons == 1) {
                    // left mouse button move
                    if (!View->MView->MouseMoved) {
                        if (Event.Mouse.KeyMask == kfCtrl) Buffer->BlockMarkColumn();
                        else if (Event.Mouse.KeyMask == kfAlt) Buffer->BlockMarkLine();
                        else Buffer->BlockMarkStream();
                        Buffer->BlockUnmark();
                        if (Event.What == evMouseMove)
                            View->MView->MouseMoved = 1;
                    }
                    Buffer->BlockExtendBegin();
                    Buffer->SetNearPos(xx, yy);
                    Buffer->BlockExtendEnd();
                } else if (Event.Mouse.Buttons == 2) {
                    // right mouse button move
                    if (Event.Mouse.KeyMask == kfAlt) {
                    } else {
                        Buffer->SetNearPos(xx, yy);
                    }
                }

                Event.What = evNone;
            }
            break;
            /*        case evMouseAuto:
                        if (View->MView->MouseCaptured) {
                            Event.What = evNone;
                        }
                        break;*/
        case evMouseUp:
            if (View->MView->MouseCaptured)
                View->MView->Win->CaptureMouse(0);
            else
                break;
            View->MView->MouseCaptured = 0;
            if (Event.Mouse.Buttons == 1) {
                // left mouse button up
                if (View->MView->MouseMoved)
                    if (SystemClipboard) {
                        // note: copy to second clipboard
                        Buffer->NextCommand();
                        Buffer->BlockCopy(0, 1);
                    }
            }
            if (Event.Mouse.Buttons == 2) {
                // right mouse button up
                if (!View->MView->MouseMoved) {
                    EEventMap *Map = View->MView->Win->GetEventMap();
                    const char *MName = 0;

                    if (Map)
                        MName = Map->GetMenu(EM_LocalMenu);
                    if (MName == 0)
                        MName = "Local";
                    View->MView->Win->Parent->PopupMenu(MName);
                }
            }
            if (Event.Mouse.Buttons == 4) {
                // middle mouse button up
                if (SystemClipboard) {
                    // note: copy to second clipboard
                    Buffer->NextCommand();
                    if (Event.Mouse.KeyMask == 0)
                        Buffer->BlockPasteStream(1);
                    else if (Event.Mouse.KeyMask == kfCtrl)
                        Buffer->BlockPasteColumn(1);
                    else if (Event.Mouse.KeyMask == kfAlt)
                        Buffer->BlockPasteLine(1);
                }
            }
            Event.What = evNone;
            break;
        }
    }
}

void EEditPort::UpdateView() {
    Buffer->Redraw();
}

void EEditPort::RepaintView() {
    RedrawAll();
}

void EEditPort::UpdateStatus() {
}

void EEditPort::RepaintStatus() {
    //Buffer->Redraw();
}

EEventMap *EBuffer::GetEventMap() {
    return FindActiveMap(Mode);
}

/**
 * MACRO: Display condition code in status bar?
 */
int EBuffer::ToggleConditionDisplay() {
    DisplayCondition = !DisplayCondition;
    return 1;
}

int EBuffer::BeginMacro() {
    return NextCommand();
}

int EBuffer::ExecMacro(const char *name) {
    int mNum = MacroNum(name);
    if (mNum == 0) return 1;
    return ((EGUI *)gui)->ExecMacro(View->MView->Win, mNum);
}

int EBuffer::ExecCommand(int Command, ExState &State) {
    if (CursorWithinEOL == 1 && (Command != ExMoveUp && Command != ExMoveDown)) {
        LastUpDownColumn = -1; // Reset when not moving up or down
    }

    switch (Command) {
        // ---------- basic cursor movements ----------
    case ExCursorLeft:
        return CursorLeft();
    case ExCursorRight:
        return CursorRight();
    case ExCursorUp:
        return CursorUp();
    case ExCursorDown:
        return CursorDown();
        // --------------------------------------------
    case ExMoveUp:
        return MoveUp();
    case ExMoveDown:
        return MoveDown();
    case ExMoveLeft:
        return MoveLeft();
    case ExMoveRight:
        return MoveRight();
    case ExMovePrev:
        return MovePrev();
    case ExMoveNext:
        return MoveNext();
    case ExMoveWordLeft:
        return MoveWordLeft();
    case ExMoveWordRight:
        return MoveWordRight();
    case ExMoveWordPrev:
        return MoveWordPrev();
    case ExMoveWordNext:
        return MoveWordNext();
    case ExMoveWordEndLeft:
        return MoveWordEndLeft();
    case ExMoveWordEndRight:
        return MoveWordEndRight();
    case ExMoveWordEndPrev:
        return MoveWordEndPrev();
    case ExMoveWordEndNext:
        return MoveWordEndNext();
    case ExMoveWordOrCapLeft:
        return MoveWordOrCapLeft();
    case ExMoveWordOrCapRight:
        return MoveWordOrCapRight();
    case ExMoveWordOrCapPrev:
        return MoveWordOrCapPrev();
    case ExMoveWordOrCapNext:
        return MoveWordOrCapNext();
    case ExMoveWordOrCapEndLeft:
        return MoveWordOrCapEndLeft();
    case ExMoveWordOrCapEndRight:
        return MoveWordOrCapEndRight();
    case ExMoveWordOrCapEndPrev:
        return MoveWordOrCapEndPrev();
    case ExMoveWordOrCapEndNext:
        return MoveWordOrCapEndNext();
    case ExMoveLineStart:
        return MoveLineStart();
    case ExMoveLineEnd:
        return MoveLineEnd();
    case ExMovePageStart:
        return MovePageStart();
    case ExMovePageEnd:
        return MovePageEnd();
    case ExMovePageUp:
        return MovePageUp();
    case ExMovePageDown:
        return MovePageDown();
    case ExMovePageLeft:
        return MovePageLeft();
    case ExMovePageRight:
        return MovePageEnd();
    case ExMoveFileStart:
        return MoveFileStart();
    case ExMoveFileEnd:
        return MoveFileEnd();
    case ExMoveBlockStart:
        return MoveBlockStart();
    case ExMoveBlockEnd:
        return MoveBlockEnd();
    case ExMoveFirstNonWhite:
        return MoveFirstNonWhite();
    case ExMoveLastNonWhite:
        return MoveLastNonWhite();
    case ExMovePrevEqualIndent:
        return MovePrevEqualIndent();
    case ExMoveNextEqualIndent:
        return MoveNextEqualIndent();
    case ExMovePrevTab:
        return MovePrevTab();
    case ExMoveNextTab:
        return MoveNextTab();
    case ExMoveTabStart:
        return MoveTabStart();
    case ExMoveTabEnd:
        return MoveTabEnd();
    case ExMoveLineTop:
        return MoveLineTop();
    case ExMoveLineCenter:
        return MoveLineCenter();
    case ExMoveLineBottom:
        return MoveLineBottom();
    case ExMoveBeginOrNonWhite:
        return MoveBeginOrNonWhite();
    case ExMoveBeginLinePageFile:
        return MoveBeginLinePageFile();
    case ExMoveEndLinePageFile:
        return MoveEndLinePageFile();
    case ExScrollLeft:
        return ScrollLeft(State);
    case ExScrollRight:
        return ScrollRight(State);
    case ExScrollDown:
        return ScrollDown(State);
    case ExScrollUp:
        return ScrollUp(State);
    case ExKillLine:
        return KillLine();
    case ExKillChar:
        return KillChar();
    case ExKillCharPrev:
        return KillCharPrev();
    case ExKillWord:
        return KillWord();
    case ExKillWordPrev:
        return KillWordPrev();
    case ExKillWordOrCap:
        return KillWordOrCap();
    case ExKillWordOrCapPrev:
        return KillWordOrCapPrev();
    case ExKillToLineStart:
        return KillToLineStart();
    case ExKillToLineEnd:
        return KillToLineEnd();
    case ExKillBlock:
        return KillBlock();
    case ExBackSpace:
        return BackSpace();
    case ExDelete:
        return Delete();
    case ExCharCaseUp:
        return CharCaseUp();
    case ExCharCaseDown:
        return CharCaseDown();
    case ExCharCaseToggle:
        return CharCaseToggle();
    case ExLineCaseUp:
        return LineCaseUp();
    case ExLineCaseDown:
        return LineCaseDown();
    case ExLineCaseToggle:
        return LineCaseToggle();
    case ExLineInsert:
        return LineInsert();
    case ExLineAdd:
        return LineAdd();
    case ExLineSplit:
        return LineSplit();
    case ExLineJoin:
        return LineJoin();
    case ExLineNew:
        return LineNew();
    case ExLineIndent:
        return LineIndent();
    case ExLineTrim:
        return LineTrim();
    case ExLineCenter:
        return LineCenter();
    case ExInsertSpacesToTab: {
        return InsertSpacesToTab(ParamStack.pop());
    }
    case ExInsertTab:
        return InsertTab();
    case ExInsertSpace:
        return InsertSpace();
    case ExWrapPara:
        return WrapPara();
    case ExInsPrevLineChar:
        return InsPrevLineChar();
    case ExInsPrevLineToEol:
        return InsPrevLineToEol();
    case ExLineDuplicate:
        return LineDuplicate();
    case ExSelectionStr:
        return BlockGet();
    case ExBlockBegin:
        return BlockBegin();
    case ExBlockEnd:
        return BlockEnd();
    case ExBlockUnmark:
        return BlockUnmark();
    case ExBlockCut:
        return BlockCut(0);
    case ExBlockCopy:
        return BlockCopy(0);
    case ExBlockCutAppend:
        return BlockCut(1);
    case ExBlockCopyAppend:
        return BlockCopy(1);
    case ExClipClear:
        return ClipClear();
    case ExBlockPaste:
        return BlockPaste();
    case ExBlockKill:
        return BlockKill();
    case ExBlockIndent: {
        int saved_persistence, ret_code;

        saved_persistence = BFI(this, BFI_PersistentBlocks);
        BFI_SET(this, BFI_PersistentBlocks, 1);
        ret_code = BlockIndent();
        BFI_SET(this, BFI_PersistentBlocks, saved_persistence);
        return ret_code;
    }
    case ExBlockUnindent: {
        int saved_persistence, ret_code;

        saved_persistence = BFI(this, BFI_PersistentBlocks);
        BFI_SET(this, BFI_PersistentBlocks, 1);
        ret_code = BlockUnindent();
        BFI_SET(this, BFI_PersistentBlocks, saved_persistence);
        return ret_code;
    }
    case ExBlockClear:
        return BlockClear();
    case ExBlockMarkStream:
        return BlockMarkStream();
    case ExBlockMarkLine:
        return BlockMarkLine();
    case ExBlockMarkColumn:
        return BlockMarkColumn();
    case ExBlockCaseUp:
        return BlockCaseUp();
    case ExBlockCaseDown:
        return BlockCaseDown();
    case ExBlockCaseToggle:
        return BlockCaseToggle();
    case ExBlockExtendBegin:
        return BlockExtendBegin();
    case ExBlockExtendEnd:
        return BlockExtendEnd();
    case ExBlockReIndent:
        return BlockReIndent();
    case ExBlockSelectWord:
        return BlockSelectWord();
    case ExBlockSelectLine:
        return BlockSelectLine();
    case ExBlockSelectPara:
        return BlockSelectPara();
    case ExBlockUnTab:
        return BlockUnTab();
    case ExBlockEnTab:
        return BlockEnTab();
    case ExUndo:
        return Undo();
    case ExRedo:
        return Redo();
    case ExMatchBracket:
        return MatchBracket();
    case ExMovePrevPos:
        return MovePrevPos();
    case ExMoveSavedPosCol:
        return MoveSavedPosCol();
    case ExMoveSavedPosRow:
        return MoveSavedPosRow();
    case ExMoveSavedPos:
        return MoveSavedPos();
    case ExSavePos:
        return SavePos();
    case ExCompleteWord:
        return CompleteWord();
    case ExBlockPasteStream:
        return BlockPasteStream();
    case ExBlockPasteLine:
        return BlockPasteLine();
    case ExBlockPasteColumn:
        return BlockPasteColumn();
    case ExBlockPasteOver:
        return BlockPasteOver();
    case ExShowPosition:
        return ShowPosition();
    case ExFoldCreate:
        return FoldCreate(VToR(CP.Row));
    case ExFoldDestroy:
        return FoldDestroy(VToR(CP.Row));
    case ExFoldDestroyAll:
        return FoldDestroyAll();
    case ExFoldPromote:
        return FoldPromote(VToR(CP.Row));
    case ExFoldDemote:
        return FoldDemote(VToR(CP.Row));
    case ExFoldOpen:
        return FoldOpen(VToR(CP.Row));
    case ExFoldOpenNested:
        return FoldOpenNested();
    case ExFoldClose:
        return FoldClose(VToR(CP.Row));
    case ExFoldOpenAll:
        return FoldOpenAll();
    case ExFoldCloseAll:
        return FoldCloseAll();
    case ExFoldToggleOpenClose:
        return FoldToggleOpenClose();
    case ExFoldCreateAtRoutines:
        return FoldCreateAtRoutines();
    case ExMoveFoldTop:
        return MoveFoldTop();
    case ExMoveFoldPrev:
        return MoveFoldPrev();
    case ExMoveFoldNext:
        return MoveFoldNext();
    case ExFileSave:
        return Save();
    case ExFilePrint:
        return FilePrint();
    case ExBlockPrint:
        return BlockPrint();
    case ExBlockTrim:
        return BlockTrim();
    case ExFileTrim:
        return FileTrim();
    case ExHilitWord:
        return HilitWord();
    case ExSearchWordPrev:
        return SearchWord(SEARCH_BACK | SEARCH_NEXT);
    case ExSearchWordNext:
        return SearchWord(SEARCH_NEXT);
    case ExHilitMatchBracket:
        return HilitMatchBracket();
    case ExToggleAutoIndent:
        return ToggleAutoIndent();
    case ExToggleInsert:
        return ToggleInsert();
    case ExToggleExpandTabs:
        return ToggleExpandTabs();
    case ExToggleShowTabs:
        return ToggleShowTabs();
    case ExToggleUndo:
        return ToggleUndo();
    case ExToggleReadOnly:
        return ToggleReadOnly();
    case ExToggleKeepBackups:
        return ToggleKeepBackups();
    case ExToggleMatchCase:
        return ToggleMatchCase();
    case ExToggleBackSpKillTab:
        return ToggleBackSpKillTab();
    case ExToggleDeleteKillTab:
        return ToggleDeleteKillTab();
    case ExToggleSpaceTabs:
        return ToggleSpaceTabs();
    case ExToggleIndentWithTabs:
        return ToggleIndentWithTabs();
    case ExToggleBackSpUnindents:
        return ToggleBackSpUnindents();
    case ExToggleWordWrap:
        return ToggleWordWrap();
//    case ExToggleTrim:
//        return ToggleTrim();
    case ExToggleShowMarkers:
        return ToggleShowMarkers();
    case ExToggleHilitTags:
        return ToggleHilitTags();
    case ExToggleShowBookmarks:
        return ToggleShowBookmarks();
    case ExToggleMakeBackups:
        return ToggleMakeBackups();
    case ExSetLeftMargin:
        return SetLeftMargin();
    case ExSetRightMargin:
        return SetRightMargin();
    case ExSetIndentWithTabs:
        return SetIndentWithTabs(State);
    case ExMoveToLine:
        return MoveToLine(State);
    case ExMoveToColumn:
        return MoveToColumn(State);
    case ExFoldCreateByRegexp:
        return FoldCreateByRegexp(State);
    case ExPlaceBookmark:
        return PlaceBookmark(State);
    case ExRemoveBookmark:
        return RemoveBookmark(State);
    case ExGotoBookmark:
        return GotoBookmark(State);
    case ExPlaceGlobalBookmark:
        return PlaceGlobalBookmark(State);
    case ExPushGlobalBookmark:
        return PushGlobalBookmark();
    case ExInsertString:
        return InsertString(State);
    case ExSelfInsert:
        return SelfInsert(State);
    case ExFileReload:
        return FileReload(State);
    case ExFileSaveAs:
        return FileSaveAs(State);
    case ExFileWriteTo:
        return FileWriteTo(State);
    case ExBlockRead:
        return BlockRead(State);
    case ExBlockReadStream:
        return BlockReadStream(State);
    case ExBlockReadLine:
        return BlockReadLine(State);
    case ExBlockReadColumn:
        return BlockReadColumn(State);
    case ExBlockWrite:
        return BlockWrite(State);
    case ExBlockSort:
        return BlockSort(0);
    case ExBlockSortReverse:
        return BlockSort(1);
    case ExFind:
        return Find(State);
    case ExFindReplace:
        return FindReplace(State);
    case ExFindRepeat:
        return FindRepeat(State);
    case ExFindRepeatOnce:
        return FindRepeatOnce(State);
    case ExFindRepeatReverse:
        return FindRepeatReverse(State);
    case ExSearch:
        return Search(State);
    case ExSearchB:
        return SearchB(State);
    case ExSearchRx:
        return SearchRx(State);
    case ExSearchAgain:
        return SearchAgain(State);
    case ExSearchAgainB:
        return SearchAgainB(State);
    case ExSearchReplace:
        return SearchReplace(State);
    case ExSearchReplaceB:
        return SearchReplaceB(State);
    case ExSearchReplaceRx:
        return SearchReplaceRx(State);
    case ExInsertChar:
        return InsertChar(State);
    case ExTypeChar:
        return TypeChar(State);
    case ExGetChar:
        return GetChar(State);
    case ExChangeMode:
        return ChangeMode(State);
        //case ExChangeKeys:          return ChangeKeys(State);
    case ExChangeFlags:
        return ChangeFlags(State);
    case ExChangeTabSize:
        return ChangeTabSize(State);
    case ExChangeLeftMargin:
        return ChangeLeftMargin(State);
    case ExChangeRightMargin:
        return ChangeRightMargin(State);
    case ExASCIITable:
        return ASCIITable(State);
    case ExCharTrans:
        return CharTrans(State);
    case ExLineTrans:
        return LineTrans(State);
    case ExBlockTrans:
        return BlockTrans(State);
    case ExTagFind:
        return FindTag(State);
    case ExTagFindWord:
        return FindTagWord(State);
    case ExSetCIndentStyle:
        return SetCIndentStyle(State);
    case ExBlockMarkFunction:
        return BlockMarkFunction();
    case ExIndentFunction:
        return IndentFunction();
    case ExMoveFunctionPrev:
        return MoveFunctionPrev();
    case ExMoveFunctionNext:
        return MoveFunctionNext();
    case ExInsertDate:
        return InsertDate(State);
    case ExInsertUid:
        return InsertUid();
    case ExShowHelpWord:
        return ShowHelpWord(State);
    case ExMessage:
        return Message(State);
    case ExGetChoice:
        return GetChoice(State);
    case ExToggleConditionDisplay:
        return ToggleConditionDisplay();

        // Stack based pushes

    case ExPushFileName:
        return PushFileName();
    case ExQuestionAt:
        return QuestionAt();
    case ExPushCurChar:
        return PushCurChar();
    case ExPushCurWord:
        return PushCurWord();
    case ExPushCurLine:
        return PushCurLine();
    case ExPushSelection:
        return PushSelection();
    case ExPushEfteVersion:
        return PushEfteVerNo();
    }
    return EModel::ExecCommand(Command, State);
}

void EBuffer::HandleEvent(TEvent &Event) {
    EModel::HandleEvent(Event);
}

int EBuffer::MoveToLine(ExState &State) {
    int No = ParamStack.pop();

    if (No == -1) {
        char Num[10];

        sprintf(Num, "%d", VToR(CP.Row) + 1);
        if (View->MView->Win->GetStr("Goto Line", sizeof(Num), Num, HIST_POSITION) == 0) {
            SetBranchCondition(0);
            return 0;
        }

        No = atol(Num);
    }

    return SetNearPosR(CP.Col, No - 1);
}

int EBuffer::MoveToColumn(ExState &State) {
    int No = ParamStack.pop();

    if (No == -1) {
        char Num[10];

        sprintf(Num, "%d", CP.Col + 1);
        if (View->MView->Win->GetStr("Goto Column", 8, Num, HIST_POSITION) == 0) {
            SetBranchCondition(0);
            return 0;
        }

        No = atol(Num);
    }

    return SetNearPos(No - 1, CP.Row);
}

int EBuffer::FoldCreateByRegexp(ExState &State) {
    if (sstack.size() == 0) {
        Msg(S_ERROR, "String stack underflow in FoldCreateByRegexp");
        SetBranchCondition(0);
        return 0;
    }

    std::string reg = sstack.back(); sstack.pop_back();
    if (reg.empty()) {
        char strbuf[1024] = "";
        if (View->MView->Win->GetStr("Create Fold Regexp", sizeof(strbuf), strbuf, HIST_REGEXP) == 0) {
            SetBranchCondition(0);
            return 0;
        }
        reg = strbuf;
    }
    return FoldCreateByRegexp(reg.c_str());
}

int EBuffer::PlaceUserBookmark(const char *n, EPoint P) {
    char name[256+4] = "_BMK";
    int result;
    EPoint prev;

    strcpy(name + 4, n);
    if (GetBookmark(name, prev) == 0) {
        prev.Row = -1;
        prev.Col = -1;
    }
    result = PlaceBookmark(name, P);
    if (result) {
        if (BFI(this, BFI_ShowBookmarks)) {
            FullRedraw();
        }
        if (BFI(this, BFI_SaveBookmarks) == 1 || BFI(this, BFI_SaveBookmarks) == 2) {
            if (!Modify()) return result;    // Never try to save to read-only
            if (BFI(this, BFI_Undo)) {
                if (PushULong(prev.Row) == 0) return 0;
                if (PushULong(prev.Col) == 0) return 0;
                if (PushUData((void *)n, strlen(n) + 1) == 0) return 0;
                if (PushULong(strlen(n) + 1) == 0) return 0;
                if (PushUChar(ucPlaceUserBookmark) == 0) return 0;
            }
        }
    }
    return result;
}

int EBuffer::RemoveUserBookmark(const char *n) {
    char name[256+4] = "_BMK";
    int result;
    EPoint p;

    strcpy(name + 4, n);
    GetBookmark(name, p);       // p is valid only if remove is successful
    result = RemoveBookmark(name);
    if (result) {
        if (BFI(this, BFI_ShowBookmarks)) {
            FullRedraw();
        }
        if (BFI(this, BFI_SaveBookmarks) == 1 || BFI(this, BFI_SaveBookmarks) == 2) {
            if (!Modify()) return result;    // Never try to save to read-only
            if (PushULong(p.Row) == 0) return 0;
            if (PushULong(p.Col) == 0) return 0;
            if (PushUData((void *)n, strlen(n) + 1) == 0) return 0;
            if (PushULong(strlen(n) + 1) == 0) return 0;
            if (PushUChar(ucRemoveUserBookmark) == 0) return 0;
        }
    }
    return result;
}

int EBuffer::GotoUserBookmark(const char *n) {
    char name[256+4] = "_BMK";

    strcpy(name + 4, n);
    return GotoBookmark(name);
}

int EBuffer::GetUserBookmarkForLine(int searchFrom, int searchForLine, char *&Name, EPoint &P) {
    int i;

    i = searchFrom;
    while (1) {
        i = GetBookmarkForLine(i, searchForLine, Name, P);
        if (i == -1) return -1;
        if (strncmp(Name, "_BMK", 4) == 0) {
            Name += 4;
            return i;
        }
    }
}

int EBuffer::PlaceBookmark(ExState &State) {
    if (sstack.size() == 0) {
        Msg(S_ERROR, "String stack underflow in PlaceBookmark");
        SetBranchCondition(0);
        return 0;
    }

    EPoint P = CP;
    P.Row = VToR(P.Row);
    std::string bm = sstack.back(); sstack.pop_back();

    if (bm.empty()) {
        char name[256] = "";
        if (View->MView->Win->GetStr("Place Bookmark", sizeof(name), name, HIST_BOOKMARK) == 0) {
            SetBranchCondition(0);
            return 0;
        }
        bm = name;
    }

    return PlaceUserBookmark(bm.c_str(), P);
}

int EBuffer::RemoveBookmark(ExState &State) {
    if (sstack.size() == 0) {
        Msg(S_ERROR, "String stack underflow in RemoveBookmark");
        SetBranchCondition(0);
        return 0;
    }

    std::string bm = sstack.back(); sstack.pop_back();

    if (bm.empty()) {
        char name[256] = "";
        if (View->MView->Win->GetStr("Remove Bookmark", sizeof(name), name, HIST_BOOKMARK) == 0) {
            SetBranchCondition(0);
            return 0;
        }
        bm = name;
    }

    return RemoveUserBookmark(bm.c_str());
}

int EBuffer::GotoBookmark(ExState &State) {
    if (sstack.size() == 0) {
        Msg(S_ERROR, "String stack underflow in RemoveBookmark");
        SetBranchCondition(0);
        return 0;
    }

    std::string bm = sstack.back(); sstack.pop_back();

    if (bm.empty()) {
        char name[256] = "";
        if (View->MView->Win->GetStr("Goto Bookmark", sizeof(name), name, HIST_BOOKMARK) == 0) {
            SetBranchCondition(0);
            return 0;
        }
        bm = name;
    }

    return GotoUserBookmark(bm.c_str());
}

int EBuffer::PlaceGlobalBookmark(ExState &State) {
    if (sstack.size() == 0) {
        Msg(S_ERROR, "String stack underflow in RemoveBookmark");
        SetBranchCondition(0);
        return 0;
    }

    EPoint P = CP;
    P.Row = VToR(P.Row);
    std::string bm = sstack.back(); sstack.pop_back();

    if (bm.empty()) {
        char name[256] = "";
        if (View->MView->Win->GetStr("Place Global Bookmark", sizeof(name), name, HIST_BOOKMARK) == 0) {
            SetBranchCondition(0);
            return 0;
        }
        bm = name;
    }

    if (markIndex.insert(bm.c_str(), this, P) == 0) {
        Msg(S_ERROR, "Error placing global bookmark %s.", bm.c_str());
    }
    return 1;
}

int EBuffer::PushGlobalBookmark() {
    EPoint P = CP;

    P.Row = VToR(P.Row);
    EMark *m = markIndex.pushMark(this, P);
    if (m)
        Msg(S_INFO, "Placed bookmark %s", m->getName());
    return m ? 1 : 0;
}

/**
 * MACRO: Get a character from the user and push it onto the stack parameter
 */
int EBuffer::GetChar(ExState &State) {
// proably want to update buffer display herem when waiting for a key
// some things aren't done now, like, updating visual cursor position,
// when outputting to buffer in a loop, like:
// { begin GetChar InsertString $TosChar again }
// i *think* when waiting for a key, display should reflect the
// current=actual state.

    char Ch;
    int No;

    TEvent E;
    E.What = evKeyDown;
    E.Key.Code = View->MView->Win->GetChar("Character:");
    if (!GetCharFromEvent(E, &Ch)) {
        SetBranchCondition(0);
        return 0;
    }
    No = Ch;

    if (No < 0 || No > 255) {
        SetBranchCondition(0);
        return 0;
    }

    Ch = char(No);
    ParamStack.push((unsigned int) Ch);

    SetBranchCondition(1);
    return 1;
}

/**
 * MACRO: Display a message on the status bar
 */
int EBuffer::Message(ExState &State) {
    if (sstack.size() == 0) {
        Msg(S_ERROR, "String stack underflow in Message");
        SetBranchCondition(0);
        return 0;
    }

    Msg(S_INFO, sstack.back().c_str());
    sstack.pop_back();

    SetBranchCondition(1);
    return 1;
}

/**
 * MACRO: Display a choice to the user and push the result onto the stack
 */
int EBuffer::GetChoice(ExState &State) {
    const char *title, *msg, *c1=NULL, *c2=NULL, *c3=NULL, *c4=NULL, *c5=NULL, *c6=NULL;
    unsigned int count = (unsigned int) ParamStack.pop();

    fprintf(stderr, "Count: %i, Stack Size: %i\n", count, (int) sstack.size());

    // Count of choices + 2 (title and message)
    if (count + 2 > sstack.size()) {
        Msg(S_ERROR, "String stack underflow in GetChoice");
        SetBranchCondition(0);
        return 0;
    }

    if (count >= 6) { c6 = sstack.back().c_str(); sstack.pop_back(); }
    if (count >= 5) { c5 = sstack.back().c_str(); sstack.pop_back(); }
    if (count >= 4) { c4 = sstack.back().c_str(); sstack.pop_back(); }
    if (count >= 3) { c3 = sstack.back().c_str(); sstack.pop_back(); }
    if (count >= 2) { c2 = sstack.back().c_str(); sstack.pop_back(); }

    c1    = sstack.back().c_str(); sstack.pop_back();
    msg   = sstack.back().c_str(); sstack.pop_back();
    title = sstack.back().c_str(); sstack.pop_back();

    int result = -1;
    switch (count) {
    case 1:
        result = View->MView->Win->Choice(GPC_ERROR, title, count, c1, msg );
        break;
    case 2:
        result = View->MView->Win->Choice(GPC_ERROR, title, count, c1, c2, msg );
        break;
    case 3:
        result = View->MView->Win->Choice(GPC_ERROR, title, count, c1, c2, c3, msg );
        break;
    case 4:
        result = View->MView->Win->Choice(GPC_ERROR, title, count, c1, c2, c3, c4, msg );
        break;
    case 5:
        result = View->MView->Win->Choice(GPC_ERROR, title, count, c1, c2, c3, c4, c5, msg );
        break;
    case 6:
        result = View->MView->Win->Choice(GPC_ERROR, title, count, c1, c2, c3, c4, c5, c6, msg );
        break;
    }

    if (result == -1) {
        SetBranchCondition(0);
        return 0;
    }

    ParamStack.push(result);

    SetBranchCondition(1);
    return 1;
}

int EBuffer::InsertChar(ExState &State) {
    int No = ParamStack.pop();
    char Ch;

    if (No == -1) {
        TEvent E;
        E.What = evKeyDown;
        E.Key.Code = View->MView->Win->GetChar("Quote Char:");
        if (!GetCharFromEvent(E, &Ch)) {
            SetBranchCondition(0);
            return 0;
        }
        No = Ch;
    }
    if (No < 0 || No > 255) {
        SetBranchCondition(0);
        return 0;
    }
    Ch = char(No);
    return InsertChar(Ch);
}

int EBuffer::TypeChar(ExState &State) {
    int No = ParamStack.pop();
    char Ch;

    if (No == -1) {
        TEvent E;
        E.What = evKeyDown;
        E.Key.Code = View->MView->Win->GetChar(0);
        if (!GetCharFromEvent(E, &Ch)) return 0;
        No = Ch;
    }
    if (No < 0 || No > 255) return 0;
    Ch = char(No);
    return TypeChar(Ch);
}

int EBuffer::InsertString(ExState &State) {
    if (sstack.size() == 0) {
        View->Msg(S_ERROR, "String stack underflow in InsertString");
        SetBranchCondition(0);
        return 0;
    }

    std::string str = sstack.back(); sstack.pop_back();

    if (str.empty()) {
        char strbuf[1024] = "";
        if (View->MView->Win->GetStr("Insert String", sizeof(strbuf), strbuf, HIST_DEFAULT) == 0) {
            SetBranchCondition(0);
            return 0;
        }

        str = strbuf;
    }

    return InsertString(str.c_str(), str.size());
}

extern int LastEventChar;

int EBuffer::SelfInsert(ExState &/*State*/) {
    if (LastEventChar != -1)
        return TypeChar(char(LastEventChar));
    SetBranchCondition(0);
    return 0;
}

int EBuffer::FileReload(ExState &/*State*/) {
    if (Modified) {
        switch (View->MView->Win->Choice(GPC_ERROR, "File Modified",
                                         2,
                                         "&Reload",
                                         "&Cancel",
                                         "%s", FileName))
        {
        case 0:
            break;
        case 1:
        case -1:
        default:
            SetBranchCondition(0);
            return 0;
        }
    }
//    GetNewNumber();
    return Reload();
}

int EBuffer::FileSaveAs(const char *FName) {
    char Name[MAXPATH];

    if (ExpandPath(FName, Name, sizeof(Name)) == -1) {
        View->MView->Win->Choice(GPC_ERROR, "Error", 1, "O&K", "Invalid path: %s.", FName);
        SetBranchCondition(0);
        return 0;
    }
    if (FindFile(Name) == 0) {
        if (FileExists(Name)) {
            switch (View->MView->Win->Choice(GPC_ERROR, "File Exists",
                                             2,
                                             "&Overwrite",
                                             "&Cancel",
                                             "%s", Name))
            {
            case 0:
                break;
            case 1:
            case -1:
            default:
                SetBranchCondition(0);
                return 0;

            }
        }
        free(FileName);
        FileName = strdup(Name);
        UpdateTitle();
        return Save();
    } else {
        View->MView->Win->Choice(GPC_ERROR, "Error", 1, "O&K", "Already editing '%s.'", Name);
        SetBranchCondition(0);
        return 0;
    }
}

int EBuffer::FileSaveAs(ExState &State) {
    if (sstack.size() == 0) {
        Msg(S_ERROR, "String stack underflow in FileSaveAs");
        SetBranchCondition(0);
        return 0;
    }

    std::string fname = sstack.back(); sstack.pop_back();
    if (fname.empty()) {
        char FName[MAXPATH] = "";
        if (View->MView->Win->GetFile("Save As", sizeof(FName), FName, HIST_PATH, GF_SAVEAS) == 0) {
            SetBranchCondition(0);
            return 0;
        }
        fname = FName;
    }

    return FileSaveAs(fname.c_str());
}

int EBuffer::FileWriteTo(const char *FName) {
    char Name[MAXPATH];

    if (ExpandPath(FName, Name, sizeof(Name)) == -1) {
        View->MView->Win->Choice(GPC_ERROR, "Error", 1, "O&K", "Invalid path: %s.", FName);
        SetBranchCondition(0);
        return 0;
    }
    if (FindFile(Name) == 0) {
        if (FileExists(Name)) {
            switch (View->MView->Win->Choice(GPC_ERROR, "File Exists",
                                             2,
                                             "&Overwrite",
                                             "&Cancel",
                                             "%s", Name))
            {
            case 0:
                break;
            case 1:
            case -1:
            default:
                SetBranchCondition(0);
                return 0;
            }
        }
        return SaveTo(Name);
    } else {
        View->MView->Win->Choice(GPC_ERROR, "Error", 1, "O&K", "Already editing '%s.'", Name);
        SetBranchCondition(0);
        return 0;
    }
}

int EBuffer::FileWriteTo(ExState &State) {
    if (sstack.size() == 0) {
        Msg(S_ERROR, "String stack underflow in FileSaveAs");
        SetBranchCondition(0);
        return 0;
    }

    std::string fname = sstack.back(); sstack.pop_back();
    if (fname.empty()) {
        char FName[MAXPATH] = "";
        if (View->MView->Win->GetFile("Write To", sizeof(FName), FName, HIST_PATH, GF_SAVEAS) == 0) {
            SetBranchCondition(0);
            return 0;
        }
        fname = FName;
    }

    return FileWriteTo(fname.c_str());
}

int EBuffer::BlockReadX(ExState &State, int blockMode) {
    if (sstack.size() == 0) {
        View->Msg(S_ERROR, "String stack underflow in BlockReadX");
        SetBranchCondition(0);
        return 0;
    }

    char Name[MAXPATH];
    std::string fname = sstack.back(); sstack.pop_back();

    if (fname.empty()) {
        char FName[MAXPATH];

        if (JustDirectory(FileName, FName, sizeof(FName)) == -1) return 0;
        SlashDir(FName);
        if (View->MView->Win->GetFile("Read block", sizeof(FName), FName, HIST_PATH, GF_OPEN) == 0) {
            SetBranchCondition(0);
            return 0;
        }
        fname = FName;
    }

    strcpy(Name, fname.c_str());

    if (ExpandPath(fname.c_str(), Name, sizeof(Name)) == -1) {
        View->MView->Win->Choice(GPC_ERROR, "Error", 1, "O&K", "Invalid path: %s.", fname.c_str());
        return 0;
    }
    return BlockReadFrom(Name, blockMode);
}

int EBuffer::BlockRead(ExState &State) {
    return BlockReadX(State, BlockMode);
}

int EBuffer::BlockReadStream(ExState &State) {
    return BlockReadX(State, bmStream);
}

int EBuffer::BlockReadLine(ExState &State) {
    return BlockReadX(State, bmLine);
}

int EBuffer::BlockReadColumn(ExState &State) {
    return BlockReadX(State, bmColumn);
}

int EBuffer::BlockWrite(ExState &State) {
    if (sstack.size() == 0) {
        Msg(S_ERROR, "String stack underflow in BlockWrite");
        SetBranchCondition(0);
        return 0;
    }

    char Name[MAXPATH];
    int Append = 0;

    std::string fname = sstack.back(); sstack.pop_back();

    if (fname.empty()) {
        char FName[MAXPATH];

        if (JustDirectory(FileName, FName, sizeof(FName)) == -1) {
            SetBranchCondition(0);
            return 0;
        }
        SlashDir(FName);
        if (View->MView->Win->GetFile("Write block", sizeof(FName), FName, HIST_PATH, GF_SAVEAS) == 0) {
            SetBranchCondition(0);
            return 0;
        }

        fname = FName;
    }

    if (ExpandPath(fname.c_str(), Name, sizeof(Name)) == -1) {
        View->MView->Win->Choice(GPC_ERROR, "Error", 1, "O&K", "Invalid path: %s.", fname.c_str());
        SetBranchCondition(0);
        return 0;
    }

    if (FindFile(Name) == 0) {
        if (FileExists(Name)) {
            switch (View->MView->Win->Choice(GPC_ERROR, "File Exists",
                                             3,
                                             "&Overwrite",
                                             "&Append",
                                             "&Cancel",
                                             "%s", Name))
            {
            case 0:
                break;
            case 1:
                Append = 1;
                break;
            case 2:
            case -1:
            default:
                SetBranchCondition(0);
                return 0;

            }
        }
    } else {
        View->MView->Win->Choice(GPC_ERROR, "Error", 1, "O&K", "Already editing '%s.'", Name);
        SetBranchCondition(0);
        return 0;
    }
    return BlockWriteTo(Name, Append);
}

int EBuffer::Find(ExState &State) {
    if (sstack.size() < 2) {
        Msg(S_ERROR, "String stack underflow in Find (%i)", sstack.size());
        SetBranchCondition(0);
        return 0;
    }
    char find[MAXSEARCH+1] = "";
    char options[32] = "";

    strcpy(options, sstack.back().c_str()); sstack.pop_back();
    strcpy(find, sstack.back().c_str()); sstack.pop_back();

    if (strlen(find) > 0) {
        if (strlen(options) == 0)
            strcpy(options, BFS(this, BFS_DefFindOpt));

        LSearch.ok = 0;
        strcpy(LSearch.strSearch, find);
        LSearch.strReplace[0] = 0;
        LSearch.Options = 0;
        if (ParseSearchOptions(0, options, LSearch.Options) == 0) {
            SetBranchCondition(0);
            return 0;
        }
        LSearch.ok = 1;
    } else if ((HaveGUIDialogs & GUIDLG_FIND) && GUIDialogs) {
        LSearch.ok = 0;
        LSearch.strSearch[0] = 0;
        LSearch.strReplace[0] = 0;
        LSearch.Options = 0;
        if (strlen(options) == 0 && BFS(this, BFS_DefFindOpt))
            (options, BFS(this, BFS_DefFindOpt));
        if (ParseSearchOptions(0, options, LSearch.Options) == 0)
            LSearch.Options = 0;

        if (DLGGetFind(View->MView->Win, LSearch) == 0) {
            SetBranchCondition(0);
            return 0;
        }
    } else {
        if (strlen(options) == 0 && BFS(this, BFS_DefFindOpt))
            strcpy(options, BFS(this, BFS_DefFindOpt));
        if (View->MView->Win->GetStr("Find", sizeof(find), find, HIST_SEARCH) == 0) {
            SetBranchCondition(0);
            return 0;
        }
        if (View->MView->Win->GetStr("Options (All/Block/Cur/Delln/Glob/Igncase/Joinln/Rev/Word/regX)",
                                     sizeof(options), options, HIST_SEARCHOPT) == 0)
        {
            SetBranchCondition(0);
            return 0;
        }

        LSearch.ok = 0;
        strcpy(LSearch.strSearch, find);
        LSearch.strReplace[0] = 0;
        LSearch.Options = 0;
        if (ParseSearchOptions(0, options, LSearch.Options) == 0) {
            SetBranchCondition(0);
            return 0;
        }
        LSearch.ok = 1;
    }
    if (LSearch.ok == 0) {
        SetBranchCondition(0);
        return 0;
    }
    LSearch.Options |= SEARCH_CENTER;
    if (Find(LSearch) == 0) {
        SetBranchCondition(0);
        return 0;
    }
    SetBranchCondition(1);
    return 1;
}

// TODO
int EBuffer::FindReplace(ExState &State) {
    if (sstack.size() < 3) {
        Msg(S_ERROR, "String stack underflow in FindReplace (%i)", sstack.size());
        SetBranchCondition(0);
        return 0;
    }

    char find[MAXSEARCH+1] = "";
    char replace[MAXSEARCH+1] = "";
    char options[32] = "";

    strcpy(options, sstack.back().c_str()); sstack.pop_back();
    strcpy(replace, sstack.back().c_str()); sstack.pop_back();
    strcpy(find,    sstack.back().c_str()); sstack.pop_back();

    if (strlen(find) > 0 && strlen(replace) > 0 && strlen(options) > 0) {
        LSearch.ok = 0;
        strcpy(LSearch.strSearch, find);
        strcpy(LSearch.strReplace, replace);
        LSearch.Options = 0;
        if (ParseSearchOptions(1, options, LSearch.Options) == 0) {
            SetBranchCondition(0);
            return 0;
        }
        LSearch.Options |= SEARCH_REPLACE;
        LSearch.ok = 1;
    } else if ((HaveGUIDialogs & GUIDLG_FINDREPLACE) && GUIDialogs) {
        LSearch.ok = 0;
        LSearch.strSearch[0] = 0;
        LSearch.strReplace[0] = 0;
        LSearch.Options = 0;
        if (strlen(options) == 0 && BFS(this, BFS_DefFindReplaceOpt))
            strcpy(options, BFS(this, BFS_DefFindReplaceOpt));
        if (ParseSearchOptions(1, options, LSearch.Options) == 0)
            LSearch.Options = 0;
        if (DLGGetFindReplace(View->MView->Win, LSearch) == 0)
            return 0;
    } else {
        if (strlen(find) == 0) {
            if (View->MView->Win->GetStr("Find", sizeof(find), find, HIST_SEARCH) == 0) {
                SetBranchCondition(0);
                return 0;
            }
        }

        if (strlen(replace) == 0) {
            if (View->MView->Win->GetStr("Replace", sizeof(replace), replace, HIST_SEARCH) == 0) {
                SetBranchCondition(0);
                return 0;
            }
        }

        if (strlen(options) == 0) {
            if (BFS(this, BFS_DefFindReplaceOpt))
                strcpy(options, BFS(this, BFS_DefFindReplaceOpt));
            if (View->MView->Win->GetStr("Options (All/Block/Cur/Delln/Glob/Igncase/Joinln/Rev/Splitln/Noask/Word/regX)",
                                         sizeof(options), options, HIST_SEARCHOPT) == 0)
            {
                SetBranchCondition(0);
                return 0;
            }
        }

        LSearch.ok = 0;
        strcpy(LSearch.strSearch, find);
        strcpy(LSearch.strReplace, replace);
        LSearch.Options = 0;
        if (ParseSearchOptions(1, options, LSearch.Options) == 0) return 0;
        LSearch.Options |= SEARCH_REPLACE;
        LSearch.ok = 1;
    }
    if (LSearch.ok == 0) return 0;
    LSearch.Options |= SEARCH_CENTER;
    if (Find(LSearch) == 0) return 0;
    return 1;
}






/* TODO sorry i don't understand the Find string stack effect.
   sometimes it returns items, sometimes it doesn't and i can't really
   see a clear line of when it does and when it doesn't.
   now these functions sometimes get their empty strings which they
   need to pass to find to avoid underflow back, it seems to have
   to do something with finding or not finding the string. but at least
   they don't abort or refuse to run because of stack underflows anymore/
*/



int EBuffer::FindWithModifier(ExState &State, int PlusFlags, int MinFlags, int ToggleFlags) {
    int rc = 1;
    int prevstack = sstack.size();
    sstack.push_back("");
    sstack.push_back("");

    if (LSearch.ok == 0)  {
        rc = Find(State);
    } else {
        LSearch.Options |= PlusFlags;
        LSearch.Options &= ~MinFlags;
        LSearch.Options ^= ToggleFlags;
        rc = Find(LSearch);
        LSearch.Options ^= ToggleFlags;
    }
    // fix stack

    int nowstack = sstack.size();
//    nowstack = prevstack: ok
//    nowstack < prevstac: bad but can't help it.
//    nowstack > prevstack: bad too, but can drop
    while (nowstack-- > prevstack)
        sstack.pop_back();

    return rc;
}


int EBuffer::FindRepeat(ExState &State) {
    return FindWithModifier(State, SEARCH_NEXT, SEARCH_GLOBAL, 0);
}

int EBuffer::FindRepeatReverse(ExState &State) {
    return FindWithModifier(State, SEARCH_NEXT, SEARCH_GLOBAL, SEARCH_BACK);
}

int EBuffer::FindRepeatOnce(ExState &State) {
    return FindWithModifier(State, SEARCH_NEXT, SEARCH_GLOBAL|SEARCH_ALL, 0);
}










int EBuffer::ChangeMode(ExState &State) {
    if (sstack.size() == 0) {
        Msg(S_ERROR, "String stack underflow in ChangeMode");
        SetBranchCondition(0);
        return 0;
    }

    std::string mode = sstack.back(); sstack.pop_back();

    int rc;
    if (mode.empty()) {
        char Mode[32] = "";
        if (View->MView->Win->GetStr("Mode", sizeof(Mode), Mode, HIST_SETUP) == 0) {
            SetBranchCondition(0);
            return 0;
        }

        mode = Mode;
    }

    rc = ChangeMode(mode.c_str());
    FullRedraw();

    return rc;
}

int EBuffer::ChangeKeys(ExState &State) {
    if (sstack.size() == 0) {
        Msg(S_ERROR, "String stack underflow in ChangeKeys");
        SetBranchCondition(0);
        return 0;
    }

    std::string keys = sstack.back(); sstack.pop_back();

    int rc;
    if (keys.empty()) {
        char Keys[32] = "";
        if (View->MView->Win->GetStr("Keys", sizeof(Keys), Keys, HIST_SETUP) == 0) {
            SetBranchCondition(0);
            return 0;
        }

        keys = Keys;
    }

    rc = ChangeKeys(keys.c_str());
    FullRedraw();

    return rc;
}

int EBuffer::ChangeFlags(ExState &State) {
    if (sstack.size() == 0) {
        Msg(S_ERROR, "String stack underflow in ChangeFlags");
        SetBranchCondition(0);
        return 0;
    }

    std::string flags = sstack.back(); sstack.pop_back();

    int rc;
    if (flags.empty()) {
        char Flags[32] = "";
        if (View->MView->Win->GetStr("Flags", sizeof(Mode), Flags, HIST_SETUP) == 0) {
            SetBranchCondition(0);
            return 0;
        }

        flags = Flags;
    }

    rc = ChangeFlags(flags.c_str());
    FullRedraw();

    return rc;
}

int EBuffer::ChangeTabSize(ExState &State) {
    int No = ParamStack.pop();

    if (No == -1) {
        char Num[10];

        sprintf(Num, "%d", BFI(this, BFI_TabSize));
        if (View->MView->Win->GetStr("TabSize", sizeof(Num), Num, HIST_SETUP) == 0) return 0;
        No = atol(Num);
    }
    if (No < 1) return 0;
    if (No > 32) return 0;
    BFI(this, BFI_TabSize) = No;
    FullRedraw();
    return 1;
}

int EBuffer::SetIndentWithTabs(ExState &State) {
    int No = ParamStack.pop();
    Flags.num[BFI_IndentWithTabs] = No ? 1 : 0;
    return 1;
}

int EBuffer::ChangeRightMargin(ExState &State) {
    int No = ParamStack.pop();

    if (No == -1) {
        char Num[10];
        sprintf(Num, "%d", BFI(this, BFI_RightMargin) + 1);
        if (View->MView->Win->GetStr("RightMargin", sizeof(Num), Num, HIST_SETUP) == 0) return 0;
        No = atol(Num) - 1;
    }
    if (No <= 1) return 0;
    BFI(this, BFI_RightMargin) = No;
    Msg(S_INFO, "RightMargin set to %d.", No + 1);
    return 1;
}

int EBuffer::ChangeLeftMargin(ExState &State) {
    int No = ParamStack.pop();;

    if (No == -1) {
        char Num[10];
        sprintf(Num, "%d", BFI(this, BFI_LeftMargin) + 1);
        if (View->MView->Win->GetStr("LeftMargin", sizeof(Num), Num, HIST_SETUP) == 0) return 0;
        No = atol(Num) - 1;
    }
    if (No < 0) return 0;
    BFI(this, BFI_LeftMargin) = No;
    Msg(S_INFO, "LeftMargin set to %d.", No + 1);
    return 1;
}


int EBuffer::CanQuit() {
    if (Modified)
        return 0;
    else
        return 1;
}

int EBuffer::ConfQuit(GxView *V, int multiFile) {
    if (Modified) {
        if (multiFile) {
            switch (V->Choice(GPC_ERROR,
                              "File Modified",
                              5,
                              "&Save",
                              "&As",
                              "A&ll",
                              "&Discard",
                              "&Cancel",
                              "%s", FileName)) {
            case 0: /* Save */
                if (Save() == 0) return 0;
                break;
            case 1: { /* As */
                char FName[MAXPATH];
                strcpy(FName, FileName);
                if (V->GetFile("Save As", sizeof(FName), FName, HIST_PATH, GF_SAVEAS) == 0) return 0;
                if (FileSaveAs(FName) == 0) return 0;
            }
            break;
            case 2: /* Save all */
                return -2;
            case 3: /* Discard */
                break;
            case 4: /* Cancel */
            case -1:
            default:
                return 0;
            }
        } else {
            switch (V->Choice(GPC_ERROR,
                              "File Modified",
                              4,
                              "&Save",
                              "&As",
                              "&Discard",
                              "&Cancel",
                              "%s", FileName)) {
            case 0: /* Save */
                if (Save() == 0) return 0;
                break;
            case 1: { /* As */
                char FName[MAXPATH];
                strcpy(FName, FileName);
                if (V->GetFile("Save As", sizeof(FName), FName, HIST_PATH, GF_SAVEAS) == 0) return 0;
                if (FileSaveAs(FName) == 0) return 0;
            }
            break;
            case 2: /* Discard */
                break;
            case 3: /* Cancel */
            case -1:
            default:
                return 0;
            }
        }
    }
    return 1;
}

void EBuffer::GetName(char *AName, int MaxLen) {
    strncpy(AName, FileName, MaxLen);
    AName[MaxLen - 1] = 0;
}

void EBuffer::GetPath(char *APath, int MaxLen) {
    JustDirectory(FileName, APath, MaxLen);
}

void EBuffer::GetInfo(char *AInfo, int /*MaxLen*/) {
    char buf[256] = {0};
    char winTitle[256] = {0};

    JustFileName(FileName, buf, sizeof(buf));
    if (buf[0] == '\0') // if there is no filename, try the directory name.
        JustLastDirectory(FileName, buf, sizeof(buf));

    if (buf[0] != 0) { // if there is a file/dir name, stick it in here.
        strncat(winTitle, buf, sizeof(winTitle) - 1 - strlen(winTitle));
        strncat(winTitle, " - ", sizeof(winTitle) - 1 - strlen(winTitle));
    }
    strncat(winTitle, FileName, sizeof(winTitle) - 1 - strlen(winTitle));
    winTitle[sizeof(winTitle) - 1] = 0;

    sprintf(AInfo,
            "%2d %04d:%03d%c%-150s ",
            ModelNo,
            1 + CP.Row, 1 + CP.Col,
            Modified ? '*' : ' ',
            winTitle);
}

void EBuffer::GetTitle(char *ATitle, int MaxLen, char *ASTitle, int SMaxLen) {
    char *p;

    strncpy(ATitle, FileName, MaxLen - 1);
    ATitle[MaxLen - 1] = 0;
    p = SepRChr(FileName);
    if (p) {
        strncpy(ASTitle, p + 1, SMaxLen - 1);
        ASTitle[SMaxLen - 1] = 0;
    } else {
        strncpy(ASTitle, FileName, SMaxLen - 1);
        ASTitle[SMaxLen - 1] = 0;
    }
}

int EBuffer::ASCIITable(ExState &/*State*/) {
    int rc;

    rc = View->MView->Win->PickASCII();
    if (rc != -1)
        return InsertChar(char(rc));

    return 0;
}

int EBuffer::ScrollLeft(ExState &State) {
    return ScrollLeft(ParamStack.pop());
}

int EBuffer::ScrollRight(ExState &State) {
    return ScrollRight(ParamStack.pop());
}

int EBuffer::ScrollDown(ExState &State) {
    return ScrollDown(ParamStack.pop());
}

int EBuffer::ScrollUp(ExState &State) {
    return ScrollUp(ParamStack.pop());
}

int EBuffer::FindTag(ExState &State) {
    if (sstack.size() == 0) {
        Msg(S_ERROR, "String stack underflow in FindTag");
        SetBranchCondition(0);
        return 0;
    }

    std::string tag = sstack.back(); sstack.pop_back();

    if (tag.empty()) {
        char Tag[MAXSEARCH] = "";

        if (View->MView->Win->GetStr("Find tag", sizeof(Tag), Tag, HIST_SEARCH) == 0) {
            SetBranchCondition(0);
            return 0;
        }

        tag = Tag;
    }

    int j = 2;
    while (j--) {
        int i;

        i = TagFind(this, View, tag.c_str());
        if (i > 0) {
            SetBranchCondition(1);
            return 1;
        } else if (j && (i < 0)) {
            /* Try autoload tags */
            if (View->ExecCommand(ExTagLoad, State) == 0)
                break;
        } else {
            Msg(S_INFO, "Tag '%s' not found.", tag.c_str());
            break;
        }
    }
    return 0;

}

// these two will probably be replaced in the future
int EBuffer::InsertDate(ExState &State) {
    if (sstack.size() == 0) {
        Msg(S_ERROR, "String stack underflow in InsertDate");
        SetBranchCondition(0);
        return 0;
    }

    std::string format = sstack.back(); sstack.pop_back();
    time_t t;
    time(&t);
    char buf[128], *p;

    if (format.empty()) {
        //** 012345678901234567890123
        //** Wed Jan 02 02:23:54 1991
        p = ctime(&t);
        sprintf(buf, "%.10s %.4s", p, p + 20);
    } else {
        struct tm *tt = localtime(&t);
        strftime(buf, sizeof(buf), format.c_str(), tt);
        buf[sizeof(buf) - 1] = 0;
    }

    return InsertString(buf, strlen(buf));
}


int EBuffer::InsertUid() {
    const char *p = getenv("USER");
    if (p == 0) p = getenv("NAME");
    if (p == 0) p = getenv("ID");
    // mostly for Windows.  Why they can't just be standard, I don't know...
    if (p == 0) p = getenv("USERNAME");
    if (p == 0) {
        Msg(S_INFO, "User ID not set ($USER).");
        //return 0;
        p = "UNKNOWN USER";
    }
    return InsertString(p, strlen(p));
}

int EBuffer::PushFileName() {
    sstack.push_back(FileName);
    return 1;
}

int EBuffer::PushCurDir() {
    return 0;
}

int EBuffer::QuestionAt() {
    ParamStack.push(CP.Col);
    ParamStack.push(VToR(CP.Row));
    return 1;
}

int EBuffer::PushCurChar() {
    PELine L;
    int P;

    L = RLine(CP.Row);
    P = CharOffset(L, CP.Col);

    if (CP.Col < LineLen()) {
        char tmp[2];
        tmp[0] = L->Chars[P];
        tmp[1] = 0;

        sstack.push_back(tmp);
    } else
        sstack.push_back("");

    return 1;
}

int EBuffer::PushCurWord() {
    PELine L;
    int P, C;
    int wordBegin, wordEnd;

    L = RLine(CP.Row);
    P = CharOffset(L, CP.Col);

    std::string word;

    if (ChClass(L->Chars[P])) {
        C = ChClassK(L->Chars[P]);

        // search start of word
        while ((P > 0) && (C == ChClassK(L->Chars[P-1]))) P--;
        wordBegin = P;

        // search end of word
        while ((P < L->Count) && (C == ChClassK(L->Chars[P]))) P++;
        wordEnd = P;

        // copy word to buffer
        word.append(L->Chars, wordBegin, wordEnd - wordBegin);
        sstack.push_back(word);
    } else {
        sstack.push_back("");
    }
    return 1;
}

int EBuffer::PushCurLine() {
    PELine L = RLine(CP.Row);

    if (L->Count > 0) {
        std::string line;
        line.append(L->Chars, 0, L->Count);

        sstack.push_back(line);
    } else {
        sstack.push_back("");
    }

    return 1;
}

int EBuffer::PushSelection() {
    //int error = 0;
    EPoint B, E;
    int L;
    PELine LL;
    int A, Z;
    int bc = 0, lc = 0;


    AutoExtend = 0;
    if (CheckBlock() == 0) {
        SetBranchCondition(0);
        return 0;
    }

    if (RCount == 0) {
        SetBranchCondition(0);
        return 0;
    }

    std::string buf;
    B = BB;
    E = BE;
    for (L = B.Row; L <= E.Row; L++) {
        A = -1;
        Z = -1;
        LL = RLine(L);
        switch (BlockMode) {
        case bmLine:
            if (L < E.Row) {
                A = 0;
                Z = LL->Count;
            }
            break;
        case bmColumn:
            if (L < E.Row) {
                A = CharOffset(LL, B.Col);
                Z = CharOffset(LL, E.Col);
            }
            break;
        case bmStream:
            if (B.Row == E.Row) {
                A = CharOffset(LL, B.Col);
                Z = CharOffset(LL, E.Col);
            } else if (L == B.Row) {
                A = CharOffset(LL, B.Col);
                Z = LL->Count;
            } else if (L < E.Row) {
                A = 0;
                Z  = LL->Count;
            } else if (L == E.Row) {
                A = 0;
                Z = CharOffset(LL, E.Col);
            }
            break;
        }
        if (A != -1 && Z != -1) {
            if (A < LL->Count) {
                if (Z > LL->Count)
                    Z = LL->Count;
                if (Z > A) {
                    buf.append(LL->Chars, A, Z-A);
                    bc += Z - A;
                }
            }
            if (BFI(this, BFI_AddCR) == 1) {
                buf.append("\n");
                bc++;
            }

            if (BFI(this, BFI_AddLF) == 1) {
                buf.append("\r");
                bc++;
                lc++;
            }
        }
    }

    sstack.push_back(buf);

    SetBranchCondition(1);
    return 1;
}

int EBuffer::PushEfteVerNo() {
    sstack.push_back(VERSION);
    return 1;
}

int EBuffer::ShowHelpWord(ExState &State) {
    //** Code for BlockSelectWord to find the word under the cursor,
    const char *achr = "+-_."; // these are accepted characters
    char    buf[128];
    int     Y = VToR(CP.Row);
    PELine  L = RLine(Y);
    int     P;

    P = CharOffset(L, CP.Col);

    // fix \b for the case of CATBS
    for (int i = 0; i < P; i++) {
        //printf("%d - %d  %d   %c %c\n", i, P, L->Chars[i],
        //L->Chars[i], L->Chars[P]);
        if ((L->Chars[i] == '\b') && (P < (L->Count - 2)))
            P += 2;
    }
    size_t len = 0;
    if (P < L->Count) {
        // To start of word,
        while ((P > 0)
                && ((L->Chars[P - 1] == '\b') || isalnum(L->Chars[P - 1])
                    || (strchr(achr, L->Chars[P - 1]) != NULL)))
            P--; // '_' for underline is hidden in achr
        if ((P < (L->Count - 1)) && (L->Chars[P] == '\b'))
            P++;
        // To end of word,
        while ((len < (sizeof(buf) - 1)) && (P < L->Count)) {
            if (((P + 1) < L->Count) && (L->Chars[P + 1] == '\b'))
                P += 2;
            else if (isalnum(L->Chars[P])
                     || (strchr(achr, L->Chars[P]) != NULL))
                buf[len++] = L->Chars[P++];
            else
                break;
        }
    }
    buf[len] = 0;
    //printf("Word: %s\n", buf);
    //if (buf[0] == 0) {
    //    Msg(INFO, "No valid word under the cursor.");
    //    return 0;
    //}
    return View->SysShowHelp(State, buf[0] ? buf : 0);
}
