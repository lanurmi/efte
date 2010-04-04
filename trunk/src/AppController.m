/*    AppController.m
 *
 *    Copyright (c) 2010, eFTE SF Group (see AUTHORS file)
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */
 
#import "AppController.h"


@implementation AppController

- (void) timerFired: (id) blah
{
    MyDispatchEvent();
}

- (id) init {
    if ((self = [super init])) {
        /* class-specific initialization goes here */
        NSTimer *timer = [NSTimer timerWithTimeInterval: 0.0f
                      target: self
                      selector: @selector( timerFired: )
                      userInfo: nil
                      repeats: YES];

        [[NSRunLoop currentRunLoop] addTimer: timer
                      forMode: NSDefaultRunLoopMode];
    }
    return self;
}

@end
