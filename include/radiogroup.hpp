#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "buildable.hpp"
#include "widgets.hpp"

// One exclusive choice among labelled options, bound to the index of the
// chosen one:
//
//   RadioGroup{ colour, {"Red", "Green", "Blue"} }                    // a column
//   RadioGroup{ size, {"S", "M", "L"} }.withOrientation(Orientation::Horizontal)
//
// It is a stack of RadioButtons, one per option, each naming its index -- the
// RadioButton model (a group IS the radios bound to one int), with the
// bookkeeping written once. So it is built entirely from existing leaves and
// needs nothing from any backend: it looks and behaves exactly like the radios
// it is made of, on all three.
//
// The index must be BOUND, for the reason a RadioButton<int>'s must be: the
// radios find each other through the caller's int.
struct RadioGroup
{
	RadioGroup(int& selected, std::vector<std::string> options)
		: m_selected(&selected)
		, m_options(std::move(options))
	{
	}

	RadioGroup(LayoutFlags flags, int& selected, std::vector<std::string> options)
		: RadioGroup(selected, std::move(options))
	{
		m_flags = flags;
	}

	// Column (the default) or row.
	RadioGroup& withOrientation(Orientation orient)
	{
		m_orient = orient;
		return *this;
	}

	// Fired with the chosen index, after the bound int has been written.
	RadioGroup& onChange(std::function<void(int)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

	RadioGroup& isDisabled(const bool& disabled = true)
	{
		m_disabled.snapshot(disabled);
		return *this;
	}

	RadioGroup& isDisabled(bool& disabled)
	{
		m_disabled.bind(disabled);
		return *this;
	}

	RadioGroup& isHidden(const bool& hidden = true)
	{
		m_hidden.snapshot(hidden);
		return *this;
	}

	RadioGroup& isHidden(bool& hidden)
	{
		m_hidden.bind(hidden);
		return *this;
	}

	std::unique_ptr<LayoutNode> buildNode()
	{
		auto node = makeBox(m_orient, m_flags.value_or(LayoutFlags{}));
		node->disabled = m_disabled;
		node->hidden = m_hidden;
		for (int i = 0; i < static_cast<int>(m_options.size()); ++i)
		{
			RadioButton<int> radio { *m_selected, i, m_options[static_cast<std::size_t>(i)] };
			if (m_onChange)
				radio.onChange(m_onChange);
			node->add(radio.buildNode());
		}
		return node;
	}

private:
	int* m_selected;
	std::vector<std::string> m_options;
	Orientation m_orient = Orientation::Vertical;
	std::optional<LayoutFlags> m_flags;
	std::function<void(int)> m_onChange;
	DisabledFlag m_disabled;
	BoundValue<bool> m_hidden { false };
};
