/*    cocoa.mm
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#import <Cocoa/Cocoa.h>
#import "AppController.h"

int cacao() {
    int argc = 1;
    char *myarg = "";
    char **argv = &myarg;

    AppController *delegate = [[AppController alloc] init];
    [NSApp setDelegate: delegate];

    return NSApplicationMain(argc, (const char **) argv);
}
