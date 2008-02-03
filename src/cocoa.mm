//
//  main.m
//  fte
//
//  Created by Lauri Nurmi on 5.1.2008.
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//

#import <Cocoa/Cocoa.h>

int cacao()
{
	int argc = 1;
	char *myarg = "";
	char **argv = &myarg;
    return NSApplicationMain(argc,  (const char **) argv);
}
