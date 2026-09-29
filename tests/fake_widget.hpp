#pragma once

#include "frameworks_core/ControlWrapper.hpp"

#include <map>
#include <memory>

namespace testfw
{

// A real, default-constructed ControlWrapper standing in for a leaf's widget,
// one per tag address. The engine does read through a leaf's widget pointer
// (LayoutNode::isDisabledEffective() asks it isDisabled()), so the pointer has
// to be a genuine object: an int reinterpret_cast to ControlWrapper* happened
// to read zeros under clang and crashed under MSVC. Nothing ever mutates these,
// so a stack address reused by a later test safely gets the same wrapper back.
inline ControlWrapper* fakeWidget(const void* tag)
{
	static std::map<const void*, std::unique_ptr<ControlWrapper>> pool;
	auto& slot = pool[tag];
	if (!slot)
		slot = std::make_unique<ControlWrapper>();
	return slot.get();
}

} // namespace testfw
