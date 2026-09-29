#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "frameworks_core/DialogWrapper.hpp"
#include "frameworks_core/TopLevelShow.hpp"
#include "buildable.hpp"

template<NodeBuildable Content>
struct Dialog
{
	Dialog(const std::string& title, Content content)
		: m_title(title)
		, m_content(std::move(content))
	{
	}

	Dialog(const std::string& title, Size size, Content content)
		: m_title(title)
		, m_size(size)
		, m_content(std::move(content))
	{
	}

	// Opt-in user resizing. The auto-fit size becomes the initial AND
	// minimum window size, so content can never be shrunk into clipping.
	// Dialogs are not user-resizable by default.
	Dialog& Resizable()
	{
		m_resizable = true;
		return *this;
	}

	// Place the window's top-left corner. Unset (the default) leaves the
	// placement to the platform. Not a Size-style {-1,-1} sentinel: {0,0} is
	// a legal position and negative coordinates are legal on a multi-monitor
	// desktop, so "unset" needs to be its own state.
	//
	// The user can still move the window afterwards -- this is where it opens,
	// not where it is pinned. Coordinates are desktop-absolute on wx and Qt;
	// on ImGui a dialog is a window inside the host window, so they are
	// relative to the host viewport.
	Dialog& setPosition(const Position& pos)
	{
		m_position = pos;
		return *this;
	}

	// Lock the rest of the application out until this dialog closes, without
	// blocking: show() still returns immediately on every backend, so the
	// caller's loop keeps running and the dialog reports what the user chose
	// through its own callbacks rather than through a return value.
	//
	// Modality is a Dialog's alone. A Window is the application frame, and a
	// frame that refuses input to everything else is a dialog by another name.
	Dialog& Modal()
	{
		m_modal = true;
		return *this;
	}

	// Fires exactly once when the dialog closes, whichever side closed it --
	// the user hitting the close button or the caller clearing the bool the
	// dialog was shown against.
	Dialog& onClose(std::function<void()> callback)
	{
		m_onClose = std::move(callback);
		return *this;
	}

	// Callable from a handler -- an onClick, a menu action, another dialog's
	// onClose() -- on every backend, with the retained backends' meaning: one
	// show() per open. On ImGui that takes the framework keeping this Dialog
	// and drawing it every frame until it closes (TopLevelShow.hpp); called
	// from the caller's own frame loop it is that frame, as before.
	void show()
	{
		showFrom(nullptr);
	}

	const std::string& title() const
	{
		return m_title;
	}

	// Show against a caller-owned flag. The flag is the single truth about
	// whether the dialog is up: clearing it closes the dialog, and closing the
	// dialog clears it. Either way onClose() fires once.
	//
	// The ref has to outlive the dialog, which is modeless on every backend
	// even when Modal() -- so it belongs to the caller's frame loop (ImGui) or
	// to something that lives as long as the event loop (wx/Qt), never to the
	// calling scope.
	//
	// Re-OPENING is the other half of the contract and it is not symmetric:
	// setting the flag back is enough on ImGui, where the caller calls show()
	// every frame, but wx and Qt destroyed the native dialog when it closed, so
	// there a second show() call is what brings it back. One show() per open on
	// a retained backend, one show() per frame on an immediate one -- unless
	// the show() comes from a handler, which is one per open everywhere: ImGui
	// then keeps the dialog, and `open` with it, until the flag is cleared.
	void show(bool& open)
	{
		showFrom(&open);
	}

private:
	void showFrom(bool* open)
	{
		if (TopLevelShow::issuedFromFrame())
		{
			// A handler's show() on ImGui. This Dialog is a temporary that dies
			// with the handler's expression, so it moves into the one copy the
			// registry keeps and rebuilds from every frame. onClose is COPIED
			// per frame: every frame's call has to be able to fire it.
			auto self = std::make_shared<Dialog>(std::move(*this));
			TopLevelShow::adopt(self->m_title, open,
				[self](bool* flag) { self->present(flag, self->m_onClose); });
			return;
		}
		present(open, std::move(m_onClose));
	}

	void present(bool* open, std::function<void()> onClose)
	{
		// build the node tree first (no rendering), then the backend sizes
		// the window from the engine result and draws
		DialogWrapper::runLayoutEngine(m_title, m_size, m_content.buildNode(), m_resizable,
			m_position, m_modal, std::move(onClose), open);
	}

	std::string m_title;
	Size m_size { -1, -1 };
	std::optional<Position> m_position;
	bool m_resizable = false;
	bool m_modal = false;
	std::function<void()> m_onClose;
	Content m_content;
};
