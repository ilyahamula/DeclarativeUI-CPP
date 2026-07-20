#pragma once

#include "frameworks_core/ILayoutBackend.hpp"
#include "frameworks_core/LayoutNode.hpp"

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

// Scripted ILayoutBackend for engine tests: leaf sizes are set per widget
// pointer (fixed, or as a function of the offered Constraints for
// width-dependent-height cases like wrapping text). Records every measure,
// place, and container call so tests can assert both geometry and traversal.
// Also the basis of the Phase 3 golden cross-backend test (T3.5), where it is
// parameterized with wx/Qt/ImGui-shaped intrinsic sizes.
class MockLayoutBackend : public ILayoutBackend
{
public:
	using MeasureFn = std::function<Size(const Constraints&)>;

	void setSize(const ControlWrapper* widget, Size size)
	{
		m_sizes[widget] = size;
	}

	void setMeasure(const ControlWrapper* widget, MeasureFn fn)
	{
		m_measureFns[widget] = std::move(fn);
	}

	Size measure(const LayoutNode& leaf, const Constraints& c) override
	{
		measureCalls.push_back({ leaf.widget, c });
		if (auto fn = m_measureFns.find(leaf.widget); fn != m_measureFns.end())
			return fn->second(c);
		if (auto it = m_sizes.find(leaf.widget); it != m_sizes.end())
			return it->second;
		return defaultSize;
	}

	void place(const LayoutNode& leaf, const Rect& frame) override
	{
		placed.push_back({ leaf.widget, frame });
		callLog.push_back("place");
	}

	EdgeInsets containerInsets(const LayoutNode& node) override
	{
		return chromeFn ? chromeFn(node) : EdgeInsets{};
	}

	bool beginContainer(const LayoutNode& node, const Rect& frame) override
	{
		containerFrames[&node] = frame;
		if (hiddenContainers.count(&node))
		{
			callLog.push_back("skip:" + node.label);
			return false;
		}
		callLog.push_back("begin:" + node.label);
		return true;
	}

	void endContainer(const LayoutNode& node) override
	{
		callLog.push_back("end:" + node.label);
	}

	// Frame recorded for a given widget, or a zero Rect if it was never placed.
	Rect frameOf(const ControlWrapper* widget) const
	{
		for (const auto& p : placed)
			if (p.widget == widget)
				return p.frame;
		return Rect{};
	}

	struct MeasureCall
	{
		ControlWrapper* widget;
		Constraints constraints;
	};
	struct Placement
	{
		ControlWrapper* widget;
		Rect frame;
	};

	Size defaultSize { 0, 0 };
	std::function<EdgeInsets(const LayoutNode&)> chromeFn; // container chrome, default none
	std::set<const LayoutNode*> hiddenContainers; // beginContainer returns false for these
	std::vector<MeasureCall> measureCalls;
	std::vector<Placement> placed;
	std::map<const LayoutNode*, Rect> containerFrames;
	std::vector<std::string> callLog;

private:
	std::map<const ControlWrapper*, Size> m_sizes;
	std::map<const ControlWrapper*, MeasureFn> m_measureFns;
};
