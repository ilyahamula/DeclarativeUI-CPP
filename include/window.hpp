#pragma once

#include <functional>
#include <memory>
#include <string>
#include <utility>

#include "frameworks_core/WindowWrapper.hpp"
#include "buildable.hpp"
#include "menus.hpp"

// A top-level application frame: wxFrame on wx, QMainWindow on Qt, an ImGui
// window inside the host viewport on ImGui (the same viewport Dialog lives in --
// the mains own the GLFW window).
//
// Window and Dialog are the two spellings of a top-level window and share the
// engine session behind them; what differs is the role, and with it the default:
//
//   * a Dialog is a transient, non-resizable box the engine sizes exactly;
//   * a Window is the application frame, so it is RESIZABLE by default with the
//     auto-fit size as its floor. Fixed() opts back out.
//
// The reason Window exists at all is chrome a dialog cannot carry: wxMenuBar
// needs a wxFrame, and QMenuBar wants a QMainWindow. withMenuBar() is that
// chrome -- outside the engine's content space on wx and Qt, a row inside the
// ImGui window, and the same model walked by all three either way.
template<NodeBuildable Content>
struct Window
{
	Window(const std::string& title, Content content)
		: m_title(title)
		, m_content(std::move(content))
	{
	}

	Window(const std::string& title, Size size, Content content)
		: m_title(title)
		, m_size(size)
		, m_content(std::move(content))
	{
	}

	// Attach the application menu bar. It is native chrome on wx and Qt --
	// outside the client area, so the engine never sees it -- and a menu row
	// inside the ImGui window, whose height is taken off the content space.
	// Taken by value and copied into the backend: the bar outlives this
	// Window, which is a temporary that dies with the show() expression.
	Window& withMenuBar(MenuBar menuBar)
	{
		m_menuBar = std::move(menuBar);
		return *this;
	}

	// Opt out of user resizing: the engine's auto-fit result becomes the
	// window's one size. The inverse of Dialog::Resizable(), because an
	// application frame that cannot be resized is the unusual case.
	Window& Fixed()
	{
		m_resizable = false;
		return *this;
	}

	// Fires exactly once when the window closes, whichever side closed it --
	// the user hitting the close button or the caller clearing the bool the
	// window was shown against.
	Window& onClose(std::function<void()> callback)
	{
		m_onClose = std::move(callback);
		return *this;
	}

	void show()
	{
		WindowWrapper::runLayoutEngine(m_title, m_size, m_content.buildNode(), m_resizable,
			menuBarModel(), std::move(m_onClose), nullptr);
	}

	// Show against a caller-owned flag. The flag is the single truth about
	// whether the window is up: clearing it closes the window, and closing the
	// window clears it. Either way onClose() fires once.
	//
	// The ref has to outlive the window, which is modeless on every backend --
	// so it belongs to the caller's frame loop (ImGui) or to something that
	// lives as long as the event loop (wx/Qt), never to the calling scope.
	void show(bool& open)
	{
		WindowWrapper::runLayoutEngine(m_title, m_size, m_content.buildNode(), m_resizable,
			menuBarModel(), std::move(m_onClose), &open);
	}

private:
	// Null when no bar was attached, which is what tells a backend to leave the
	// chrome (and, on ImGui, the row's height) out altogether.
	const MenuBarModel* menuBarModel() const
	{
		return m_menuBar.empty() ? nullptr : &m_menuBar;
	}

	std::string m_title;
	Size m_size { -1, -1 };
	bool m_resizable = true; // an application frame resizes by default
	std::function<void()> m_onClose;
	MenuBarModel m_menuBar;
	Content m_content;
};
