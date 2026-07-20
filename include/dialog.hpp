#pragma once

#include <memory>
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

	void show()
	{
		// build the node tree first (no rendering), then the backend sizes
		// the window from the engine result and draws
		DialogWrapper::runLayoutEngine(m_title, m_size, m_content.buildNode(), m_resizable);
	}

private:
	std::string m_title;
	Size m_size { -1, -1 };
	bool m_resizable = false;
	Content m_content;
};
