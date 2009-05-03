/* w_memory.cpp
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

#define MEMORY_LIMIT 5242880
std::vector<int> memory;



int MemoryStore() {
    PSCHECK(2, "!");
    int ret = 0;
    int loc = ParamStack.pop();

    if (loc < MEMORY_LIMIT) {

        int initialized = memory.size();
        while (loc >= initialized++ )
            memory.push_back(0);

        memory[loc] = ParamStack.pop();
        ret++;
    }
    SetBranchCondition(ret);
    return ret;
}


int MemoryFetch() {
    PSCHECK(1, "@");
    int ret = 0;
    int loc = ParamStack.pop();

    if (loc < MEMORY_LIMIT) {

        int initialized = memory.size();
        while (loc >= initialized++ )
            memory.push_back(0);

        ParamStack.push(memory[loc]);
        ret++;
    }

    SetBranchCondition(ret);
    return ret;
}



int MemoryFetch2() {
    PSCHECK(1, "@2");
    int ret = 0;
    int loc = ParamStack.pop();

    if (loc+1 < MEMORY_LIMIT) {

        int initialized = memory.size();
        while (loc+1 >= initialized++ )
            memory.push_back(0);

        ParamStack.push(memory[loc]);
        ParamStack.push(memory[loc+1]);
        ret++;
    }

    SetBranchCondition(ret);
    return ret;
}


int MemoryStore2() {
    PSCHECK(3, "!2");
    int ret = 0;
    int loc = ParamStack.pop();

    if (loc < MEMORY_LIMIT) {

        int initialized = memory.size();
        while (loc+1 >= initialized++ )
            memory.push_back(0);

        memory[loc+1] = ParamStack.pop();
        memory[loc] = ParamStack.pop();
        ret++;
    }
    SetBranchCondition(ret);
    return ret;
}




unsigned int dp=0;            // "dictionary pointer". pointer to free memory. what is below, is allocated memory.


int MemoryHere()  {
    ParamStack.push(dp);
    return 1;
}

int MemoryAllot()  {
    PSCHECK(1, "allot");
    int ret = 1;
    int requested = ParamStack.pop();
    dp += requested;
    if (dp > (MEMORY_LIMIT-1024)) {
        dp -= requested;
        ret--;
    }
    SetBranchCondition(ret);
    return ret;
}


int MemoryEnd() {
    ParamStack.push(MEMORY_LIMIT);
    return 1;
}




// --- shared variables ---
void InitSharedVars() {          // --- need to init before they can be used ---
    int initialized = memory.size();
    while (dp > initialized++ )
        memory.push_back(0);
}

unsigned int verbosity = dp++;
int Verbosity() {
    ParamStack.push(verbosity);
    return 1;
}

unsigned int base = dp++;
int Base() {
    ParamStack.push(base);
    return 1;
}

unsigned int autotrim = dp++;
int AutoTrim() {
    ParamStack.push(autotrim);
    return 1;
}

unsigned int insert = dp++;
int Insert() {
    ParamStack.push(insert);
    return 1;
}


unsigned int mouse             = dp;      // refers to first shared mousevar
unsigned int mousex            = dp++;
unsigned int mousey            = dp++;
unsigned int mousexrelative    = dp++;
unsigned int mouseyrelative    = dp++;
unsigned int mousebutton       = dp++;
unsigned int mousewinsizex     = dp++;
unsigned int mousewinsizey     = dp++;
unsigned int mouseeventcounter = dp++;
unsigned int mouseeventtype    = dp++;

int Mouse() {
    ParamStack.push(mouse);
    return 1;
}