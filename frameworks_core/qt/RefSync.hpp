#pragma once

#include <QObject>
#include <QSignalBlocker>
#include <QTimer>
#include <QWidget>

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
// not fixed. The timer is parented to the widget and the connection carries it as
// context, so both die with it.

// Poll interval. Fast enough that an externally driven value looks immediate,
// slow enough that a dialog full of bound widgets stays cheap.
inline constexpr int kRefSyncIntervalMs = 16;

template <typename Pull, typename Want, typename Push>
void bindExternalRefSync(QWidget* control, Pull pull, Want want, Push push)
{
	auto* timer = new QTimer(control);
	QObject::connect(timer, &QTimer::timeout, control,
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
	timer->start(kRefSyncIntervalMs);
}
