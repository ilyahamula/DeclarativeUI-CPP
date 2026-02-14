#pragma once

#include "frameworks_core/ControlWrapper.hpp"

template <typename T>
concept FittableLayout = requires(T layout, ControlWrapper* parent)
{
	layout.fitTo(parent);
};
