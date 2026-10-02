#pragma once

#include "frameworks_core/CoreTypes/TextField.hpp"

#include "imgui.h"

#include <cstdint>
#include <unordered_map>

// Focus and validity of an ImGui text field (TextField.hpp).
//
// A field is focused exactly while its item is ACTIVE (being edited) -- ImGui
// keeps no other focus for an item unless keyboard navigation is on. The tree
// is rebuilt every frame, so whether the field was active last frame lives
// here, keyed like the field's own snapshot (WidgetSnapshot::slotKey), and a
// change between frames is a focus or a blur.
//
// One difference from wx and Qt follows from that definition: Enter in a
// single-line field ends the edit, so it is a blur here, where on wx and Qt the
// caret stays in the field.
namespace imgui_text_field
{

struct FieldState
{
	bool active = false;
	bool seen = false;
};

inline std::unordered_map<std::uint64_t, FieldState>& fieldStates()
{
	static std::unordered_map<std::uint64_t, FieldState> states;
	return states;
}

// Open right before the field's input item, call after() right after it.
class Scope
{
public:
	Scope(TextFieldOptions& options, std::uint64_t key)
		: m_options(options)
		, m_state(fieldStates()[key])
		, m_invalid(options.invalid.get())
	{
		// A bound flag asks for focus until the field has it; a snapshot
		// `true` asks once, the first frame the field is drawn.
		const bool request = m_options.focused.isBound()
			? (m_options.focused.get() && !m_state.active)
			: (!m_state.seen && m_options.focused.get());
		m_state.seen = true;
		if (request)
			ImGui::SetKeyboardFocusHere();
		if (m_invalid)
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.60f, 0.16f, 0.16f, 0.65f));
	}

	Scope(const Scope&) = delete;
	Scope& operator=(const Scope&) = delete;

	void after()
	{
		if (m_invalid)
		{
			ImGui::PopStyleColor();
			// an outline inside the item rect, so the drift guard never sees it
			ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
				ImGui::GetColorU32(ImVec4(0.90f, 0.25f, 0.25f, 1.0f)), ImGui::GetStyle().FrameRounding);
		}
		const bool active = ImGui::IsItemActive();
		if (active == m_state.active)
			return;
		m_state.active = active;
		if (m_options.focused.isBound())
			m_options.focused.set(active);
		if (active && m_options.onFocus)
			m_options.onFocus();
		else if (!active && m_options.onBlur)
			m_options.onBlur();
	}

private:
	TextFieldOptions& m_options;
	FieldState& m_state;
	bool m_invalid;
};

} // namespace imgui_text_field
