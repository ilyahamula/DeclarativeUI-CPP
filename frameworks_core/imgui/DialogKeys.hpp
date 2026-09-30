#pragma once

#include "frameworks_core/CoreTypes/DialogKeys.hpp"

#include "imgui.h"

#include <functional>

// Enter / Escape for Button::isDefault() / isCancel() on ImGui.
//
// Nothing is retained, so there is no button to look up when a key arrives:
// instead every Dialog/Window opens a WindowScope around its frame, each
// enabled button with a role OFFERS its press into the innermost scope as it
// renders, and the window dispatches after its tree is drawn. A hidden or
// folded button never renders and so never offers, and a disabled one is
// refused (LeafScope, set by ImGuiLayoutBackend::place, which knows the
// cascaded disabled state) -- the same "enabled and visible" wx and Qt check.
//
// The dispatch mirrors what the retained backends decide from focus:
//   * only the focused window (child windows -- a ScrollPanel -- included);
//   * nothing while an item is still active, which is exactly a multi-line
//     field keeping its newline: a single-line field DEACTIVATES on Enter or
//     Escape in the same frame, so by the time the window dispatches it is
//     done -- having reported onEnter first, as QLineEdit does.
// A scope nests: an adopted child window drawn inside its parent's frame
// offers into its own scope, not the parent's.
namespace imgui_dialog_keys
{

struct Offers
{
	std::function<void()> defaultPress;
	std::function<void()> cancelPress;
};

inline Offers* g_current = nullptr;
inline bool g_leafDisabled = false;

class WindowScope
{
public:
	WindowScope()
		: m_previous(g_current)
	{
		g_current = &m_offers;
	}

	~WindowScope() { g_current = m_previous; }

	WindowScope(const WindowScope&) = delete;
	WindowScope& operator=(const WindowScope&) = delete;

	// Inside the window, after its tree has been drawn.
	void dispatch()
	{
		if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) || ImGui::IsAnyItemActive())
			return;
		if (ImGui::GetIO().KeyMods != ImGuiMod_None)
			return;
		// copied out: a handler may show() a window that opens a scope of its own
		if (m_offers.cancelPress && ImGui::IsKeyPressed(ImGuiKey_Escape, false))
		{
			const auto press = m_offers.cancelPress;
			press();
		}
		else if (m_offers.defaultPress && enterPressed())
		{
			const auto press = m_offers.defaultPress;
			press();
		}
	}

	static bool enterPressed()
	{
		return ImGui::IsKeyPressed(ImGuiKey_Enter, false) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, false);
	}

private:
	Offers m_offers;
	Offers* m_previous;
};

// Around one leaf's render(): whether that leaf is disabled, own flag or
// inherited.
class LeafScope
{
public:
	explicit LeafScope(bool disabled)
		: m_previous(g_leafDisabled)
	{
		g_leafDisabled = disabled;
	}

	~LeafScope() { g_leafDisabled = m_previous; }

	LeafScope(const LeafScope&) = delete;
	LeafScope& operator=(const LeafScope&) = delete;

private:
	bool m_previous;
};

// First offer of each role wins: tree order, as on wx and Qt.
inline void offer(unsigned roles, const std::function<void()>& press)
{
	if (g_current == nullptr || g_leafDisabled)
		return;
	if ((roles & kDefaultButton) != 0 && !g_current->defaultPress)
		g_current->defaultPress = press;
	if ((roles & kCancelButton) != 0 && !g_current->cancelPress)
		g_current->cancelPress = press;
}

} // namespace imgui_dialog_keys
