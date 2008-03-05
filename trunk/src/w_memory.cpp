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

int MemoryDump() {
    for (std::vector<int>::size_type i=0; i < memory.size(); i++) {
        if (i>0) fprintf(stderr, ", ");
        fprintf(stderr, "%i=%i", (int)i, memory[i]);
    }
    fprintf(stderr, "\n");

    return 1;
}

int MemoryStore() {
    PSCHECK(2, "!");
    int loc = ParamStack.pop();

    if (loc > MEMORY_LIMIT) {
        SetBranchCondition(0);
        return 0;
    }

    int initialized = memory.size();
    while (loc >= initialized++ )
        memory.push_back(0);

    memory[loc] = ParamStack.pop();

    SetBranchCondition(1);
    return 1;
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




unsigned int dp=0;            // "dictionary pointer". pointer to free memory. what is below, is allocated memory.



int MemoryHere()  {
    ParamStack.push(dp);
    return 1;
}

int MemoryAllot()  {
    PSCHECK(1, "allot");

    int requested = ParamStack.pop();
    if (dp+requested+1024 >= MEMORY_LIMIT) {
        SetBranchCondition(0);
        return 0;
    }
    dp += requested;
    return 1;
}

int MemoryEnd() {
    ParamStack.push(MEMORY_LIMIT);
    return 1;
}





// --- shared variables ---
void InitSharedVars() {          // --- need to init before they can be used ---
    fprintf(stderr,"init shared vars to dp=%d\n", dp);
    int initialized = memory.size();
    while (dp > initialized++ )
        memory.push_back(0);
}


unsigned int verbosity = dp++;
int Verbosity() {
    ParamStack.push(verbosity);
    return 1;
}


