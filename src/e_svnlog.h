/*
 * e_svnlog.h
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 * S.Pinigin copy o_cvslog.h and replace cvs/Cvs/CVS to svn/Svn/SVN.
 *
 * Subclass of EBuffer for writing log for SVN commit. Creates temporary file
 * used for commit which is deleted when view is closed. Asks for commit or
 * discard on view close.
 */

#ifndef _SVNLOG_H_
#define _SVNLOG_H_

#include "e_buffer.h"

class ESvnLog: public EBuffer {
public:
    ESvnLog(int createFlags, EModel **ARoot, char *Directory, char *OnFiles);
    ~ESvnLog();

    // List files into buffer
    // p        - line where to print
    // fCount   - number of files which will be printed
    // title    - title
    // cnt      - total number of files
    // position - positions of files in list
    // len      - length of files
    // status   - status of files
    // list     - list of filenames
    // incexc   - status of files to print/not to print
    // exc      - incexc is exclusion
    void ListFiles(int &p, const int fCount, const char *title, const int cnt, const int *position, const int *len, const char *status, const char *list, const char *excinc, const int exc = 0);

    virtual int CanQuit();
    virtual int ConfQuit(GxView *V, int multiFile = 0);
    virtual EViewPort *CreateViewPort(EView *V);

    virtual void GetName(char *AName, int MaxLen);
    virtual void GetInfo(char *AInfo, int MaxLen);
    virtual void GetTitle(char *ATitle, int MaxLen, char *ASTitle, int SMaxLen);
};

extern ESvnLog *SvnLogView;

#endif
