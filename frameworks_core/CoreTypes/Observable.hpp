#pragma once

#include <cstdint>
#include <unordered_map>
#include <utility>

// A caller-owned value that COUNTS its changes, so a retained backend can tell
// whether it moved by comparing one integer instead of the whole value.
//
// Any bound T& is polled (wx on idle, Qt on a 16 ms timer), and with a plain
// T& the only way to see a change is to compare the value with a copy of it --
// a cost that grows with the data. For a 5 000-row table that is most of what
// an idle window spends. Hold the data in an Observable instead and pass it
// where the T& would go:
//
//   Observable<TableRows> rows;
//   Table { columns, rows, picked }            // binds, exactly like TableRows&
//   rows.edit().push_back({ "new.txt", "3" });  // a change: the table refills
//
// Read through get(), change through edit() or set(): both count. The T&
// conversion is for BINDING only -- a write through a reference obtained that
// way is not counted and is never seen. edit() counts once, at the call, so a
// change made through the reference it returns must finish before the event
// handler returns -- keep it to the one statement.
//
// The framework finds the counter by the address of the value
// (observedVersion), so nothing about a widget's signature changes: every
// binding that takes T& takes an Observable<T>. Its own writes -- a selection
// committed, a table cell edited -- count too (markChanged). UI-thread only,
// like every bound value; and like every bound value it must outlive the
// windows bound to it.

namespace observable_detail
{

inline std::unordered_map<const void*, std::uint64_t*>& registry()
{
	static std::unordered_map<const void*, std::uint64_t*> versions;
	return versions;
}

} // namespace observable_detail

// The change counter of the value at `value`, or nullptr when it is a plain
// variable rather than an Observable's.
inline const std::uint64_t* observedVersion(const void* value)
{
	const auto& versions = observable_detail::registry();
	const auto found = versions.find(value);
	return found != versions.end() ? found->second : nullptr;
}

// Count a write the framework made through a bound reference. A no-op for a
// plain variable.
inline void markChanged(const void* value)
{
	auto& versions = observable_detail::registry();
	if (const auto found = versions.find(value); found != versions.end())
		++*found->second;
}

template <typename T>
class Observable
{
public:
	Observable() { enroll(); }

	Observable(T value)
		: m_value(std::move(value))
	{
		enroll();
	}

	Observable(const Observable& other)
		: m_value(other.m_value)
	{
		enroll();
	}

	Observable(Observable&& other)
		: m_value(std::move(other.m_value))
	{
		enroll();
		++other.m_version;
	}

	Observable& operator=(const Observable& other)
	{
		set(other.m_value);
		return *this;
	}

	Observable& operator=(T value)
	{
		set(std::move(value));
		return *this;
	}

	~Observable() { observable_detail::registry().erase(&m_value); }

	const T& get() const { return m_value; }

	// Counts the change, then hands out the value to make it with.
	T& edit()
	{
		++m_version;
		return m_value;
	}

	void set(T value)
	{
		m_value = std::move(value);
		++m_version;
	}

	std::uint64_t version() const { return m_version; }

	// For binding: pass an Observable<T> wherever a widget binds a T&.
	operator T&() { return m_value; }

private:
	void enroll() { observable_detail::registry()[&m_value] = &m_version; }

	T m_value {};
	std::uint64_t m_version = 0;
};
