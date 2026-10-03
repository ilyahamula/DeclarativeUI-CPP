#pragma once

#include "imgui.h"

#include <algorithm>
#include <string>

// Caller text in an ImGui widget label, shown in full.
//
// ImGui reads "##" in a label as "the rest is only an id" and draws nothing
// from it on -- Button("C## code") shows "C" -- and has no escape for it. So a
// label that contains "##" goes to the widget as a hidden id ("##label") and
// the text is drawn by these helpers instead, exactly where ImGui would have
// drawn it; a label without "##" takes ImGui's own path, unchanged. Sizes need
// no change: every measure here uses CalcTextSize, which does not stop at
// "##" unless asked to, so the frames were always the full text's.
//
// Covered: buttons, check boxes, radio buttons and Selectable rows (lists,
// combos, tables, the file browser). Not covered, because the label IS the
// widget's identity there: window and dialog titles, tab labels, menu labels
// and table column headers -- documented in CLAUDE.md.
namespace imgui_labels
{

inline bool hidesText(const std::string& label)
{
	return label.find("##") != std::string::npos;
}

// The text in [min, max], placed by `align` (0..1 per axis) the way
// RenderTextClipped places a label, clipped to the box.
inline void drawText(const ImVec2& min, const ImVec2& max, const std::string& text, const ImVec2& align)
{
	const ImVec2 size = ImGui::CalcTextSize(text.c_str());
	const ImVec2 pos(min.x + std::max(0.0f, (max.x - min.x - size.x) * align.x),
		min.y + std::max(0.0f, (max.y - min.y - size.y) * align.y));
	ImDrawList* draw = ImGui::GetWindowDrawList();
	draw->PushClipRect(min, max, true);
	draw->AddText(pos, ImGui::GetColorU32(ImGuiCol_Text), text.c_str());
	draw->PopClipRect();
}

// ImGui::Button. `size` follows Button's own rules (0 = fit the text).
inline bool button(const std::string& label, ImVec2 size = ImVec2(0, 0))
{
	if (!hidesText(label))
		return ImGui::Button(label.empty() ? "##button" : label.c_str(), size);
	const ImGuiStyle& style = ImGui::GetStyle();
	const ImVec2 text = ImGui::CalcTextSize(label.c_str());
	if (size.x <= 0.0f)
		size.x = text.x + style.FramePadding.x * 2.0f;
	if (size.y <= 0.0f)
		size.y = ImGui::GetFrameHeight();
	const bool pressed = ImGui::Button("##button", size);
	const ImVec2 pad = style.FramePadding;
	drawText(ImVec2(ImGui::GetItemRectMin().x + pad.x, ImGui::GetItemRectMin().y + pad.y),
		ImVec2(ImGui::GetItemRectMax().x - pad.x, ImGui::GetItemRectMax().y - pad.y),
		label, style.ButtonTextAlign);
	return pressed;
}

// ImGui::Selectable. The row's text starts where the cursor was, as ImGui
// draws it, and is clipped to the row.
inline bool selectable(const std::string& label, bool selected, ImGuiSelectableFlags flags = 0,
	const ImVec2& size = ImVec2(0, 0))
{
	if (!hidesText(label))
		return ImGui::Selectable(label.c_str(), selected, flags, size);
	const ImVec2 origin = ImGui::GetCursorScreenPos();
	const bool clicked = ImGui::Selectable("##row", selected, flags, size);
	drawText(origin, ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMax().y), label,
		ImGui::GetStyle().SelectableTextAlign);
	return clicked;
}

// The text beside a check box or radio button, clickable like ImGui's own
// label: true when it was clicked.
inline bool clickableLabel(const std::string& label)
{
	if (label.empty())
		return false;
	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::TextUnformatted(label.c_str());
	return ImGui::IsItemClicked();
}

inline bool checkbox(const std::string& label, bool* value)
{
	if (!hidesText(label))
		return ImGui::Checkbox(label.empty() ? "##checkbox" : label.c_str(), value);
	bool changed = ImGui::Checkbox("##checkbox", value);
	if (clickableLabel(label))
	{
		*value = !*value;
		changed = true;
	}
	return changed;
}

inline bool radioButton(const std::string& label, bool active)
{
	if (!hidesText(label))
		return ImGui::RadioButton(label.empty() ? "##radio" : label.c_str(), active);
	const bool pressed = ImGui::RadioButton("##radio", active);
	return clickableLabel(label) || pressed;
}

} // namespace imgui_labels
