/* w_misc.cpp
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

void StackTrace()  {
    if (memory[verbosity] > 1) {

        // Param Stack
        for (int idx=ParamStack.size()-1; idx > -1; idx--)
            fprintf(stderr, "%i ", ParamStack.peek(idx));

        // String Stack
        fprintf(stderr, "   ");

        for (int idx=sstack.size()-1; idx > -1; idx--) {
            std::size_t  found=std::string::npos;
            std::string s = sstack[idx];
            while((found = s.find("\n")) != std::string::npos)
                s.replace(found, 1, "\\n");
            while((found = s.find("\r")) != std::string::npos)
                s.replace(found, 1, "\\r");
            fprintf(stderr, "'%s' ", s.c_str());
        }
        fprintf(stderr, "\n");
    }
}

#define INDENT 3
unsigned int indent = 0;

void Dodent()  {
    fprintf(stderr, "\n");
    int i = indent;
    for ( ; i>1; i--) {
        fprintf(stderr, "|");
        int j = INDENT-1;
        for ( ;j ;j--)
            fprintf(stderr, " ");

    }
}

void Redent(int change)  { indent += change; }
void Nodent()            { indent=0; }
void Indent()            { Redent(1);  }
void Undent()            { Redent(-1); }
