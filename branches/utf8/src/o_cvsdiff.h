/*
 * o_cvsdiff.h
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 * Contributed by Martin Frydl <frydl@matfyz.cz>
 *
 * Class showing output from CVS diff command. Allows copying of lines
 * to clipboard and allows to jump to lines in real sources.
 */

#ifndef CVSDIFF_H_
#define CVSDIFF_H_

class ECvsDiff: public ECvsBase {
public:
    int CurrLine, ToLine, InToFile;
    char *CurrFile;

    ECvsDiff(int createFlags, EModel **ARoot, char *Dir, char *ACommand, char *AOnFiles);
    ~ECvsDiff();

    void ParseFromTo(char *line, int len);
    virtual void ParseLine(char *line, int len);
    // Returns 0 if OK
    virtual int RunPipe(char *Dir, char *Command, char *Info);

    virtual int ExecCommand(int Command, ExState &State);
    int BlockCopy(int Append);

    virtual int GetContext() {
        return CONTEXT_CVSDIFF;
    }
    virtual EEventMap *GetEventMap();
};

extern ECvsDiff *CvsDiffView;

#endif
