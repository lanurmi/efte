/*
 * o_svndiff.h
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 * S.Pinigin copy o_cvsdiff.h and replace cvs/Cvs/CVS to svn/Svn/SVN.
 *
 * Class showing output from SVN diff command. Allows copying of lines
 * to clipboard and allows to jump to lines in real sources.
 */

#ifndef SVNDIFF_H_
#define SVNDIFF_H_

class ESvnDiff: public ESvnBase {
public:
    int CurrLine, ToLine, InToFile;
    char *CurrFile;

    ESvnDiff(int createFlags, EModel **ARoot, const char *Dir, const char *ACommand, char *AOnFiles);
    ~ESvnDiff();

    void ParseFromTo(const char *line, int len);
    virtual void ParseLine(const char *line, int len);
    // Returns 0 if OK
    virtual int RunPipe(const char *Dir, const char *Command, const char *OnFiles);

    virtual int ExecCommand(int Command, ExState &State);
    int BlockCopy(int Append);

    virtual int GetContext() const;
    virtual EEventMap *GetEventMap();
};

extern ESvnDiff *SvnDiffView;

#endif
