#pragma once

#include <memory>
#include <optional>
#include <utility>

#include "frameworks_core/LayoutNode.hpp"

// A user-defined struct/class that stands for a group of widgets and is used
// anywhere a built-in widget fits:
//
//   struct PasswordRow
//   {
//       std::string& password;
//       DECLARE_UI(HStack { StaticText{"Password:"}, PasswordInput{password} })
//   };
//
//   VStack { PasswordRow{pw}.withFlags(LayoutFlags().Expand()).isHidden(hide) }
//
// Nothing new is needed for such a type to work: every container takes a
// NodeBuildable, i.e. anything with `std::unique_ptr<LayoutNode> buildNode()`.
// DECLARE_UI(...) only writes that function, so the class body reads as its
// sub-widget tree, and adds the modifiers a container has -- withFlags(),
// isDisabled(), isHidden() -- returning the class's own type so chaining keeps
// it. Writing buildNode() by hand is equivalent and stays the spelling for a
// body that needs logic before its return.
//
// Rules of use:
//   * The argument is variadic on purpose: the preprocessor splits macro
//     arguments on commas inside `{}`, so `HStack{a, b}` is several arguments.
//   * The macro switches to `public:` -- the modifiers must be callable, and an
//     aggregate's members must be public -- so declarations after it are
//     public. In an aggregate put it AFTER the data members, or `PasswordRow{pw}`
//     would try to initialise the macro's member first. A class with a
//     constructor can place it anywhere.
//   * The type is a TEMPORARY like every widget: it dies with the enclosing
//     expression, long before a wx/Qt handler fires. A handler must never
//     capture `this` -- capture the caller-owned variables themselves
//     (`[&status = m_status]`) or values.
//   * The modifiers land on the root node the body returns. withFlags()
//     replaces the root's flags, as Widget::withFlags() replaces defaults. A root
//     that already carries its own isDisabled()/isHidden() is not overwritten:
//     it is wrapped in a one-child box carrying the outer flag, so both apply.

// The state DECLARE_UI adds: one member, so an aggregate gains exactly one
// trailing, default-initialised field.
struct DeclaredUiModifiers
{
	std::optional<LayoutFlags> flags;
	std::optional<DisabledFlag> disabled;
	std::optional<BoundValue<bool>> hidden;

	std::unique_ptr<LayoutNode> apply(std::unique_ptr<LayoutNode> root) const
	{
		if (flags)
			root->flags = *flags;

		// A flag the root already uses (bound, or a snapshot `true`) would be lost
		// by assignment, so the outer one goes on a wrapper instead. The wrapper
		// takes over the root's flags (margins, proportion, alignment in the
		// parent) and the root fills it; a one-child box opens no gap.
		auto inUse = [](const BoundValue<bool>& flag) { return flag.isBound() || flag.get(); };
		if ((disabled && inUse(root->disabled)) || (hidden && inUse(root->hidden)))
		{
			auto wrapper = makeBox(Orientation::Vertical, root->flags);
			root->flags = LayoutFlags().Expand();
			wrapper->add(std::move(root));
			root = std::move(wrapper);
		}

		if (disabled)
			root->disabled = *disabled;
		if (hidden)
			root->hidden = *hidden;
		return root;
	}
};

#define DECLARE_UI(...)                                                         \
public:                                                                         \
	DeclaredUiModifiers m_declaredUiModifiers {};                               \
	auto& withFlags(LayoutFlags flags)                                          \
	{                                                                           \
		m_declaredUiModifiers.flags = flags;                                    \
		return *this;                                                           \
	}                                                                           \
	auto& isDisabled(const bool& disabled = true)                               \
	{                                                                           \
		m_declaredUiModifiers.disabled.emplace(disabled);                       \
		return *this;                                                           \
	}                                                                           \
	auto& isDisabled(bool& disabled)                                            \
	{                                                                           \
		m_declaredUiModifiers.disabled.emplace(disabled);                       \
		return *this;                                                           \
	}                                                                           \
	auto& isHidden(const bool& hidden = true)                                   \
	{                                                                           \
		m_declaredUiModifiers.hidden.emplace(hidden);                           \
		return *this;                                                           \
	}                                                                           \
	auto& isHidden(bool& hidden)                                                \
	{                                                                           \
		m_declaredUiModifiers.hidden.emplace(hidden);                           \
		return *this;                                                           \
	}                                                                           \
	std::unique_ptr<LayoutNode> buildNode()                                     \
	{                                                                           \
		return m_declaredUiModifiers.apply((__VA_ARGS__).buildNode());          \
	}
