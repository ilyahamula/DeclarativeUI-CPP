#pragma once

#include <QObject>
#include <QPointer>
#include <QSignalBlocker>
#include <QTimer>
#include <QWidget>

#include "frameworks_core/RefWatch.hpp"

#include <deque>
#include <functional>
#include <utility>

// Mirrors an externally-owned value back into a native widget -- the Qt twin of
// frameworks_core/wx/RefSync.hpp. Same contract, different clock: wx has an idle
// event to ride on, Qt does not, so each bound widget carries a low-frequency
// timer instead.
//
// The sync is driven by CHANGES TO THE REF, never by the widget merely disagreeing
// with it. That distinction is load-bearing. A widget can legitimately display
// something the ref does not yet know about: while a combo box's popup is open the
// highlighted item has moved, but the commit signal has not fired, so the ref still
// holds the old value. Pushing on disagreement would reset the widget mid-gesture
// and cancel the interaction outright -- the user sees a control they cannot change,
// and the commit signal never arrives. (That is not hypothetical: it is exactly the
// bug this shape was written to fix on wx.) Watching the ref instead means an
// untouched value produces no writes at all, whatever the widget is doing.
//
// `want` reports the value the widget should show and `pull` what it shows now, both
// in the widget's own domain (ints for sliders, QDate for the date edit) so the
// compare never depends on float equality. `pull` is consulted only after the ref has
// actually changed, purely to skip a redundant write.
//
// Every push runs under a QSignalBlocker. Unlike wx, Qt setters emit their change
// signal for programmatic writes too, so without the block a mirrored write would
// re-enter the user's onChange and echo back into the ref.
//
// Capture the bound value by reference, never the wrapper: the ref belongs to the
// caller and outlives everything here, whereas wrapper and widget teardown order is
// not fixed.
//
// One clock per WINDOW, not per binding. Every sync registered under a top-level
// window rides a single timer owned by that window, which walks the entries in
// registration order; an entry whose widget has been destroyed is skipped and
// dropped. A dialog full of bound controls is therefore one timer rather than one
// per property, and nothing outlives the window.

// Poll interval. Fast enough that an externally driven value looks immediate,
// slow enough that a dialog full of bound widgets stays cheap.
inline constexpr int kRefSyncIntervalMs = 16;

namespace refsync_detail
{

// The per-window clock. A plain QObject subclass, no Q_OBJECT: it declares no
// signals or slots, and is found again by object name rather than by
// qobject_cast (which needs a meta-object of its own).
class Hub : public QObject
{
public:
	static constexpr const char* kName = "dui_refsync_hub";

	explicit Hub(QWidget* window)
		: QObject(window)
	{
		setObjectName(QLatin1String(kName));
		// A child, so it dies with the hub and shows up as the window's one
		// timer to anyone who looks.
		auto* timer = new QTimer(this);
		QObject::connect(timer, &QTimer::timeout, this, [this] { tick(); });
		timer->start(kRefSyncIntervalMs);
	}

	void add(QWidget* control, std::function<void()> sync)
	{
		m_entries.push_back(Entry { control, std::move(sync) });
	}

private:
	struct Entry
	{
		QPointer<QWidget> control;
		std::function<void()> sync;
	};

	void tick()
	{
		// Dead entries are dropped BEFORE the walk, never during it: a push can
		// relayout the window, which realizes new controls and registers more
		// entries here. A deque keeps the running entry where it is while the
		// walk appends, and re-reading size() picks the newcomers up.
		std::erase_if(m_entries, [](const Entry& e) { return e.control.isNull(); });
		for (std::size_t i = 0; i < m_entries.size(); ++i)
		{
			if (!m_entries[i].control.isNull())
				m_entries[i].sync();
		}
	}

	std::deque<Entry> m_entries;
};

inline Hub* hubFor(QWidget* control)
{
	QWidget* window = control->window();
	if (QObject* found = window->findChild<QObject*>(QLatin1String(Hub::kName), Qt::FindDirectChildrenOnly))
		return static_cast<Hub*>(found);
	return new Hub(window);
}

} // namespace refsync_detail

// As bindExternalRefSync, for a binding whose `want()` is expensive -- an item
// list, a table, a decoded selection. `watch` (a RefWatch over the caller's
// variables `want` reads) is asked first, and `want()` runs only after it
// reports a change; the push rule is unchanged.
template <typename Watch, typename Pull, typename Want, typename Push>
void bindWatchedRefSync(QWidget* control, Watch watch, Pull pull, Want want, Push push)
{
	refsync_detail::hubFor(control)->add(control,
		[control, watch = std::move(watch), pull = std::move(pull), want = std::move(want),
			push = std::move(push), last = want()]() mutable {
			if (!watch.changed())
				return;
			auto target = want();
			if (target != last)
			{
				if (pull() != target)
				{
					const QSignalBlocker block(control);
					push(target);
				}
				last = std::move(target);
			}
		});
}

template <typename Pull, typename Want, typename Push>
void bindExternalRefSync(QWidget* control, Pull pull, Want want, Push push)
{
	refsync_detail::hubFor(control)->add(control,
		[control, pull = std::move(pull), want = std::move(want), push = std::move(push),
			last = want()]() mutable {
			const auto target = want();
			if (target != last)
			{
				last = target;
				if (pull() != target)
				{
					const QSignalBlocker block(control);
					push(target);
				}
			}
		});
}
