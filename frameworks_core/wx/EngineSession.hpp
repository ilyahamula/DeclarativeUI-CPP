#pragma once

#include "frameworks_core/LayoutEngine.hpp"
#include "frameworks_core/LayoutNode.hpp"
#include "frameworks_core/wx/LayoutBackend.hpp"
#include "frameworks_core/wx/RefSync.hpp"

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
	// reads, Splitter sash positions, and Expander open states.
	void bindInvalidation(LayoutNode& node)
	{
		// A sash drag (or anything else writing the bound int) changes a value
		// the engine reads, and wx has no notification for that -- so it is
		// polled like any other external ref. `resolved` is what the layout
		// currently shows and `position` what it should show, which makes this
		// the ordinary RefSync shape rather than a special case: a full
		// re-measure, because the pane widths decide table columns and text
		// wrapping, not just where the rectangles land.
		if (node.kind == NodeKind::Splitter)
		{
			SplitterState* split = &node.split;
			bindExternalRefSync(window,
				[split] { return split->resolved; },
				[split] { return split->position.get(); },
				[this](int) { relayout(); });
		}
		// Clicking a header (or anything else writing the bound bool) changes
		// what the measure pass will find, and wx has no notification for that
		// either. Same RefSync shape as the sash above: `applied` is what the
		// layout currently shows and `expanded` what it should show. It has to
		// be a full re-measure rather than a re-arrange -- a section opening
		// changes what there is to lay out, not merely where it lands.
		if (node.kind == NodeKind::Expander)
		{
			ExpanderState* expander = &node.expander;
			bindExternalRefSync(window,
				[expander] { return expander->applied; },
				[expander] { return expander->expanded.get(); },
				[this](bool) { relayout(); });
		}
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
