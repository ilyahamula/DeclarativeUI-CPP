// The Cocoa half of wx_backend_test.cpp: a real button click. Kept out of the
// C++ test file because AppKit's headers declare their own Size and Rect.
#import <AppKit/AppKit.h>

extern "C" void wxTestNativeClick(void* nsView)
{
	[(NSButton*)nsView performClick:nil];
}
