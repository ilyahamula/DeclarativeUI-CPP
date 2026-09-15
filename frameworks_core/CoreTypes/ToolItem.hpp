#pragma once

#include "frameworks_core/CoreTypes/BoundValue.hpp"

#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// One tool in a ToolBar: framework-independent data every backend walks, like
// MenuItem and TreeItem. wx turns it into a wxToolBar tool, Qt into a QAction on
// a QToolBar, and ImGui draws it as a button -- all three from these fields.
//
// Real constructors rather than aggregate initialisation because BoundValue's
// constructors are explicit: brace-initialising a member would never pick the
// binding overload, which is the whole point of toggled(bool&) vs
// toggled(const bool&).
struct ToolItem
{
	explicit ToolItem(std::string toolLabel)
		: label(std::move(toolLabel))
	{
	}

	// A divider between groups of tools. It carries no label and no handler, so
	// it is the one ToolItem that is not built from a label.
	static ToolItem Separator()
	{
		ToolItem item{ std::string() };
		item.isSeparator = true;
		return item;
	}

	// Path to an image file. A path that fails to load is not fatal: the tool
	// falls back to showing its label and the failure is logged (R9.3).
	ToolItem& withIcon(std::string path)
	{
		iconPath = std::move(path);
		return *this;
	}

	ToolItem& onClick(std::function<void()> callback)
	{
		clickHandler = std::move(callback);
		return *this;
	}

	// Makes this a CHECK tool: it stays down while the value is true. The
	// framework owns this one...
	ToolItem& toggled(const bool& value)
	{
		toggledFlag = BoundValue<bool>(value);
		return *this;
	}

	// ...and this one IS the caller's bool, so pressing the tool writes through
	// and anything else writing it moves the tool.
	ToolItem& toggled(bool& value)
	{
		toggledFlag = BoundValue<bool>(value);
		return *this;
	}

	ToolItem& isDisabled(const bool& value)
	{
		disabledFlag = DisabledFlag(value);
		return *this;
	}

	ToolItem& isDisabled(bool& value)
	{
		disabledFlag = DisabledFlag(value);
		return *this;
	}

	ToolItem& withTooltip(std::string text)
	{
		tooltip = std::move(text);
		return *this;
	}

	std::string label;
	std::string iconPath;
	std::string tooltip;
	std::function<void()> clickHandler;
	std::optional<BoundValue<bool>> toggledFlag; // engaged => a check tool
	DisabledFlag disabledFlag { false };
	bool isSeparator = false;
};
