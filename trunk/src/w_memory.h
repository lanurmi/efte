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

int MemoryDump();
int MemoryStore();
int MemoryFetch();
int MemoryHere();
int MemoryAllot();
int MemoryEnd();

int Verbosity();
int Base();
int AutoTrim();
int Insert();
void InitSharedVars();
int MouseXY();

#endif // W_MEMORY_H
