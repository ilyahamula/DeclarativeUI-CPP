#pragma once

#include "frameworks_core/CoreTypes/GeneralTypes.hpp"

#include <string>

// ImGui's half of ToastWrapper: the toasts that are still up, and the one hook
// the top-level wrappers call to draw them.
//
// Nothing here is a window. A toast is drawn on the FOREGROUND draw list, which
// is what keeps it above everything -- a Modal() dialog's dimming included, a
// plain overlay window would sit under it -- and what makes it take no focus,
// no input and no ImGui id, so showing one never renumbers anybody's state.
namespace ToastQueue
{

// Enqueue a toast. Its clock starts on the first frame that DRAWS it, not here,
// so a toast shown while no framework window is up is not half gone by the time
// one is.
void push(const std::string& message, MessageBoxStyle style, int durationMs);

// Draw the live toasts at the main viewport's bottom-right and drop the expired
// ones. Dialog/Window runLayoutEngine() call it on every show(), so an app that
// draws any framework window gets toasts for free -- but only the FIRST call of
// a frame draws: a frame with three windows must not draw every toast three
// times. A toast pushed after that call (by a later window's handler) is drawn
// from the next frame.
void draw();

} // namespace ToastQueue
