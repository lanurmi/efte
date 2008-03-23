/* w_number.h
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 */

#ifndef W_NUMBER_H
#define W_NUMBER_H

int Print(GxView *view, ExState &State);
int ParamDepth();
int Plus();
int Minus();
int Mul();
int Div();
int SlashMod();
int Random();
int And();
int Or();
int Xor();
int Shift();
int Equals();
int Less();
int Flag();
int Fail();
int Dup();
int Drop();
int Swap();
int Over();
int Rot();
int ToR();
int RFrom();
int RFetch();
int I();
int J();

#endif // W_NUMBER_H
