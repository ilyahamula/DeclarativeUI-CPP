#pragma once

#include <functional>

// Bound values belong to the UI thread. Every backend reads them from it --
// the wx idle sync, the Qt poll timer, the ImGui frame -- so a worker thread
// writing a bound std::string or vector directly is a data race, not merely a
// value that shows up late.
//
// postToUi() is the one supported way across: it queues `task` to run on the
// UI thread, where it may write bound values, show a Toast or a Dialog, or do
// anything a click handler may. Callable from any thread, returns at once.
//
//   * wx:    wxTheApp->CallAfter -- run from the event loop;
//   * Qt:    a queued invocation on the application object;
//   * ImGui: a locked queue drained by the first Dialog/Window of the next
//            frame, inside its frame -- so a show() from a task is a
//            handler's show() and is kept up until closed (TopLevelShow.hpp).
//
// Tasks run in the order they were posted. One posted after the application
// has shut down is dropped.
void postToUi(std::function<void()> task);

#ifdef USE_IMGUI
namespace UiThreadQueue
{
// Run every task posted since the last drain. Called by the top-level wrappers
// once per frame (the first call of a frame does the work).
void drain();
}
#endif
