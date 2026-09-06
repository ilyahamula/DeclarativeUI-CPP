#pragma once

#include "frameworks_core/LayoutEngine.hpp"
#include "frameworks_core/LayoutNode.hpp"
#include "frameworks_core/wx/LayoutBackend.hpp"

#include <wx/wx.h>

#include <algorithm>
#include <functional>
#include <memory>

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

	// Lifecycle seams the caller fills in; both are inert until the elements
	// that set them land. `openFlag` is a caller-owned bool the window is shown
	// against (Dialog/Window::show(bool&), T3.6/T2.1): clearing it closes the
	// window, closing the window clears it. `onClose` fires exactly once when
	// the window goes away.
	bool* openFlag = nullptr;
	std::function<void()> onClose;

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
	// Today that is AutoGrow text fields, whose live content the measure pass
	// reads. The other sources are node-level BoundValues the engine reads the
	// same way and which the user drives directly: a Splitter's sash position
	// (T1.5) and an Expander's collapsed state (T1.6). Both join here as a
	// check on the container node before the recursion below, polling the value
	// with bindExternalRefSync on `window` and calling relayout() when it moves
	// -- they cannot be written yet because neither NodeKind exists.
	void bindInvalidation(LayoutNode& node)
	{
		if (node.isLeaf())
		{
			if (node.flags.autoGrow() && node.widget != nullptr)
			{
				if (auto* control = static_cast<wxWindow*>(node.widget->nativeHandle()))
				{
					control->Bind(wxEVT_TEXT, [this](wxCommandEvent& event) {
						// after the control has applied the edit
						window->CallAfter([this]() { relayout(); });
						event.Skip();
					});
				}
			}
			return;
		}
		for (auto& child : node.children)
			bindInvalidation(*child);
	}
};
