/*    dialog.h
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef DIALOG_H_
#define DIALOG_H_

#define askYes     0
#define askNo      1
#define askCancel  2
#define askOK      3

int AskString(char *Prompt, char *String, int MaxLen);
int AskYesNo(char *Prompt);
int AskYesNoCancel(char *Prompt);
int AskOKCancel(char *Prompt);

#endif
