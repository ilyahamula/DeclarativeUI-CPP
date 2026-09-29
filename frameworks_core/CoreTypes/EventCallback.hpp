#pragma once

#include "frameworks_core/CoreTypes/BoundValue.hpp"

#include <functional>
#include <utility>

// A control's event callback, stored once whichever way the caller spelled it.
//
// Every widget offers two spellings of each event -- onChange(value) and
// onChange(value, void* nativeWidget) -- and before this type every widget and
// every wrapper stored both, and every backend chose between them with the same
// `if (cb) ... else if (cbw) ...` at every call site. Here the short spelling is
// adapted into the long one on the way in, so there is one function to carry,
// one to capture and one to call: `callback(value, native)`.
//
// Setting either spelling replaces whatever was set before -- the last call
// wins, as it does for every other fluent modifier.
template <typename... Args>
class EventCallback
{
public:
	using Full = std::function<void(Args..., void*)>;
	using Short = std::function<void(Args...)>;

	EventCallback() = default;

	void set(Short callback)
	{
		if (callback)
			m_fn = [callback = std::move(callback)](Args... args, void*) { callback(args...); };
		else
			m_fn = nullptr;
	}

	void set(Full callback)
	{
		m_fn = std::move(callback);
	}

	explicit operator bool() const { return static_cast<bool>(m_fn); }

	// A no-op when nothing was set, so call sites need no test of their own.
	void operator()(Args... args, void* native) const
	{
		if (m_fn)
			m_fn(args..., native);
	}

private:
	Full m_fn;
};

// What a retained backend's event handler does with a value the user just
// produced: write it through to the caller's variable when the control is
// bound, then report it -- value first, so a handler reading the bound
// variable sees the new one.
//
// Holds only the caller's variable (never the wrapper: wrapper and native
// window teardown order is not fixed, see wx/RefSync.hpp), so it is safe to
// capture in a handler that outlives the wrapper. Unbound, there is nothing to
// write -- the native control keeps the value itself -- and only the callback
// runs.
template <typename T, typename Callback>
class ValueCommit
{
public:
	ValueCommit(T* bound, Callback callback, void* native)
		: m_bound(bound)
		, m_callback(std::move(callback))
		, m_native(native)
	{
	}

	void operator()(const T& value) const
	{
		if (m_bound != nullptr)
			*m_bound = value;
		m_callback(value, m_native);
	}

private:
	T* m_bound;
	Callback m_callback;
	void* m_native;
};

template <typename T, typename Callback>
ValueCommit<T, Callback> commitTo(BoundValue<T>& value, Callback callback, void* native)
{
	return ValueCommit<T, Callback>(value.isBound() ? &value.get() : nullptr, std::move(callback), native);
}
