#pragma once

#include "frameworks_core/LayoutEngine.hpp"
#include "frameworks_core/LayoutNode.hpp"
#include "frameworks_core/NodeSource.hpp"
#include "frameworks_core/qt/DialogKeys.hpp"
#include "frameworks_core/qt/LayoutBackend.hpp"
#include "frameworks_core/qt/RefSync.hpp"

#include <QLineEdit>
#include <QObject>
#include <QPlainTextEdit>
#include <QTimer>
#include <QWidget>

#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

// Retained engine state for one shown top-level window; freed when the window
// is destroyed (QObject::destroyed). The Qt twin of
// frameworks_core/wx/EngineSession.hpp -- same contract, same fields.
//
// The window is a plain QWidget rather than the QDialog subclass that owns the
// resize hook, because the same session drives both spellings of a top-level
// window: Dialog (QDialog) today and Window (QMainWindow, T2.1) next. The
// caller owns creation, signal wiring and teardown -- including feeding resizes
// to rearrange() -- and this owns the layout.
struct EngineSession
{
	QWidget* window = nullptr;
	std::unique_ptr<QtLayoutBackend> backend;
	std::unique_ptr<LayoutEngine> engine;
	std::unique_ptr<LayoutNode> root;
	bool autoFit = true;
	bool resizable = false;
	Size fixedContent { -1, -1 };
	bool busy = false;

	// How much bigger the WINDOW is than the space the engine lays out into.
	// Zero for a Dialog and for a Window with no chrome, one menu-bar height
	// once a bar is attached -- QMainWindow stacks the bar above the central
	// widget the engine renders into, so the two rectangles stop agreeing.
	// (Not on macOS, where the bar goes to the system menu and takes no room;
	// the caller measures it and reports what it actually cost.)
	Size chrome { 0, 0 };

	// Lifecycle the caller fills in. `openFlag` is a caller-owned bool the
	// window is shown against (Window::show(bool&), and Dialog's in T3.6):
	// clearing it closes the window, closing the window clears it. `onClose`
	// fires exactly once when the window goes away.
	bool* openFlag = nullptr;
	std::function<void()> onClose;
	bool closed = false; // one-shot guard for notifyClosed()

	// The one close path, whatever triggered it -- the user's close button or
	// the caller clearing `openFlag`. Both clear the flag and fire onClose
	// once, so a caller cannot tell the two apart by what it observes.
	void notifyClosed()
	{
		if (closed)
			return;
		closed = true;
		if (openFlag != nullptr)
			*openFlag = false;
		if (onClose)
			onClose();
	}

	// Polls the caller-owned flag: clearing it closes the window. Qt has no
	// notification for a bool written from somewhere else, so this is the
	// ordinary RefSync shape -- `pull` is whether the window is up now, `want`
	// whether the flag says it should be.
	void bindOpenFlag()
	{
		if (openFlag == nullptr)
			return;
		const bool* flag = openFlag;
		bindExternalRefSync(window,
			[this] { return window->isVisible(); },
			[flag] { return *flag; },
			[this](bool) { window->close(); }); // -> closeEvent -> notifyClosed()
	}

	// The space the engine actually has: the window minus its chrome.
	Size contentSize() const
	{
		return { std::max(0, window->width() - chrome.width),
				 std::max(0, window->height() - chrome.height) };
	}

	// The window size that gives the engine `content`.
	Size windowSizeFor(const Size& content) const
	{
		return { content.width + chrome.width, content.height + chrome.height };
	}

	Size minClient() const
	{
		const EdgeInsets margin = root->flags.border();
		return { root->desired.width + margin.left + margin.right,
				 root->desired.height + margin.top + margin.bottom };
	}

	// Window size changed, measures still valid: arrange-only.
	void rearrange()
	{
		if (busy)
			return;
		busy = true;
		engine->render(*root, contentSize());
		busy = false;
	}

	// Content or display metrics changed: full re-measure. Auto-fit windows
	// follow their content; resizable ones keep the user's size but never
	// below the new content floor.
	void relayout()
	{
		if (busy)
			return;
		busy = true;

		const Size content = engine->resolve(*root, autoFit ? Size { -1, -1 } : fixedContent);
		if (autoFit && resizable)
		{
			const Size floor = windowSizeFor(minClient());
			window->setMinimumSize(floor.width, floor.height);
			const int width = std::max(window->width(), floor.width);
			const int height = std::max(window->height(), floor.height);
			if (width != window->width() || height != window->height())
				window->resize(width, height);
			engine->render(*root, contentSize());
		}
		else if (autoFit)
		{
			const Size target = windowSizeFor(content);
			window->setFixedSize(target.width, target.height);
			engine->render(*root, content);
		}
		else
		{
			engine->render(*root, contentSize());
		}

		busy = false;
	}

	// Arms a re-measure for every source of engine-visible change in the tree.
	//
	// AutoGrow fields are event-driven: their live text is what measure reads,
	// so an edit re-measures. Everything else -- sash positions, Expander open
	// states, isHidden() flags, ForEach data -- is one poll over the tree
	// (refreshTree, NodeSource.hpp) rather than a poll per node: rows can now
	// be destroyed and rebuilt, and a poll holding a pointer into a node would
	// outlive it. Walking the live tree each time holds none.
	void watch()
	{
		bindAutoGrow(*root);
		refsync_detail::hubFor(window)->add(window, [this] { poll(); });
		// Enter / Escape for default and cancel buttons (DialogKeys.hpp).
		qt_dialog_keys::install(window);
	}

	void poll()
	{
		if (busy)
			return;
		std::vector<LayoutNode*> fresh;
		if (!refreshTree(*root, *backend, fresh))
			return;
		relayout(); // realizes the fresh rows
		for (LayoutNode* row : fresh)
			bindAutoGrow(*row);
	}

	void bindAutoGrow(LayoutNode& node)
	{
		if (!node.isLeaf())
		{
			for (auto& child : node.children)
				bindAutoGrow(*child);
			return;
		}
		if (!node.flags.autoGrow() || node.widget == nullptr)
			return;
		auto* control = static_cast<QWidget*>(node.widget->nativeHandle());
		auto queueRelayout = [this] {
			// after the control has applied the edit
			QTimer::singleShot(0, window, [this] { relayout(); });
		};
		if (auto* line = qobject_cast<QLineEdit*>(control))
			QObject::connect(line, &QLineEdit::textChanged, window,
				[queueRelayout](const QString&) { queueRelayout(); });
		else if (auto* edit = qobject_cast<QPlainTextEdit*>(control))
			QObject::connect(edit, &QPlainTextEdit::textChanged, window, queueRelayout);
	}
};
