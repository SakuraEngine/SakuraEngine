#include "platform/apple/macos/window.h"

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#include "stdio.h"
#include "utils/log.h"

@interface DemoView : NSView // interface of DemoView class
{                            // (subclass of NSView class)
}
@end

@implementation DemoView
- (void)windowWillClose:(NSNotification*)notification
{
    [NSApp terminate:self];
}
@end // end of DemoView implementation

void* nswindow_create()
{
    NSWindow* myWindow;  // typed pointer to NSWindow object
    NSView* myView;      // typed pointer to NSView object
    NSRect graphicsRect; // contains an origin, width, height

    // initialize the rectangle variable
    graphicsRect = NSMakeRect(100.0, 350.0, 400.0, 400.0);

    myWindow = [[NSWindow alloc] // create the window
    initWithContentRect:graphicsRect
              styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable
                backing:NSBackingStoreBuffered
                  defer:NO];

    [myWindow setTitle:@"Tiny Application Window"];

    // create amd initialize the DemoView instance
    myView = [[[DemoView alloc] initWithFrame:graphicsRect] autorelease];

    [myWindow setContentView:myView]; // set window's view

    [myWindow makeKeyAndOrderFront:nil]; // display window
    return (void*)myWindow;
}

void* nswindow_get_content_view(void* window)
{
    NSWindow* nswin = (NSWindow*)window;
    NSView* nsview = [nswin contentView];

    NSBundle* bundle = [NSBundle bundleWithPath:@"/System/Library/Frameworks/QuartzCore.framework"];
    if (!bundle)
    {
        SKR_LOG_FATAL("Cocoa: Failed to find QuartzCore.framework\n");
        return nullptr;
    }

    // NOTE: Create the layer here as makeBackingLayer should not return nil
    nsview.layer = [[bundle classNamed:@"CAMetalLayer"] layer];
    if (!nsview.layer)
    {
        SKR_LOG_FATAL("Cocoa: Failed to create layer for view\n");
        return nullptr;
    }
    [nsview setWantsLayer:YES];

    return (void*)nsview;
}