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
// i changed logic to branch in error case, instead of in good case.
// branching will most likely cause the CPU to throw away the work
// it has done in its pipeline, and having to start filling the
// pipeline with operations again.

int DupStr() {
    int ret = 0;
    if (sstack.size()) {
        sstack.push_back(sstack[sstack.size()-1]);
        ret++;
    }
    SetBranchCondition(ret);
    return ret;
}



int DropStr() {
    int ret = 0;
    if (sstack.size()) {
        sstack.pop_back();
        ret++;
    }
    SetBranchCondition(ret);
    return ret;
}


int SwapStr() {
    int ret = 0;
    int tosIdx = sstack.size();
    if (tosIdx > 1) {
        tosIdx--;

        std::string tos = sstack.at(tosIdx);
        sstack[tosIdx] = sstack[tosIdx - 1];
        sstack[tosIdx - 1] = tos;
        ret++;
    }
    SetBranchCondition(ret);
    return ret;

}


int DiagStr() {
    fprintf(stderr, "Stringstack: ");

    int tos = sstack.size();
    if (tos)
        for ( int i=0; i<tos ; )
            fprintf(stderr, "'%s' ", sstack[i++].c_str());
    else
        fprintf(stderr, "empty");

    if (memory[verbosity] < 2)                 // don't linefeed if verbosity >1:  command trace will advance.
        fprintf(stderr, "\n");
    return 1;
}


int RotStr() {
    int ret = 0;
    if (sstack.size() > 2) {
        std::string tos = sstack[sstack.size() - 1];
        sstack.pop_back();
        SwapStr();
        sstack.push_back(tos);
        SwapStr();
        ret++;
    }
    SetBranchCondition(ret);
    return ret;
}



int CompareStr() {
    int ret = 0;
    if (sstack.size() > 1) {
        int tos = sstack.size() - 1;
        int compareTo;

        std::string what = sstack[tos];
        sstack.pop_back();

        compareTo = tos - 1;
        ParamStack.push(-(what.compare(sstack[compareTo])));
        sstack.pop_back();
        ret++;
    }
    SetBranchCondition(ret);
    return ret;
}



int OverStr() {
    int ret = 0;
    if (sstack.size() > 1) {
        sstack.push_back(sstack[sstack.size()-2]);
        ret++;
    }
    SetBranchCondition(ret);
    return ret;
}



int LenStr() {
    int ret = 0;
    int idx = sstack.size();
    if (idx) {
        ParamStack.push(sstack[idx-1].length());
        sstack.pop_back();
        ret++;
    }
    SetBranchCondition(ret);
    return ret;
}



int AppendStr() {
    int ret = 0;
    unsigned int tos = sstack.size();

    if (tos > 1) {

        std::string tosS = sstack[--tos];
        std::string nosS = sstack[--tos];
        sstack.pop_back();
        sstack.pop_back();

        sstack.push_back(nosS + tosS);
        ret++;
    }
    SetBranchCondition(ret);
    return ret;
}



int SplitStr() {
    int ret = 0;
    int tos = sstack.size();

    if (tos) {
        unsigned int pos = ParamStack.pop();
        std::string tosS = sstack[--tos];

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
        ret++;
    }

    SetBranchCondition(ret);
    return ret;
}



int DepthStr() {
    ParamStack.push(sstack.size());
    return 1;
}




// stopped reversing conds here.  TODO: remove this TODO when not needed anymore


int PickStr() {
    PSCHECK(1, "pick$");
    int idx = ParamStack.pop();

    SSCHECK((unsigned int) idx + 1, "pick$");
    idx = sstack.size() - 1 - idx;

    sstack.push_back(sstack[idx]);

    SetBranchCondition(1);
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
