#pragma once

#include <memory>
#include <optional>
#include <string>

#include "frameworks_core/DialogWrapper.hpp"
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

	void show()
	{
		// build the node tree first (no rendering), then the backend sizes
		// the window from the engine result and draws
		DialogWrapper::runLayoutEngine(m_title, m_size, m_content.buildNode(), m_resizable,
			m_position);
	}

private:
	std::string m_title;
	Size m_size { -1, -1 };
	std::optional<Position> m_position;
	bool m_resizable = false;
	Content m_content;
};
