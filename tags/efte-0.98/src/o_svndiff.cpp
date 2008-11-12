/*
 * o_svndiff.cpp
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 * S.Pinigin copy o_cvsdiff.cpp and replace cvs/Cvs/CVS to svn/Svn/SVN.
 *
 * Class showing output from SVN diff command. Allows copying of lines
 * to clipboard and allows to jump to lines in real sources.
 */

#include "fte.h"

ESvnDiff       *SvnDiffView = 0;

ESvnDiff::ESvnDiff(int createFlags, EModel ** ARoot, char *ADir, char *ACommand,
                   char *AOnFiles):
        ESvnBase(createFlags, ARoot, "SVN diff") {
    SvnDiffView = this;
    CurrFile = 0;
    CurrLine = 0;
    ToLine = 0;
    InToFile = 0;
    RunPipe(ADir, ACommand, AOnFiles);
}

ESvnDiff::~ESvnDiff() {
    SvnDiffView = 0;
    free(CurrFile);
}

void
ESvnDiff::ParseFromTo(char *line, int /*len */) {
    char           *start;
    char           *end;

    // "@@ -1,20 +1,20 @@"
    //            ^ ^^

    start = strchr(line, '+');

    CurrLine = strtol(start, &end, 10) - 1;
    if (*end == ',')
        ToLine = CurrLine + atoi(end + 1);
    else
        ToLine = CurrLine + 1;
    if (!(CurrLine < ToLine && ToLine > 0))
        CurrLine = ToLine = 0;
}


//AddLine(0, -1, line);                 - output only, default color
//AddLine(CurrFile, -1, line);          - output and tofile, default color
//AddLine(CurrFile, CurrLine, line);    - output and tofile in line, default color
//AddLine(CurrFile, CurrLine, line, 1); - output and tofile in line, color 1
void
ESvnDiff::ParseLine(char *line, int len) {
    if (len > 8 && strncmp(line, "+++ ", 4) == 0) {
        //"+++ test.txt\t...."
        free(CurrFile);
        CurrFile = strdup(line + 4);
        strtok(CurrFile, " \t");
        CurrLine = ToLine = InToFile = 0;
        AddLine(CurrFile, -1, line);
    } else {
        if (len > 8 && strncmp(line, "@@ ", 3) == 0) {
            // To file or to hunk
            if (strcmp(line + len - 3, " @@") == 0) {
                // "@@ -1,20 +1,20 @@"
                // To hunk
                if (CurrFile) {
                    ParseFromTo(line, len);
                    AddLine(CurrFile, CurrLine, line, 2);
                } else {
                    AddLine(0, -1, line);
                }
            } else {
                AddLine(0, -1, line);
            }
        } else {
            if (CurrLine < ToLine) {
                // Diff line (markable, if CurrFile is set, also hilited)
                if (*line == '+') {
                    //"+(new line from file)"
                    AddLine(CurrFile, CurrLine, line, 1);
                    CurrLine++;
                } else {
                    if (*line == '-') {
                        //"-(deleted line from file)"
                        AddLine(0, -1, line);
                    } else {
                        //" (line without change)"
                        AddLine(CurrFile, CurrLine, line);
                        CurrLine++;
                    }
                }
            } else {
                AddLine(0, -1, line);
            }
        }
    }
}

int
ESvnDiff::RunPipe(char *ADir, char *ACommand, char *AOnFiles) {
    FreeLines();
    free(CurrFile);
    CurrLine = ToLine = InToFile = 0;
    CurrFile = 0;
    return ESvnBase::RunPipe(ADir, ACommand, AOnFiles);
}

int
ESvnDiff::ExecCommand(int Command, ExState & State) {
    switch (Command) {
    case ExBlockCopy:
        return BlockCopy(0);
    case ExBlockCopyAppend:
        return BlockCopy(1);
    }
    return EList::ExecCommand(Command, State);
}

int
ESvnDiff::BlockCopy(int Append) {
    if (SSBuffer == 0)
        return ErFAIL;
    if (Append) {
        if (SystemClipboard)
            GetPMClip(0);
    } else
        SSBuffer->Clear();
    SSBuffer->BlockMode = bmLine;
    // How to set these two ?
    BFI(SSBuffer, BFI_TabSize) = 8;
    BFI(SSBuffer, BFI_ExpandTabs) = 0;
    BFI(SSBuffer, BFI_Undo) = 0;
    // Go through list of marked lines
    int             last = -1,
                           tl = 0;
    for (int i = 0; i < LineCount; i++) {
        if (Lines[i]->Status & 2) {
            // Marked
            if (last != i - 1 && tl) {
                // Gap between this and last marked line
                SSBuffer->InsLine(tl++, 0);
            }
            SSBuffer->InsertLine(tl++, strlen(Lines[i]->Msg + 2),
                                 Lines[i]->Msg + 2);
            last = i;
        }
    }
    if (SystemClipboard)
        PutPMClip(0);
    return ErOK;
}

// Event map - this name is used in config files when defining eventmap
EEventMap      *
ESvnDiff::GetEventMap() {
    return FindEventMap("SVNDIFF");
}
