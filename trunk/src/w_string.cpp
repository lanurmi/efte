/* w_string.cpp
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

#include "w_string.h"

/// "A" -> 65
int SSAsc() {
    SSCHECK(1, "asc");

    std::string s = sstack.back(); sstack.pop_back();
    ParamStack.push(s[0]);

    return 1;
}

/// 65 -> "A"
int PSChar() {
    PSCHECK(1, "char$");

    char t[2];
    t[0] = ParamStack.pop();
    t[1] = 0;

    sstack.push_back(t);

    return 1;
}

// --- string stack ---

int DupStr() {
    if (sstack.size() == 0) {
        SetBranchCondition(0);
        return 0;
    }

    sstack.push_back(sstack[sstack.size()-1]);

    SetBranchCondition(1);
    return 1;
}

int DropStr() {
    if (sstack.size() == 0) {
        SetBranchCondition(0);
        return 0;
    }

    sstack.pop_back();

    SetBranchCondition(1);
    return 1;
}

int SwapStr() {
    int tosIdx = sstack.size();
    if (tosIdx < 2) {
        SetBranchCondition(0);
        return 0;
    }
    tosIdx--;

    std::string tos = sstack.at(tosIdx);
    sstack[tosIdx] = sstack[tosIdx - 1];
    sstack[tosIdx - 1] = tos;

    SetBranchCondition(1);
    return 1;
}


int DiagStr() {
    fprintf(stderr, "Stringstack: ");

    int tos = sstack.size();
    if (tos)
        for ( int i=0; i<tos ; )
            fprintf(stderr, "'%s' ", sstack[i++].c_str());
    else
        fprintf(stderr, "empty");

    if (verbosity <= 1)                 // don't linefeed if verbosity >1:  command trace will advance.
        fprintf(stderr, "\n");
    return 1;
}


int RotStr() {
    if (sstack.size() < 3) {
        SetBranchCondition(0);
        return 0;
    }

    std::string tos = sstack[sstack.size() - 1];
    sstack.pop_back();

    SwapStr();
    sstack.push_back(tos);
    SwapStr();

    SetBranchCondition(1);
    return 1;
}

int CompareStr() {
    if (sstack.size() < 2) {
        SetBranchCondition(0);
        return 0;
    }

    int tos = sstack.size() - 1;
    int compareTo;

    std::string what = sstack[tos];
    sstack.pop_back();

    compareTo = tos - 1;
    ParamStack.push(-(what.compare(sstack[compareTo])));
    sstack.pop_back();

    SetBranchCondition(1);
    return 1;
}

int OverStr() {
    if (sstack.size() < 2) {
        SetBranchCondition(0);
        return 0;
    }

    sstack.push_back(sstack[sstack.size()-2]);
    SetBranchCondition(1);
    return 1;
}

int PickStr() {
    PSCHECK(1, "pick$");
    int idx = ParamStack.pop();

    SSCHECK((unsigned int) idx + 1, "pick$");
    idx = sstack.size() - 1 - idx;

    sstack.push_back(sstack[idx]);

    SetBranchCondition(1);
    return 1;
}

int DepthStr() {
    ParamStack.push(sstack.size());
    return 1;
}

int SubSearchStr() {
    int tos = sstack.size() - 1;
    int searchInI  = tos - ParamStack.pop();
    int searchForI = tos - ParamStack.pop();

    if (searchInI < 0 || searchForI < 0) {
        SetBranchCondition(0);
        return 0;
    }

    std::string searchIn  = sstack[searchInI];
    std::string searchFor = sstack[searchForI];

    std::string::size_type foundAt = searchIn.find(searchFor, 0);
    ParamStack.push(foundAt == std::string::npos ? -1 : (int) foundAt);

    SetBranchCondition(1);
    return 1;
}

int MergeStr() {
    unsigned int tos = sstack.size();
    if (tos < 2) {
        SetBranchCondition(0);
        return 0;
    }

    tos--; // 0 based index

    std::string tosS = sstack[tos];
    std::string nosS = sstack[tos-1];
    sstack.pop_back();
    sstack.pop_back();

    sstack.push_back(nosS + tosS);

    SetBranchCondition(1);
    return 1;
}

int SplitStr() {
    int tos = sstack.size() - 1;
    if (tos < 0) {
        SetBranchCondition(0);
        return 0;
    }

    unsigned int pos = ParamStack.pop();
    std::string tosS = sstack[tos];

    sstack.pop_back();

    if (pos > tosS.size()) {
        sstack.push_back("");
        sstack.push_back(tosS);
    } else if (pos <= 0) {
        sstack.push_back(tosS);
        sstack.push_back("");
    } else {
        sstack.push_back(tosS.substr(pos));
        sstack.push_back(tosS.substr(0, pos));
    }

    SetBranchCondition(1);
    return 1;
}

int LenStr() {
    int idx = sstack.size() - 1;

    if ((unsigned int) idx >= sstack.size()) {
        SetBranchCondition(0);
        return 0;
    }

    ParamStack.push(sstack[idx].length());

    SetBranchCondition(1);
    return 1;
}

int MidStr() {
    PSCHECK(2, "mid$");
    int end = ParamStack.pop();
    int start = ParamStack.pop();

    SSCHECK(1, "mid$");

    std::string tos = sstack.back();

    if (start < 0) start = tos.length() + start;
    if (end < 0) end = tos.length() + end;

    sstack[sstack.size()-1] = tos.substr(start, end);

    SetBranchCondition(1);
    return 1;
}

int GetString(GxView *view) {
    EView *View = 0;
    if (view != NULL) {
        ExModelView *V = (ExModelView *)view->Top;
        View = V->View;
    } else {
        SetBranchCondition(0);
        return 0;
    }

    SSCHECK(1, "GetString");

    std::string msg = sstack.back(); sstack.pop_back();

    char str[256] = "";
    if (View->MView->Win->GetStr(msg.c_str(), sizeof(str), str, HIST_DEFAULT) == 0) {
        SetBranchCondition(0);
        return 0;
    }

    sstack.push_back(str);

    SetBranchCondition(1);
    return 1;
}
