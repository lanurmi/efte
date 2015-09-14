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

    ECvsDiff(int createFlags, EModel **ARoot, const char *Dir, const char *ACommand, const char *AOnFiles);
    ~ECvsDiff();

    void ParseFromTo(const char *line, int len);
    void ParseLine(const char *line, int len) override;
    // Returns 0 if OK
    int RunPipe(const char *Dir, const char *Command, const char *OnFiles) override;

    int ExecCommand(int Command, ExState &State) override;
    int BlockCopy(int Append);

    int GetContext() const override {
        return CONTEXT_CVSDIFF;
    }
    EEventMap *GetEventMap() override;
};

extern ECvsDiff *CvsDiffView;

#endif
