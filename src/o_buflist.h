/*    o_buflist.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef BUFLIST_H_
#define BUFLIST_H_

class BufferView: public EList {
public:
    char **BList;
    int BCount;
    int SearchLen;
    char SearchString[MAXISEARCH];
    int SearchPos[MAXISEARCH];

    BufferView(int createFlags, EModel **ARoot);
    virtual ~BufferView();
    EEventMap *GetEventMap() override;
    int GetContext() const override;
    void DrawLine(PCell B, int Line, int Col, ChColor color, int Width) override;
    char* FormatLine(int Line) const override;
    void UpdateList() override;
    EModel *GetBufferById(int No);
    int ExecCommand(int Command, ExState &State) override;
    void HandleEvent(TEvent &Event) override;
	int GetMatchingLine(int start, int direction) const;
    int Activate(int No) override;
    void CancelSearch();
    void GetInfo(char *AInfo, int MaxLen) const override;
    void GetTitle(char *ATitle, int MaxLen, char *ASTitle, int SMaxLen) const override;
};

#endif
