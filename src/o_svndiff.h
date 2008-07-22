/*
 * o_svndiff.h
 *
 * S.Pinigin copy o_cvsdiff.h and replace cvs/Cvs/CVS to svn/Svn/SVN.
 *
 * Class showing output from SVN diff command. Allows copying of lines
 * to clipboard and allows to jump to lines in real sources.
 */

#ifndef __SVNDIFF_H__
#define __SVNDIFF_H__

class ESvnDiff: public ESvnBase {
public:
    int CurrLine, ToLine, InToFile;
    char *CurrFile;

    ESvnDiff(int createFlags, EModel **ARoot, char *Dir, char *ACommand, char *AOnFiles);
    ~ESvnDiff();

    void ParseFromTo(char *line, int len);
    virtual void ParseLine(char *line, int len);
    // Returns 0 if OK
    virtual int RunPipe(char *Dir, char *Command, char *Info);

    virtual int ExecCommand(int Command, ExState &State);
    int BlockCopy(int Append);

    virtual int GetContext() {
        return CONTEXT_SVNDIFF;
    }
    virtual EEventMap *GetEventMap();
};

extern ESvnDiff *SvnDiffView;

#endif
