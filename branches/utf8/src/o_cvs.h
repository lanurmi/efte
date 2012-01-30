/*
 * o_cvs.h
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 * Contributed by Martin Frydl <frydl@matfyz.cz>
 *
 * Class providing access to most of CVS commands.
 */

#ifndef CVS_H_
#define CVS_H_

class ECvs: public ECvsBase {
public:
    char *LogFile;
    int Commiting;

    ECvs(int createFlags, EModel **ARoot, char *Dir, char *ACommand, char *AOnFiles);
    ECvs(int createFlags, EModel **ARoot);
    ~ECvs();

    void RemoveLogFile();
    // Return marked files in allocated space separated list
    char *MarkedAsList();
    // Return CVS status char of file or 0 if unknown
    // (if char is lowercase, state was guessed from last command invoked upon file)
    char GetFileStatus(char *file);

    virtual void ParseLine(char *line, int len);
    // Returns 0 if OK
    virtual int RunPipe(char *Dir, char *Command, char *OnFiles);
    virtual void ClosePipe();
    // Start commit process (opens message buffer), returns 0 if OK
    int RunCommit(char *Dir, char *Command, char *OnFiles);
    // Finish commit process (called on message buffer close), returns 0 if OK
    int DoneCommit(int commit);

    virtual int CanQuit();
    virtual int ConfQuit(GxView *V, int multiFile);

    virtual int GetContext() {
        return CONTEXT_CVS;
    }
    virtual EEventMap *GetEventMap();
};

extern ECvs *CvsView;

#endif
