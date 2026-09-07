#pragma once

#include <memory>
#include <optional>
#include <tuple>

#include "buildable.hpp"

// An N-column form container: cells fill rows left-to-right, top-to-bottom, and
// the engine sizes each column to its widest cell and each row to its tallest.
// It replaces the "one HStack per row plus a SizeGroup per column" idiom -- the
// columns line up because they are one band, not because two stacks were talked
// into agreeing.
//
// A ragged last row is fine: the grid simply has fewer cells than columns * rows.
//
// A cell's Border() is both its padding inside its band and the source of the
// gutter beside it, so the grid is a little more generous with borders than a
// Stack is (docs/specs/widget_catalogue_completion/requirements.md R2.1). With
// no borders anywhere, columns and rows are kDefaultGap apart, exactly like a
// Stack's children.
//
// Alignment inside a cell is the ordinary crossAlign rule applied to both axes:
// leaves default to Start (top-left of their band, at their intrinsic size),
// containers to Stretch, and Expand()/Center*() override per axis.
template <NodeBuildable... W>
struct Grid
{
	Grid(int columns, W... cells)
		: m_columns(columns)
		, m_cells(std::make_tuple(cells...))
	{
	}

	Grid(int columns, LayoutFlags flags, W... cells)
		: m_columns(columns)
		, m_flags(flags)
		, m_cells(std::make_tuple(cells...))
	{
	}

	// Disable the grid and, with it, every cell inside it. Snapshot the flag as
	// it stands now.
	Grid& isDisabled(const bool& disabled = true)
	{
		m_disabled.snapshot(disabled);
		return *this;
	}

	// Bind to a caller-owned flag: flipping it disables/enables the whole
	// subtree without rebuilding the tree. A cell cannot opt back out --
	// disabling always cascades down.
	Grid& isDisabled(bool& disabled)
	{
		m_disabled.bind(disabled);
		return *this;
	}

	std::unique_ptr<LayoutNode> buildNode()
	{
		auto node = makeGrid(m_columns, m_flags.value_or(LayoutFlags{}));
		node->disabled = m_disabled;
		// Declaration order IS the grid order: the node's child index carries
		// both the row (index / columns) and the column (index % columns).
		std::apply([&](auto&... cell) {
			(node->add(cell.buildNode()), ...);
		}, m_cells);
		return node;
	}

private:
	int m_columns;
	std::optional<LayoutFlags> m_flags;
	DisabledFlag m_disabled;
	std::tuple<W...> m_cells;
};
