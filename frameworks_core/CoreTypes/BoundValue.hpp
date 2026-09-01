#pragma once

#include <functional>
#include <optional>
#include <string>

// A control's value: either a snapshot the framework owns or a live binding to
// a caller-owned variable. Held by the declarative widget (include/widgets.hpp)
// and, copied from it, by the wrapper that outlives it.
//
// The two are not interchangeable and the distinction is made exactly once, by
// overload resolution on the widget's constructor:
//
//   Slider(range, 50)       -> snapshot -- the source is a temporary
//   Slider(range, myValue)  -> bind     -- edits write through to myValue
//
// A bound value belongs to the caller and outlives the tree, the wrappers and
// the native windows, which is what lets a control write back to it at any
// time. A snapshot may only ever be taken from a value we then own, because the
// declarative elements are temporaries that die with the enclosing expression --
// keeping a reference into one is a dangling reference by the time the backend
// realizes or renders the control. Copying a BoundValue preserves both cases:
// the copy shares the caller's variable, or carries its own snapshot.
template <typename T>
class BoundValue
{
public:
	BoundValue() = default;

	// Explicit so a bare T can never silently pick the binding overload of a
	// constructor that takes a BoundValue.
	explicit BoundValue(const T& snapshotValue)
		: m_value(snapshotValue)
	{
	}

	explicit BoundValue(T& external)
		: m_ref(external)
	{
	}

	void snapshot(const T& value)
	{
		m_value = value;
		m_ref.reset();
	}

	void bind(T& value)
	{
		m_ref = value;
	}

	bool isBound() const { return m_ref.has_value(); }

	// The live value: the caller's variable while bound, our snapshot otherwise.
	// Writing through it is how a control commits an edit -- unbound, the write
	// lands in the snapshot and only the onChange callback observes it.
	//
	// A retained backend may capture this by reference into a native event handler
	// ONLY under an isBound() check: bound, it is the caller's variable and outlives
	// every window; unbound, it points into the wrapper, whose teardown order
	// against the native window is not fixed (see wx/RefSync.hpp).
	T& get() { return m_ref ? m_ref->get() : m_value; }
	const T& get() const { return m_ref ? m_ref->get() : m_value; }

	void set(const T& value) { get() = value; }

	// Non-null only while bound: the caller-owned variable to watch. A snapshot
	// reports nullptr -- there is nothing to poll, it cannot change behind us.
	const T* boundValue() const { return m_ref ? &m_ref->get() : nullptr; }

private:
	T m_value{};
	std::optional<std::reference_wrapper<T>> m_ref;
};

// A control's disabled state: a snapshot taken when the tree was built, or a
// live binding to a caller-owned bool. Held by ControlWrapper (one widget) and
// by LayoutNode (a container, inherited by its whole subtree) -- which is why a
// bound flag has to be pollable: a retained backend mirrors it into the native
// window long after the control was created.
using DisabledFlag = BoundValue<bool>;

// A control's hover text, on the same terms: a snapshot taken when the tree was
// built, or a live binding to a caller-owned string. Held by ControlWrapper --
// tooltips decorate leaves only, so no container carries one.
using TooltipText = BoundValue<std::string>;
