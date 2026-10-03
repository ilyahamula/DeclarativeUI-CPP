#pragma once

class wxWindow;

// macOS only. wxOSX draws a fixed set of tick marks for wxSL_AUTOTICKS and
// ignores wxSlider::SetTickFreq (verified: frequencies 4, 5 and 25 on a 0..100
// slider all draw the same ~21 marks). NSSlider takes the COUNT of tick marks,
// evenly spread from end to end, so this sets it directly. Returns the count
// the native slider now has, or -1 when the view is not an NSSlider.
int wxOsxSetSliderTickCount(wxWindow* slider, int count);

// The native tick count, or -1 when the view is not an NSSlider.
int wxOsxSliderTickCount(wxWindow* slider);
