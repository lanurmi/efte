/*    o_modemap.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef MAPVIEW_H_
#define MAPVIEW_H_

class EventMapView: public EList {
public:
    char **BList;
    int BCount;
    EEventMap *EMap;

    void AddLine(const char *Line);
    void DumpKey(const char *aPrefix, EKey *Key);
    void DumpMap(const char *aPrefix, EKeyMap *aKeyMap);
    void DumpEventMap(EEventMap *aEventMap);

    EventMapView(int createFlags, EModel **ARoot, EEventMap *Map);
    virtual ~EventMapView();

    void FreeView();
    void ViewMap(EEventMap *Map);

    int ExecCommand(int Command, ExState &State) override;
    EEventMap *GetEventMap() override;
    int GetContext() const override;

    void DrawLine(PCell B, int Line, int Col, ChColor color, int Width) override;
    char* FormatLine(int Line) const override;
    void UpdateList() override;
    int CanActivate(int Line) const override;
    void GetName(char *AName, int MaxLen) const override;
    void GetInfo(char *AInfo, int MaxLen) const override;
    void GetTitle(char *ATitle, int MaxLen, char *ASTitle, int SMaxLen) const override;
};

extern EventMapView *TheEventMapView;

#endif
