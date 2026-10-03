#include "frameworks_core/wx/OsxSliderTicks.hpp"

#include <wx/window.h>

#import <AppKit/AppKit.h>

int wxOsxSetSliderTickCount(wxWindow* slider, int count)
{
	NSView* view = slider->GetHandle();
	if (![view isKindOfClass:[NSSlider class]])
		return -1;
	NSSlider* native = (NSSlider*)view;
	native.numberOfTickMarks = count;
	return (int)native.numberOfTickMarks;
}

int wxOsxSliderTickCount(wxWindow* slider)
{
	NSView* view = slider->GetHandle();
	if (![view isKindOfClass:[NSSlider class]])
		return -1;
	return (int)((NSSlider*)view).numberOfTickMarks;
}
