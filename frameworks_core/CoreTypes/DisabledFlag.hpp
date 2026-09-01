#pragma once

#include <functional>
#include <optional>

// A disabled state that is either a snapshot taken when the tree was built or
// a live binding to a caller-owned bool. Held by ControlWrapper (one widget)
// and by LayoutNode (a container, inherited by its whole subtree).
//
// A bound flag belongs to the caller and outlives the tree, the wrappers and
// the native windows -- that is what lets a retained backend poll it long
// after the control was created. A snapshot may only ever be taken from a
// value the framework owns, because the declarative elements are temporaries
// that die with the enclosing expression.
class DisabledFlag
{
public:
	void set(bool disabled)
	{
		m_value = disabled;
		m_ref.reset();
	}

	void bind(bool& disabled)
	{
		m_value = disabled;
		m_ref = disabled;
	}

	bool value() const
	{
		return m_ref ? m_ref->get() : m_value;
	}

	// Non-null only while bound: the caller-owned flag to poll. Snapshots
	// report nullptr -- there is nothing to watch, they cannot change.
	const bool* boundFlag() const
	{
		return m_ref ? &m_ref->get() : nullptr;
	}

private:
	bool m_value = false;
	std::optional<std::reference_wrapper<bool>> m_ref;
};
