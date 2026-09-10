#pragma once

#include "frameworks_core/LayoutEngine.hpp"
#include "frameworks_core/LayoutNode.hpp"
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

	// Lifecycle seams the caller fills in; both are inert until the elements
	// that set them land. `openFlag` is a caller-owned bool the window is shown
	// against (Dialog/Window::show(bool&), T3.6/T2.1): clearing it closes the
	// window, closing the window clears it. `onClose` fires exactly once when
	// the window goes away.
	bool* openFlag = nullptr;
	std::function<void()> onClose;

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
		engine->render(*root, { window->width(), window->height() });
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
			const Size floor = minClient();
			window->setMinimumSize(floor.width, floor.height);
			const int width = std::max(window->width(), floor.width);
			const int height = std::max(window->height(), floor.height);
			if (width != window->width() || height != window->height())
				window->resize(width, height);
			engine->render(*root, { width, height });
		}
		else if (autoFit)
		{
			window->setFixedSize(content.width, content.height);
			engine->render(*root, content);
		}
		else
		{
			engine->render(*root, { window->width(), window->height() });
		}

		busy = false;
	}

	// Arms a re-measure for every source of engine-visible change in the tree.
	//
	// Today that is AutoGrow text fields, whose live content the measure pass
	// reads, and Splitter sash positions. The remaining source is an Expander's
	// collapsed state (T1.6), which joins here on the same terms.
	void bindInvalidation(LayoutNode& node)
	{
		// A sash drag (or anything else writing the bound int) changes a value
		// the engine reads, and Qt has no notification for that -- so it is
		// polled like any other external ref. `resolved` is what the layout
		// currently shows and `position` what it should show, which makes this
		// the ordinary RefSync shape rather than a special case: a full
		// re-measure, because the pane widths decide table columns and text
		// wrapping, not just where the rectangles land.
		if (node.kind == NodeKind::Splitter)
		{
			SplitterState* split = &node.split;
			bindExternalRefSync(window,
				[split] { return split->resolved; },
				[split] { return split->position.get(); },
				[this](int) { relayout(); });
		}
		if (node.isLeaf())
		{
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
			return;
		}
		for (auto& child : node.children)
			bindInvalidation(*child);
	}
};
