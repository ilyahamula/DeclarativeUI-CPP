#pragma once

#include <tuple>

#include "frameworks_core/ControlWrapper.hpp"

template <typename T>
concept CreateAndAddable = requires(T widget, ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
	widget.createAndAdd(parent, layout, flags);
};

template <CreateAndAddable... W>
void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags, std::tuple<W...> widgets)
{
	std::apply([parent, layout, flags](auto&&... widget) {
		(widget.createAndAdd(parent, layout, flags), ...);
	}, widgets);
}

template <CreateAndAddable... W>
void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags, W... widgets)
{
	createAndAdd(parent, layout, flags, std::make_tuple(widgets...));
}