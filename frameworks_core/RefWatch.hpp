#pragma once

#include "frameworks_core/CoreTypes/Observable.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <tuple>
#include <utility>

// The cheap first half of a RefSync poll: "has anything this binding depends
// on changed since the last poll?", answered from the caller's own variables.
//
// A RefSync entry's `want()` turns bound data into what the native control
// should show -- decoding a selection against a 5 000-row table, copying a
// whole item list -- and the plain entry runs it on every tick just to learn
// that nothing moved. An idle window with a few large bindings spent a third of
// a core doing exactly that on Qt (review finding 7). A RefWatch keeps a copy of
// each source it watches and compares it with operator== -- no allocation, no
// decoding, and an early exit at the first difference -- so `want()` runs only
// on the tick after a source really changed.
//
// It is still a compare of the whole value when nothing changed: O(size), not
// O(1). Plain T& bindings have no other way to tell, because the caller writes
// them directly; the copy here is what replaces the one `want()` used to make.
//
// Unless the source is an Observable's value (Observable.hpp): then it counts
// its own changes, and the watch compares that counter -- O(1), and no copy
// kept. Which kind a source is gets decided once, here, by its address.
//
// A null source -- the unbound half of a BoundValue -- never changes.
template <typename T>
class WatchedSource
{
public:
	explicit WatchedSource(const T* source)
		: m_source(source)
		, m_version(source != nullptr ? observedVersion(source) : nullptr)
	{
		if (m_version != nullptr)
			m_lastVersion = *m_version;
		else if (source != nullptr)
			m_last = *source;
	}

	bool changed()
	{
		if (m_source == nullptr)
			return false;
		if (m_version != nullptr)
		{
			if (*m_version == m_lastVersion)
				return false;
			m_lastVersion = *m_version;
			return true;
		}
		if (*m_source == *m_last)
			return false;
		*m_last = *m_source;
		return true;
	}

private:
	const T* m_source;
	const std::uint64_t* m_version;
	std::uint64_t m_lastVersion = 0;
	std::optional<T> m_last;
};

template <typename... Sources>
class RefWatch
{
public:
	explicit RefWatch(const Sources*... sources)
		: m_sources(WatchedSource<Sources>(sources)...)
	{
	}

	// True once per change of any source; adopts the new values. Every source
	// is asked, so each adopts its change even when an earlier one reported.
	bool changed()
	{
		return std::apply([](auto&... source) {
			bool any = false;
			((any = source.changed() || any), ...);
			return any;
		}, m_sources);
	}

private:
	std::tuple<WatchedSource<Sources>...> m_sources;
};

template <typename... Sources>
RefWatch<Sources...> watchRefs(const Sources*... sources)
{
	return RefWatch<Sources...>(sources...);
}
