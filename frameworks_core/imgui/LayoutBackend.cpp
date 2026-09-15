#include "frameworks_core/imgui/LayoutBackend.hpp"

#include "frameworks_core/ControlWrapper.hpp"
#include "frameworks_core/LayoutNode.hpp"
#include "frameworks_core/imgui/ImGuiWidgetIdManager.hpp"
#include "frameworks_core/imgui/MenuDraw.hpp"

#include "imgui.h"

#include <cmath>
#include <cstdio>
#include <string>

namespace
{

// Engine frames are absolute; ImGui cursor coordinates are relative to the
// current window's content origin (which is the child's own, inside a
// BeginChild). `scopeX/scopeY` is the absolute origin of that window -- 0 for
// the dialog, the panel's origin inside a ScrollPanel.
ImVec2 toWindowPos(const Rect& frame, int scopeX, int scopeY)
{
	const ImVec2 origin = ImGui::GetCursorStartPos();
	return ImVec2(origin.x + (float)(frame.x - scopeX), origin.y + (float)(frame.y - scopeY));
}

// Screen-space point for draw-list chrome.
ImVec2 toScreenPos(const Rect& frame, int scopeX, int scopeY)
{
	const ImVec2 window = ImGui::GetWindowPos();
	const ImVec2 local = toWindowPos(frame, scopeX, scopeY);
	return ImVec2(window.x + local.x - ImGui::GetScrollX(),
		window.y + local.y - ImGui::GetScrollY());
}

int ceilInt(float v)
{
	return (int)std::ceil(v);
}

// The content half of an Expander: the child that folds away, as opposed to
// the header leaf beside it.
bool isExpanderContent(const LayoutNode& node)
{
	return node.parent != nullptr && node.parent->kind == NodeKind::Expander;
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

	// own flag OR'd with every ancestor container's — BeginDisabled nests the
	// same way, so a leaf inside a disabled group box stays disabled either way
	const bool disabled = leaf.isDisabledEffective();

	ImGui::SetCursorPos(toWindowPos(frame, currentScope().originX, currentScope().originY));
	// group the render so composite widgets (DatePicker = 3 items) read
	// back as one item rect
	ImGui::BeginGroup();
	ImGui::BeginDisabled(disabled);
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

	// EndGroup emits the whole render as one item, so a composite widget hovers
	// as a single rect — but it emits it OUTSIDE the BeginDisabled scope, so a
	// disabled control would still answer IsItemHovered(). Excluded by hand to
	// match wx and Qt, neither of which delivers tooltip events to a disabled
	// window. Runs after the drift guard so SetTooltip's own window can never
	// disturb the item rect the guard reads.
	const std::string& tooltip = leaf.widget->tooltip();
	if (!disabled && !tooltip.empty())
		ImGui::SetItemTooltip("%s", tooltip.c_str());

	// The right-click menu, on the same terms and for the same reasons: the
	// group is the item the popup hangs off, so a composite widget opens one
	// menu over the whole of itself, and a disabled leaf opens none -- wx and Qt
	// deliver no context-menu event to a disabled window either.
	//
	// The popup needs an id of its OWN, and an explicit one: EndGroup adds the
	// group with id 0, so letting BeginPopupContextItem fall back to the last
	// item's id (its nullptr default) would trip its own assert. The id must
	// also survive from frame to frame -- a popup is reopened by id every frame
	// it stays up -- which rules out anything derived from the node, since the
	// tree is rebuilt. Hence the dedicated counter, pushed so that every leaf's
	// "##ctx" hashes differently.
	const ContextMenu& menu = leaf.widget->contextMenu();
	if (!disabled && !menu.empty())
	{
		const int contextId = WidgetIdManager::nextContextMenuId();
		ImGui::PushID(contextId);
		if (ImGui::BeginPopupContextItem("##ctx"))
		{
			// The path keys an unbound check item's state, which on ImGui lives
			// outside the frame; the counter is what makes it unique per leaf.
			drawMenuItems(menu, "##ctx" + std::to_string(contextId));
			ImGui::EndPopup();
		}
		ImGui::PopID();
	}
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
	case NodeKind::ScrollPanel:
	{
		// The scrollbar gutter, reserved on the cross side of each scrolling
		// axis and reserved ALWAYS -- containerInsets runs before the content
		// is measured, so "only when it overflows" could only ever read the
		// previous pass and would let a panel near the boundary oscillate.
		// ImGui reserves the same gutter wx and Qt do, rather than letting its
		// scrollbar overlay the content: one tree, one set of content frames.
		const int bar = ceilInt(style.ScrollbarSize);
		return EdgeInsets {
			0,
			scrollsVertically(node.scroll) ? bar : 0,
			0,
			scrollsHorizontally(node.scroll) ? bar : 0,
		};
	}
	default:
		return EdgeInsets{};
	}
}

bool ImGuiLayoutBackend::beginContainer(const LayoutNode& node, const Rect& frame)
{
	// A disabled container greys its own chrome and everything drawn inside
	// it: BeginDisabled nests by OR, so no child can opt back in. The scope is
	// popped in endContainer, and on every early-out below.
	const bool disabled = node.isDisabledEffective();

	// A collapsed section draws nothing at all -- no chrome, no disabled scope,
	// no child window -- so it reports invisible before any of that is opened,
	// exactly as an inactive tab page does. Nothing else is needed on ImGui:
	// there are no retained windows here to take back out of view.
	if (isExpanderContent(node) && !node.parent->expander.applied)
		return false;

	// a container whose open parent is a TabPanel is a tab page
	const bool isTabPage = !m_containerStack.empty()
		&& m_containerStack.back()->kind == NodeKind::TabPanel;
	if (isTabPage)
	{
		const char* label = node.label.empty() ? "##tab" : node.label.c_str();
		if (!ImGui::BeginTabItem(label))
			return false; // inactive page: engine skips the subtree

		// The scope opens AFTER the tab item, so a disabled page greys its
		// content but keeps its label selectable -- wxNotebook has no per-tab
		// enable, and greying only here would make ImGui the odd one out.
		// Disable the whole TabPanel to lock tab switching on all three.
		if (disabled)
			ImGui::BeginDisabled();
		m_containerStack.push_back(&node);
		m_disabledStack.push_back(disabled);
		return true;
	}

	if (disabled)
		ImGui::BeginDisabled();

	switch (node.kind)
	{
	case NodeKind::GroupBox:
	{
		// chrome only — border + title via the draw list, no child window,
		// so every frame stays in one coordinate space
		const ImVec2 min = toScreenPos(frame, currentScope().originX, currentScope().originY);
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
		ImGui::SetCursorPos(toWindowPos(frame, currentScope().originX, currentScope().originY));
		ImGui::PushID(WidgetIdManager::nextWidgetId());
		if (!ImGui::BeginTabBar("##tabs"))
		{
			ImGui::PopID();
			if (disabled)
				ImGui::EndDisabled();
			return false;
		}
		break;
	}
	case NodeKind::ScrollPanel:
	{
		ImGui::SetCursorPos(toWindowPos(frame, currentScope().originX, currentScope().originY));
		ImGui::PushID(WidgetIdManager::nextWidgetId());

		// Bars are AlwaysX to match the gutter containerInsets always reserves;
		// a bar that came and went would move the content under the user.
		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoSavedSettings;
		if (scrollsVertically(node.scroll))
			windowFlags |= ImGuiWindowFlags_AlwaysVerticalScrollbar;
		if (scrollsHorizontally(node.scroll))
			windowFlags |= ImGuiWindowFlags_HorizontalScrollbar
				| ImGuiWindowFlags_AlwaysHorizontalScrollbar;

		// Whatever BeginChild returns, EndChild must run -- and the engine only
		// calls endContainer when beginContainer returned true, so this branch
		// never reports invisible the way an inactive tab page does.
		ImGui::BeginChild("##scroll", ImVec2((float)frame.width, (float)frame.height),
			ImGuiChildFlags_None, windowFlags);

		// Declare the virtual extent up front so the child has something to
		// scroll over; the content itself is drawn at absolute cursor positions
		// on top of it, so this Dummy only ever sets the content size.
		if (!node.children.empty())
		{
			const Rect& content = node.children.front()->frame;
			ImGui::Dummy(ImVec2((float)content.width, (float)content.height));
		}

		// From here on the subtree is drawn in the panel's own space.
		m_originStack.push_back(Scope { &node, frame.x, frame.y });
		break;
	}
	default:
		break; // plain Box: no chrome
	}

	m_containerStack.push_back(&node);
	m_disabledStack.push_back(disabled);
	return true;
}

void ImGuiLayoutBackend::endContainer(const LayoutNode& node)
{
	bool disabled = false;
	if (!m_containerStack.empty() && m_containerStack.back() == &node)
	{
		m_containerStack.pop_back();
		disabled = m_disabledStack.back();
		m_disabledStack.pop_back();
	}

	const bool isTabPage = !m_containerStack.empty()
		&& m_containerStack.back()->kind == NodeKind::TabPanel;

	// A child window closes BEFORE the BeginDisabled that wrapped it: ImGui
	// records the disabled-stack depth per window and asserts in EndChild if it
	// has moved since BeginChild.
	if (node.kind == NodeKind::ScrollPanel)
	{
		ImGui::EndChild();
		ImGui::PopID();
		if (m_originStack.size() > 1 && m_originStack.back().node == &node)
			m_originStack.pop_back();
	}

	// closed before EndTabItem (it opened after BeginTabItem) but after the
	// other chrome, which it greyed
	if (disabled)
		ImGui::EndDisabled();

	if (isTabPage)
	{
		ImGui::EndTabItem();
	}
	else if (node.kind == NodeKind::TabPanel)
	{
		ImGui::EndTabBar();
		ImGui::PopID();
	}
}
