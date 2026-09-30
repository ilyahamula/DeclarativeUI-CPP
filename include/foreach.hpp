#pragma once

#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

#include "buildable.hpp"
#include "frameworks_core/NodeSource.hpp"

// A stack whose rows come from a vector:
//
//   VForEach { todos, [&](const Todo& todo, std::size_t i) {
//       return HStack {
//           CheckBox{ todo.done, todo.title }
//               .onChange([&todos, i](bool on) { todos[i].done = on; }),
//           Button{"Remove"}.onClick([&todos, i] { todos.erase(todos.begin() + i); })
//       };
//   } }
//
// A row is a FUNCTION OF ITS ITEM: `row(item)` or `row(item, index)`, called
// with an item of the ForEach's own copy of the data, so a row never holds a
// reference into the caller's vector -- which moves whenever it grows. A row
// changes the data through callbacks, as above, and the rows follow.
//
// Pass the vector as a non-const lvalue and the ForEach BINDS it: when it
// changes, the rows follow -- every row when the count changed, only the rows
// whose item changed otherwise (Item must be ==-comparable to tell). On ImGui
// that is free, the tree being rebuilt every frame; on wx and Qt the stale
// rows' native windows are destroyed and fresh ones built, and an auto-fit
// window re-measures. Pass it const, or as a temporary, and it is a snapshot.
//
// Two consequences worth knowing:
//   * a row rebuilt on wx/Qt is a NEW control -- a text field that writes its
//     own item back on every keystroke is rebuilt as you type and loses focus.
//     Keep such a field's text outside the vector, or commit it on a button.
//   * on ImGui an unbound value in a row is kept by the row's POSITION, so
//     inserting above it shifts it; name the row's controls with withId(key)
//     where rows carry unbound state.
//
// Otherwise it is a VStack / HStack: flags, gaps, cross alignment, isDisabled()
// and isHidden() all behave the same.
template <typename Item, typename Row>
struct ForEach
{
	static_assert(std::copyable<Item>, "ForEach keeps its own copy of the items");

	ForEach(Orientation orient, const std::vector<Item>& items, Row row)
		: m_orient(orient)
		, m_items(items)
		, m_row(std::move(row))
	{
	}

	ForEach(Orientation orient, std::vector<Item>& items, Row row)
		: m_orient(orient)
		, m_bound(&items)
		, m_items(items)
		, m_row(std::move(row))
	{
		static_assert(std::equality_comparable<Item>,
			"a bound ForEach compares items to tell which rows changed; give Item an operator==");
	}

	ForEach(Orientation orient, LayoutFlags flags, const std::vector<Item>& items, Row row)
		: ForEach(orient, items, std::move(row))
	{
		m_flags = flags;
	}

	ForEach(Orientation orient, LayoutFlags flags, std::vector<Item>& items, Row row)
		: ForEach(orient, items, std::move(row))
	{
		m_flags = flags;
	}

	ForEach& isDisabled(const bool& disabled = true)
	{
		m_disabled.snapshot(disabled);
		return *this;
	}

	ForEach& isDisabled(bool& disabled)
	{
		m_disabled.bind(disabled);
		return *this;
	}

	ForEach& isHidden(const bool& hidden = true)
	{
		m_hidden.snapshot(hidden);
		return *this;
	}

	ForEach& isHidden(bool& hidden)
	{
		m_hidden.bind(hidden);
		return *this;
	}

	std::unique_ptr<LayoutNode> buildNode()
	{
		auto node = makeBox(m_orient, m_flags.value_or(LayoutFlags{}));
		node->disabled = m_disabled;
		node->hidden = m_hidden;
		auto source = std::make_shared<Source>(m_bound, m_bound != nullptr ? *m_bound : m_items, m_row);
		for (std::size_t i = 0; i < source->count(); ++i)
			node->add(source->buildRow(i));
		node->source = std::move(source);
		return node;
	}

private:
	// The NodeSource the retained sessions poll. Holds the caller's vector only
	// by pointer, and only when bound -- the caller owns it and it outlives the
	// window, like every bound value -- and builds rows from its own copy.
	class Source : public NodeSource
	{
	public:
		Source(const std::vector<Item>* bound, std::vector<Item> items, Row row)
			: m_bound(bound)
			, m_items(std::move(items))
			, m_row(std::move(row))
		{
		}

		SourceUpdate update() override
		{
			SourceUpdate update;
			if (m_bound == nullptr)
				return update;
			if constexpr (std::equality_comparable<Item>)
			{
				if (m_bound->size() != m_items.size())
					update.all = true;
				else
				{
					for (std::size_t i = 0; i < m_items.size(); ++i)
					{
						if (!((*m_bound)[i] == m_items[i]))
							update.rows.push_back(i);
					}
				}
				if (!update.empty())
					m_items = *m_bound;
			}
			return update;
		}

		std::size_t count() const override { return m_items.size(); }

		std::unique_ptr<LayoutNode> buildRow(std::size_t index) override
		{
			const Item& item = m_items[index];
			if constexpr (std::is_invocable_v<Row&, const Item&, std::size_t>)
				return m_row(item, index).buildNode();
			else
				return m_row(item).buildNode();
		}

	private:
		const std::vector<Item>* m_bound;
		std::vector<Item> m_items;
		Row m_row;
	};

	Orientation m_orient;
	std::optional<LayoutFlags> m_flags;
	const std::vector<Item>* m_bound = nullptr;
	std::vector<Item> m_items;
	Row m_row;
	DisabledFlag m_disabled;
	BoundValue<bool> m_hidden { false };
};

// Rows stacked top to bottom.
template <typename Item, typename Row>
struct VForEach : public ForEach<Item, Row>
{
	VForEach(const std::vector<Item>& items, Row row)
		: ForEach<Item, Row>(Orientation::Vertical, items, std::move(row)) {}
	VForEach(std::vector<Item>& items, Row row)
		: ForEach<Item, Row>(Orientation::Vertical, items, std::move(row)) {}
	VForEach(LayoutFlags flags, const std::vector<Item>& items, Row row)
		: ForEach<Item, Row>(Orientation::Vertical, flags, items, std::move(row)) {}
	VForEach(LayoutFlags flags, std::vector<Item>& items, Row row)
		: ForEach<Item, Row>(Orientation::Vertical, flags, items, std::move(row)) {}

	// re-declared so chaining keeps the VForEach type
	VForEach& isDisabled(const bool& disabled = true) { ForEach<Item, Row>::isDisabled(disabled); return *this; }
	VForEach& isDisabled(bool& disabled) { ForEach<Item, Row>::isDisabled(disabled); return *this; }
	VForEach& isHidden(const bool& hidden = true) { ForEach<Item, Row>::isHidden(hidden); return *this; }
	VForEach& isHidden(bool& hidden) { ForEach<Item, Row>::isHidden(hidden); return *this; }
};

// Rows side by side, left to right.
template <typename Item, typename Row>
struct HForEach : public ForEach<Item, Row>
{
	HForEach(const std::vector<Item>& items, Row row)
		: ForEach<Item, Row>(Orientation::Horizontal, items, std::move(row)) {}
	HForEach(std::vector<Item>& items, Row row)
		: ForEach<Item, Row>(Orientation::Horizontal, items, std::move(row)) {}
	HForEach(LayoutFlags flags, const std::vector<Item>& items, Row row)
		: ForEach<Item, Row>(Orientation::Horizontal, flags, items, std::move(row)) {}
	HForEach(LayoutFlags flags, std::vector<Item>& items, Row row)
		: ForEach<Item, Row>(Orientation::Horizontal, flags, items, std::move(row)) {}

	HForEach& isDisabled(const bool& disabled = true) { ForEach<Item, Row>::isDisabled(disabled); return *this; }
	HForEach& isDisabled(bool& disabled) { ForEach<Item, Row>::isDisabled(disabled); return *this; }
	HForEach& isHidden(const bool& hidden = true) { ForEach<Item, Row>::isHidden(hidden); return *this; }
	HForEach& isHidden(bool& hidden) { ForEach<Item, Row>::isHidden(hidden); return *this; }
};

template <typename Item, typename Row> VForEach(const std::vector<Item>&, Row) -> VForEach<Item, Row>;
template <typename Item, typename Row> VForEach(std::vector<Item>&, Row) -> VForEach<Item, Row>;
template <typename Item, typename Row> VForEach(LayoutFlags, const std::vector<Item>&, Row) -> VForEach<Item, Row>;
template <typename Item, typename Row> VForEach(LayoutFlags, std::vector<Item>&, Row) -> VForEach<Item, Row>;
template <typename Item, typename Row> HForEach(const std::vector<Item>&, Row) -> HForEach<Item, Row>;
template <typename Item, typename Row> HForEach(std::vector<Item>&, Row) -> HForEach<Item, Row>;
template <typename Item, typename Row> HForEach(LayoutFlags, const std::vector<Item>&, Row) -> HForEach<Item, Row>;
template <typename Item, typename Row> HForEach(LayoutFlags, std::vector<Item>&, Row) -> HForEach<Item, Row>;
