/*
 * o_svn.h
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 *
 * S.Pinigin copy o_cvs.h and replace cvs/Cvs/CVS to svn/Svn/SVN.
 *
 * Class providing access to most of SVN commands.
 */

#ifndef SVN_H_
#define SVN_H_

class ESvn: public ESvnBase {
public:
    char *LogFile;
    int Commiting;

    ESvn(int createFlags, EModel **ARoot, const char *Dir, const char *ACommand, const char *AOnFiles);
    ESvn(int createFlags, EModel **ARoot);
    ~ESvn();

    void RemoveLogFile();
    // Return marked files in allocated space separated list
    char *MarkedAsList();
    // Return SVN status char of file or 0 if unknown
    // (if char is lowercase, state was guessed from last command invoked upon file)
    char GetFileStatus(const char *file);

    virtual void ParseLine(const char *line, int len);
    // Returns 0 if OK
    virtual int RunPipe(const char *Dir, const char *Command, const char *OnFiles);
    virtual void ClosePipe();
    // Start commit process (opens message buffer), returns 0 if OK
    int RunCommit(const char *Dir, const char *Command, const char *OnFiles);
    // Finish commit process (called on message buffer close), returns 0 if OK
    int DoneCommit(int commit);

    virtual int CanQuit() const;
    virtual int ConfQuit(GxView *V, int multiFile);

    virtual int GetContext() const;
    virtual EEventMap *GetEventMap();
};

extern ESvn *SvnView;

#endif
