#ifndef __EMARK_H
#define __EMARK_H

class EMark {
public:
    EMark(char *aName, char *aFileName, EPoint aPoint, EBuffer *aBuffer = 0);
    ~EMark();

    int setBuffer(EBuffer *aBuffer);
    int removeBuffer(EBuffer *aBuffer);

    char *getName() { return Name; }
    char *getFileName() { return FileName; }
    EPoint &getPoint();
    EBuffer *getBuffer() { return Buffer; }
private:
    /* bookmark */
    char *Name;
    EPoint Point;
    char *FileName;

    /* bookmark in file */
    EBuffer *Buffer;
};

class EMarkIndex {
public:
    EMarkIndex();
    ~EMarkIndex();

    EMark *insert(char *aName, char *aFileName, EPoint aPoint, EBuffer *aBuffer = 0);
    EMark *insert(char *aName, EBuffer *aBuffer, EPoint aPoint);
    EMark *locate(char *aName);
    int remove(char *aName);
    int view(EView *aView, char *aName);

//    int MarkPush(EBuffer *B, EPoint P);
//    int MarkPop(EView *V);
//    int MarkSwap(EView *V, EBuffer *B, EPoint P);
//    int MarkNext(EView *V);
    //    int MarkPrev(EView *V);
    EMark *pushMark(EBuffer *aBuffer, EPoint P);
    int popMark(EView *aView);

    int retrieveForBuffer(EBuffer *aBuffer);
    int storeForBuffer(EBuffer *aBuffer);

    int saveToDesktop(FILE *fp);

private:
    int markCount;
    EMark **marks;
};

extern EMarkIndex markIndex;

#endif
