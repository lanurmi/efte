/*    FTETextAreaView.m
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#import "FTETextAreaView.h"

@implementation FTETextAreaView

extern unsigned char *ScreenBuffer;
extern unsigned int ScreenCols, ScreenRows;

- (id)initWithFrame:(NSRect)frameRect
{
	NSRect realSize = NSMakeRect(0, 0, ScreenCols*9, ScreenRows * 18);
	//[[self window] setContentSize:NSMakeSize(10.0,8.0)];
	if ((self = [super initWithFrame:frameRect]) != nil) {
		// Add initialization code here
		MyDispatchEvent();
	}
	return self;
}

void MyResizeWindow(int x, int y);

- (void)viewDidEndLiveResize
{
	NSLog(@"size changed");
	NSRect mySize = [self bounds];
	MyResizeWindow(mySize.size.width, mySize.size.height);
	[self setNeedsDisplay:YES];
}

static const NSColor* getNSColorForDOSColor(int color) {
	const NSColor *table[] = {
		[NSColor blackColor], // 0
		[NSColor blueColor],  // 1
		[NSColor greenColor], // 2
		[NSColor cyanColor],  // 3
		[NSColor redColor],   // 4
		[NSColor purpleColor],// 5
		[NSColor brownColor], // 6
		[NSColor lightGrayColor],// 7

		[NSColor darkGrayColor], // 8
		[NSColor blueColor], // !!!
		[NSColor greenColor],// !!!
		[NSColor cyanColor],// 11
		[NSColor redColor],// !!!
		[NSColor magentaColor],//13
		[NSColor yellowColor], //14
		[NSColor whiteColor], // 15
	};

	return table[color];
}

- (void)drawRect:(NSRect)rect
{
	[[NSColor redColor] set];
	NSRect bounds = [self bounds];
	NSRectFill([self bounds]);
	NSMutableDictionary *attribs = [NSMutableDictionary dictionary];
	
	int x, y;
	const unsigned char *sb = ScreenBuffer;
	[attribs setObject:[NSFont fontWithName:@"Courier" size:14] forKey:NSFontAttributeName];
	
	fprintf(stderr, "size: %d x %d, %fx%f\n", ScreenRows, ScreenCols, bounds.size.width, bounds.size.height);

	for(y = 0; y < ScreenRows; y++)
		for(x = 0; x < ScreenCols; x++) {
			int fore = sb[1] & 0xf, back = (sb[1] >> 4);
			char theChar[2] = {0};
			//fprintf(stderr, "color: %d %d ", fore, back );
			
			[attribs setObject:getNSColorForDOSColor(fore) forKey:NSForegroundColorAttributeName];
			[attribs setObject:getNSColorForDOSColor(back) forKey:NSBackgroundColorAttributeName];
	
			if(sb[0] < 32 || sb[0] > 126)
				*theChar = '?';
			else
				*theChar = sb[0];
			
			NSString *hello = [NSString stringWithCString:theChar length:1];
			sb += 2;
			[hello drawAtPoint:NSMakePoint(8*x, bounds.size.height-18*(y+1)) withAttributes:attribs];
		}
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)mouseDown:(NSEvent *)theEvent {
    [super mouseDown:theEvent];
	NSLog(@"mouse");
	MyDispatchKeyEvent('m');
	[self setNeedsDisplay:YES];
}

void *theGlobalGUI = 0;
void MyDispatchEvent();

- (void)keyDown:(NSEvent *)theEvent {
	NSLog(@"key pressed");
	NSLog([theEvent characters]);
	
	MyDispatchKeyEvent([[theEvent characters] characterAtIndex:0]);
	[self setNeedsDisplay:YES];
	[self display];

//	DispatchEvent(frames, NextEvent.Msg.View, NextEvent);
//	NextEvent.What = evNone;
}


@end
