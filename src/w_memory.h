/* w_memory.h
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 */

#ifndef W_MEMORY_H
#define W_MEMORY_H

int MemoryFetch();
int MemoryStore();
int MemoryFetch2();
int MemoryStore2();
int MemoryHere();
int MemoryAllot();
int MemoryEnd();

int Verbosity();
int Base();
int AutoTrim();
int Insert();
int Mouse();
void InitSharedVars();

#endif // W_MEMORY_H
