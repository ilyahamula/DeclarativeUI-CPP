#pragma once

#include "frameworks_core/LayoutEngine.hpp"
#include "frameworks_core/LayoutNode.hpp"
#include "frameworks_core/NodeSource.hpp"
#include "frameworks_core/wx/DialogKeys.hpp"
#include "frameworks_core/wx/LayoutBackend.hpp"
#include "frameworks_core/wx/RefSync.hpp"

#include <wx/wx.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

// Retained engine state for one shown top-level window: the node tree (owning
// the wrappers), the adapter, and the engine. The window is non-modal, so this
// lives on the heap until the window is destroyed (wxEVT_DESTROY).
//
// The window is a wxTopLevelWindow rather than a wxDialog because the same
// session drives both spellings of a top-level window: Dialog (wxDialog) today
// and Window (wxFrame, T2.1) next. Nothing here is dialog-specific -- the
// caller owns creation, event wiring and teardown, and this owns the layout.
struct EngineSession
{
	wxTopLevelWindow* window = nullptr;
	std::unique_ptr<WxLayoutBackend> backend;
	std::unique_ptr<LayoutEngine> engine;
	std::unique_ptr<LayoutNode> root;
	bool autoFit = true;
	bool resizable = false;
	Size fixedContent { -1, -1 };
	bool busy = false; // re-entrancy guard: SetClientSize fires wxEVT_SIZE

	// Lifecycle the caller fills in. `openFlag` is a caller-owned bool the
	// window is shown against (Dialog/Window::show(bool&)): clearing it closes
	// the window, closing the window clears it. `onClose` fires exactly once
	// when the window goes away.
	bool* openFlag = nullptr;
	std::function<void()> onClose;
	bool closed = false; // one-shot guard for notifyClosed()

	// Dialog::Modal(). Held for as long as the window is up, because that is
	// exactly how long the rest of the application stays disabled: the disabler
	// re-enables every window it touched when it dies. Only a Dialog ever sets
	// one -- an application frame that locked the application out would be a
	// dialog by another name.
	std::unique_ptr<wxWindowDisabler> modalGuard;

	// The one close path, whatever triggered it -- the user's close button or
	// the caller clearing `openFlag`. Both clear the flag and fire onClose
	// once, so a caller cannot tell the two apart by what it observes.
	void notifyClosed()
	{
		if (closed)
			return;
		closed = true;
		// Before the callback, not after: a handler that opens the next dialog
		// must not find the application still locked out behind this one. The
		// native window is only destroyed later (wx defers it to idle), so
		// waiting for the session's own teardown would leave the lock on for
		// longer than the dialog was up.
		modalGuard.reset();
		if (openFlag != nullptr)
			*openFlag = false;
		if (onClose)
			onClose();
	}

	// Polls the caller-owned flag: clearing it closes the window. wx has no
	// notification for a bool written from somewhere else, so this is the
	// ordinary RefSync shape -- `pull` is whether the window is up now, `want`
	// whether the flag says it should be.
	void bindOpenFlag()
	{
		if (openFlag == nullptr)
			return;
		const bool* flag = openFlag;
		bindExternalRefSync(window,
			[this] { return window->IsShown(); },
			[flag] { return *flag; },
			[this](bool) { window->Close(); }); // -> wxEVT_CLOSE_WINDOW -> notifyClosed()
	}

	Size minClient() const
	{
		const EdgeInsets margin = root->flags.border();
		return { root->desired.width + margin.left + margin.right,
				 root->desired.height + margin.top + margin.bottom };
	}

	// Window size changed, measures still valid: arrange-only from the
	// cached desired sizes.
	void rearrange()
	{
		if (busy || window->IsBeingDeleted())
			return;
		busy = true;
		const wxSize client = window->GetClientSize();
		engine->render(*root, { client.x, client.y });
		busy = false;
	}

	// Content or display metrics changed: full re-measure. Auto-fit windows
	// follow their content; resizable ones keep the user's size but never
	// below the new content floor.
	void relayout()
	{
		if (busy || window->IsBeingDeleted())
			return;
		busy = true;

		const Size content = engine->resolve(*root, autoFit ? Size { -1, -1 } : fixedContent);
		if (autoFit && resizable)
		{
			const Size floor = minClient();
			window->SetMinClientSize(wxSize(floor.width, floor.height));
			wxSize client = window->GetClientSize();
			if (client.x < floor.width || client.y < floor.height)
			{
				window->SetClientSize(std::max(client.x, floor.width),
					std::max(client.y, floor.height));
				client = window->GetClientSize();
			}
			engine->render(*root, { client.x, client.y });
		}
		else if (autoFit)
		{
			window->SetClientSize(content.width, content.height);
			engine->render(*root, content);
		}
		else
		{
			const wxSize client = window->GetClientSize();
			engine->render(*root, { client.x, client.y });
		}

		busy = false;
	}

	// Arms a re-measure for every source of engine-visible change in the tree.
	//
	// AutoGrow fields are event-driven: their live text is what measure reads,
	// so an edit re-measures. Everything else -- sash positions, Expander open
	// states, isHidden() flags, ForEach data -- is one poll over the tree
	// (refreshTree, NodeSource.hpp) rather than a poll per node: rows can now
	// be destroyed and rebuilt, and a poll holding a pointer into a node would
	// outlive it. Walking the live tree each time holds none.
	void watch()
	{
		bindAutoGrow(*root);
		window->Bind(wxEVT_IDLE, [this](wxIdleEvent& event) {
			poll();
			event.Skip();
		});
		// Enter / Escape for default and cancel buttons (DialogKeys.hpp).
		window->Bind(wxEVT_CHAR_HOOK, [w = window](wxKeyEvent& event) {
			wx_dialog_keys::onCharHook(w, event);
		});
	}

	void poll()
	{
		if (busy)
			return;
		std::vector<LayoutNode*> fresh;
		if (!refreshTree(*root, *backend, fresh))
			return;
		relayout(); // realizes the fresh rows
		for (LayoutNode* row : fresh)
			bindAutoGrow(*row);
	}

	void bindAutoGrow(LayoutNode& node)
	{
		if (!node.isLeaf())
		{
			for (auto& child : node.children)
				bindAutoGrow(*child);
			return;
		}
		if (!node.flags.autoGrow() || node.widget == nullptr)
			return;
		if (auto* control = static_cast<wxWindow*>(node.widget->nativeHandle()))
		{
			control->Bind(wxEVT_TEXT, [this](wxCommandEvent& event) {
				// after the control has applied the edit
				window->CallAfter([this]() { relayout(); });
				event.Skip();
			});
		}
	}
};
