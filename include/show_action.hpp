#pragma once

#include <concepts>
#include <functional>
#include <memory>
#include <utility>

#include "frameworks_core/TopLevelShow.hpp"
#include "buildable.hpp"

// A command callback that opens a Dialog or Window, declared in place:
//
//     MenuItem{"About"}.onSelect(ShowAction(Dialog{"About", ...}))
//     ToolItem{"Rename"}.onClick(ShowAction(renameOpen, Dialog{...}))
//
// It is a plain std::function<void()>, so it fits every callback that is a
// COMMAND -- a menu item, a tool, a link, a button -- and none that reports a
// value: onChange(bool) or onResult(path) cannot take it, which is the point.
// A window that depends on the value a callback reports cannot be declared
// ahead of it anyway; there, call show() inside the handler.
//
// `open` is the single truth about whether the window is up, exactly as for
// show(bool&) -- which is what this calls -- and it is also what keeps it to
// ONE: firing while the flag is set does nothing, on every backend. Closing the
// window clears the flag, so the next firing opens it again, rebuilt from this
// declaration. The flag must outlive both the callback's owner and the window,
// as for any show(bool&) from a handler.
//
// The window is copied per firing: show() consumes what it shows, and every
// firing needs a fresh one. It is held by shared_ptr so the callback stays
// cheap to copy -- the retained backends copy a menu model, handlers and all.
template<FlagShowable W>
	requires std::copy_constructible<W>
std::function<void()> ShowAction(bool& open, W window)
{
	return [flag = &open, shared = std::make_shared<W>(std::move(window))]() {
		if (*flag)
			return;
		// Set before show(), not after: it is what the window's lifetime
		// follows from here on, and a handler's show() reads it at once.
		*flag = true;
		W fresh = *shared;
		fresh.show(*flag);
	};
}

// The same, for a caller who has no use for the flag: the framework owns it,
// keyed by the window's title, so it is still ONE at a time on every backend.
// Not owned by the callback -- on ImGui a menu or a Button is rebuilt every
// frame and would forget, and on wx and Qt the window can outlive whatever
// opened it and would poll a dead flag. Two actions showing the same title
// therefore share one flag, which is right: a title is one window on ImGui.
template<TitledTopLevel W>
	requires std::copy_constructible<W>
std::function<void()> ShowAction(W window)
{
	bool& open = TopLevelShow::ownedOpenFlag(window.title());
	return ShowAction(open, std::move(window));
}
