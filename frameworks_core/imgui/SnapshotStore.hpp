#pragma once

#include "frameworks_core/CoreTypes/BoundValue.hpp"
#include "frameworks_core/imgui/ImGuiWidgetIdManager.hpp"

#include <cstdint>
#include <unordered_map>

// Persistent home for the SNAPSHOT half of a BoundValue (USE_IMGUI only).
//
// ImGui rebuilds the declarative tree every frame, so a wrapper -- and the
// snapshot it copied out of the widget -- dies with the frame and is built
// again from the caller's literal on the next one. Without this, the literal
// wins every frame: CheckBox{"On", false} un-ticks itself the instant it is
// ticked, a Slider snaps back to its declared position, a typed field clears.
// The retained backends have no such problem -- there the wrapper is created
// once and lives with the native window, which retains the edit itself.
//
// So an unbound value's storage lives here instead of in the wrapper, keyed the
// same way the control's own ImGui state is (scope seed + the sequential widget
// id, see WidgetIdManager). That deliberately gives it exactly ImGui's own
// stability: the value survives for as long as the control keeps its place in
// the tree, and a tree whose shape shifts renumbers both alike.
//
// A BOUND value needs none of this -- it is the caller's variable and already
// outlives the frame, the tree and the wrapper -- so every entry point below is
// a no-op while isBound(). That is what keeps the two halves of BoundValue
// behaving the same on all three backends rather than only on wx and Qt.
template <typename T>
class SnapshotStore
{
public:
	// The first frame seeds the store from the declared literal; every later
	// frame overwrites the freshly rebuilt literal with what the control last
	// committed.
	static void restore(std::uint64_t key, BoundValue<T>& value)
	{
		if (value.isBound())
			return;
		const auto [it, inserted] = s_snapshots.try_emplace(key, value.get());
		if (!inserted)
			value.set(it->second);
	}

	static void commit(std::uint64_t key, const BoundValue<T>& value)
	{
		if (value.isBound())
			return;
		s_snapshots[key] = value.get();
	}

private:
	static inline std::unordered_map<std::uint64_t, T> s_snapshots;
};

// Adopts a stored snapshot for the duration of a render() and writes back
// whatever the control committed into it. Declare it before the first read of
// the value -- several wrappers bind a reference into the BoundValue and edit
// through it in place.
template <typename T>
class SnapshotScope
{
public:
	SnapshotScope(std::uint64_t key, BoundValue<T>& value)
		: m_key(key)
		, m_value(value)
	{
		SnapshotStore<T>::restore(m_key, m_value);
	}

	~SnapshotScope() { SnapshotStore<T>::commit(m_key, m_value); }

	SnapshotScope(const SnapshotScope&) = delete;
	SnapshotScope& operator=(const SnapshotScope&) = delete;

private:
	std::uint64_t m_key;
	BoundValue<T>& m_value;
};

// The usual case: one control, one value. Consumes the control's sequential
// ImGui id, which render() still needs for its own PushID() -- so a wrapper
// declares this instead of calling WidgetIdManager::nextWidgetId() by hand, and
// the id is consumed exactly once either way.
template <typename T>
class WidgetSnapshot
{
public:
	explicit WidgetSnapshot(BoundValue<T>& value)
		: m_id(WidgetIdManager::nextWidgetId())
		, m_scope(WidgetIdManager::stateKey(m_id), value)
	{
	}

	int id() const { return m_id; }

	// Key for a second value on the same control (the Table's rows next to its
	// selection), so the two never share a slot.
	std::uint64_t slotKey(int slot) const { return WidgetIdManager::stateKey(m_id, slot); }

private:
	int m_id;
	SnapshotScope<T> m_scope;
};
