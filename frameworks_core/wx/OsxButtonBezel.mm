#include "frameworks_core/wx/OsxButtonBezel.hpp"

#include <wx/window.h>

#import <AppKit/AppKit.h>

void wxOsxAllowTallButton(wxWindow* button)
{
	NSView* view = button->GetHandle();
	if (![view isKindOfClass:[NSButton class]])
		return;
	NSButton* native = (NSButton*)view;
	if (native.bezelStyle == NSBezelStyleRounded)
		native.bezelStyle = NSBezelStyleRegularSquare;
}
