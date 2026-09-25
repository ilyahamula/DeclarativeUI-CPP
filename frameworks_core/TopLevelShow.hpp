#pragma once

#include <functional>
#include <string>

// A top-level show() -- Dialog's or Window's -- issued from INSIDE another
// top-level's frame: a click handler, a menu action, a file dialog's result,
// an onClose() callback.
//
// On wx and Qt that is simply a show(): the native window outlives the call, so
// "one show() per open" holds wherever the call comes from. On ImGui it does
// not. A window exists only while someone submits it every frame, and a handler
// runs once -- so a Dialog shown from an onClick would be drawn for exactly the
// frame of the click and then vanish. ImGui therefore ADOPTS such a show: the
// Dialog (or Window) is kept, and drawn every frame until it closes, which gives
// the handler the retained backends' contract. A show() from the caller's own
// frame loop -- outside every top-level -- is untouched: it is the frame, as it
// always was.
//
// `Present` is the adopted window drawing itself once against `open`, the
// flag that decides whether it is up.
namespace TopLevelShow
{
	using Present = std::function<void(bool* open)>;

#ifdef USE_IMGUI
	// True while a top-level is mid-frame, i.e. when a show() now comes from a
	// handler rather than from the caller's frame loop.
	bool issuedFromFrame();

	// Keep `present` and draw it every frame until `open` (or, when null, a
	// flag the registry owns) is cleared. Keyed by `title`, as ImGui keys the
	// window itself: adopting a title that is already up replaces it, so a
	// second click refreshes the one window rather than opening another.
	void adopt(const std::string& title, bool* open, Present present);
#else
	inline bool issuedFromFrame() { return false; }

	// Never reached on a retained backend -- issuedFromFrame() is false -- but
	// spelled out so the shared headers compile unchanged, and correct anyway.
	inline void adopt(const std::string&, bool* open, Present present) { present(open); }
#endif
}
