#include "frameworks_core/UiThread.hpp"

#include <wx/app.h>

void postToUi(std::function<void()> task)
{
	// CallAfter posts an event, which wx allows from any thread; the functor
	// then runs from the main event loop.
	if (wxTheApp != nullptr && task)
		wxTheApp->CallAfter(std::move(task));
}
