#include "frameworks_core/imgui/LayoutBackend.hpp"

#include "frameworks_core/ControlWrapper.hpp"
#include "frameworks_core/LayoutNode.hpp"
#include "frameworks_core/imgui/ImGuiWidgetIdManager.hpp"

#include "imgui.h"

#include <cmath>
#include <cstdio>

namespace
{

// Engine frames are relative to the window content origin; ImGui cursor
// coordinates are window-relative including title bar and padding.
ImVec2 toWindowPos(const Rect& frame)
{
	const ImVec2 origin = ImGui::GetCursorStartPos();
	return ImVec2(origin.x + (float)frame.x, origin.y + (float)frame.y);
}

// Screen-space point for draw-list chrome.
ImVec2 toScreenPos(const Rect& frame)
{
	const ImVec2 window = ImGui::GetWindowPos();
	const ImVec2 local = toWindowPos(frame);
	return ImVec2(window.x + local.x - ImGui::GetScrollX(),
		window.y + local.y - ImGui::GetScrollY());
}

int ceilInt(float v)
{
	return (int)std::ceil(v);
}

} // unnamed namespace

Size ImGuiLayoutBackend::measure(const LayoutNode& leaf, const Constraints& c)
{
	return leaf.widget != nullptr ? leaf.widget->measureContent(c) : Size { 0, 0 };
}

void ImGuiLayoutBackend::place(const LayoutNode& leaf, const Rect& frame)
{
	if (leaf.widget == nullptr)
		return;

	ImGui::SetCursorPos(toWindowPos(frame));
	// group the render so composite widgets (DatePicker = 3 items) read
	// back as one item rect
	ImGui::BeginGroup();
	ImGui::BeginDisabled(leaf.widget->isDisabled());
	leaf.widget->render(frame);
	ImGui::EndDisabled();
	ImGui::EndGroup();

#ifndef NDEBUG
	// Drift guard (architecture.md): measure() re-implements ImGui's sizing
	// rules and can drift across style changes or ImGui upgrades. Overflowing
	// the engine frame is the drift mode that causes overlaps, so it is
	// reported; rendering smaller than the frame is legitimate slack
	// (alignment, non-stretchable heights).
	const ImVec2 item = ImGui::GetItemRectSize();
	if (item.x > (float)frame.width + 2.0f || item.y > (float)frame.height + 2.0f)
	{
		std::fprintf(stderr,
			"[DeclarativeUI drift] rendered item %.0fx%.0f overflows engine frame %dx%d\n",
			item.x, item.y, frame.width, frame.height);
	}
#endif
}

EdgeInsets ImGuiLayoutBackend::containerInsets(const LayoutNode& node)
{
	const ImGuiStyle& style = ImGui::GetStyle();
	switch (node.kind)
	{
	case NodeKind::GroupBox:
	{
		// 1px border + padding; the title row rides on the top border
		const int pad = ceilInt(style.FramePadding.x) + 1;
		const int top = node.label.empty()
			? pad
			: ceilInt(ImGui::GetTextLineHeight()) + ceilInt(style.FramePadding.y) + 1;
		return EdgeInsets { pad, pad, top, pad };
	}
	case NodeKind::TabPanel:
		// tab bar row above the pages
		return EdgeInsets { 0, 0, ceilInt(ImGui::GetFrameHeight() + style.ItemSpacing.y), 0 };
	default:
		return EdgeInsets{};
	}
}

bool ImGuiLayoutBackend::beginContainer(const LayoutNode& node, const Rect& frame)
{
	// a container whose open parent is a TabPanel is a tab page
	const bool isTabPage = !m_containerStack.empty()
		&& m_containerStack.back()->kind == NodeKind::TabPanel;
	if (isTabPage)
	{
		const char* label = node.label.empty() ? "##tab" : node.label.c_str();
		if (!ImGui::BeginTabItem(label))
			return false; // inactive page: engine skips the subtree
		m_containerStack.push_back(&node);
		return true;
	}

	switch (node.kind)
	{
	case NodeKind::GroupBox:
	{
		// chrome only — border + title via the draw list, no child window,
		// so every frame stays in one coordinate space
		const ImVec2 min = toScreenPos(frame);
		const ImVec2 max = ImVec2(min.x + (float)frame.width, min.y + (float)frame.height);
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddRect(min, max, ImGui::GetColorU32(ImGuiCol_Border));
		if (!node.label.empty())
		{
			const ImGuiStyle& style = ImGui::GetStyle();
			const ImVec2 titlePos(min.x + style.FramePadding.x * 2.0f, min.y + 1.0f);
			drawList->AddText(titlePos, ImGui::GetColorU32(ImGuiCol_Text), node.label.c_str());
		}
		break;
	}
	case NodeKind::TabPanel:
	{
		ImGui::SetCursorPos(toWindowPos(frame));
		ImGui::PushID(WidgetIdManager::nextWidgetId());
		if (!ImGui::BeginTabBar("##tabs"))
		{
			ImGui::PopID();
			return false;
		}
		break;
	}
	default:
		break; // plain Box: no chrome
	}

	m_containerStack.push_back(&node);
	return true;
}

void ImGuiLayoutBackend::endContainer(const LayoutNode& node)
{
	if (!m_containerStack.empty() && m_containerStack.back() == &node)
		m_containerStack.pop_back();

	const bool isTabPage = !m_containerStack.empty()
		&& m_containerStack.back()->kind == NodeKind::TabPanel;
	if (isTabPage)
	{
		ImGui::EndTabItem();
		return;
	}
	if (node.kind == NodeKind::TabPanel)
	{
		ImGui::EndTabBar();
		ImGui::PopID();
	}
}
