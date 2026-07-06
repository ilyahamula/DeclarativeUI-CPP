#include "frameworks_core/DialogWrapper.hpp"
#include "frameworks_core/ControlWrappers.hpp"

#include "frameworks_core/LayoutEngine.hpp"
#include "frameworks_core/LayoutNode.hpp"
#include "frameworks_core/qt/LayoutBackend.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#include <QDialog>
#include <QGuiApplication>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QScreen>
#include <QTimer>
#include <QWindow>

#include <algorithm>

DialogWrapper::DialogWrapper(const std::string& title, const Size& size)
{
	auto* dialog = new QDialog(nullptr);
	dialog->setWindowTitle(QString::fromStdString(title));
	if (size.width > 0 && size.height > 0)
		dialog->resize(size.width, size.height);
	m_nativeWidget = dialog;
}

void DialogWrapper::show()
{
	static_cast<QDialog*>(m_nativeWidget)->show();
}

namespace
{

// QDialog with a resize hook (virtual override — no moc needed).
class EngineDialog : public QDialog
{
public:
	using QDialog::QDialog;
	std::function<void()> onResize;

protected:
	void resizeEvent(QResizeEvent* event) override
	{
		QDialog::resizeEvent(event);
		if (onResize)
			onResize();
	}
};

// Retained engine state for one shown dialog; freed when the dialog is
// destroyed (mirrors the wx session, T3.3).
struct EngineSession
{
	EngineDialog* dialog = nullptr;
	std::unique_ptr<QtLayoutBackend> backend;
	std::unique_ptr<LayoutEngine> engine;
	std::unique_ptr<LayoutNode> root;
	bool autoFit = true;
	bool resizable = false;
	Size fixedContent { -1, -1 };
	bool busy = false;

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
		engine->render(*root, { dialog->width(), dialog->height() });
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
			dialog->setMinimumSize(floor.width, floor.height);
			const int width = std::max(dialog->width(), floor.width);
			const int height = std::max(dialog->height(), floor.height);
			if (width != dialog->width() || height != dialog->height())
				dialog->resize(width, height);
			engine->render(*root, { width, height });
		}
		else if (autoFit)
		{
			dialog->setFixedSize(content.width, content.height);
			engine->render(*root, content);
		}
		else
		{
			engine->render(*root, { dialog->width(), dialog->height() });
		}

		busy = false;
	}

	// AutoGrow text fields re-measure their live content as it changes.
	void bindAutoGrow(LayoutNode& node)
	{
		if (node.isLeaf())
		{
			if (!node.flags.autoGrow() || node.widget == nullptr)
				return;
			auto* window = static_cast<QWidget*>(node.widget->nativeHandle());
			auto queueRelayout = [this] {
				// after the control has applied the edit
				QTimer::singleShot(0, dialog, [this] { relayout(); });
			};
			if (auto* line = qobject_cast<QLineEdit*>(window))
				QObject::connect(line, &QLineEdit::textChanged, dialog,
					[queueRelayout](const QString&) { queueRelayout(); });
			else if (auto* edit = qobject_cast<QPlainTextEdit*>(window))
				QObject::connect(edit, &QPlainTextEdit::textChanged, dialog, queueRelayout);
			return;
		}
		for (auto& child : node.children)
			bindAutoGrow(*child);
	}
};

} // unnamed namespace

void DialogWrapper::runLayoutEngine(const std::string& title, const Size& size,
	std::unique_ptr<LayoutNode> root, bool resizable)
{
	auto* dialog = new EngineDialog(nullptr);
	dialog->setWindowTitle(QString::fromStdString(title));
	dialog->setAttribute(Qt::WA_DeleteOnClose);

	auto* session = new EngineSession;
	session->dialog = dialog;
	session->backend = std::make_unique<QtLayoutBackend>(dialog);
	session->engine = std::make_unique<LayoutEngine>(*session->backend);
	session->root = std::move(root);
	session->resizable = resizable;

	if (const QScreen* screen = QGuiApplication::primaryScreen())
		session->engine->setMaxAutoFitWidth((screen->availableGeometry().width() * 9) / 10);

	// on Qt the dialog widget size IS the engine's content space (the
	// window frame lives outside it)
	const bool fixed = size.width > 0 && size.height > 0;
	session->autoFit = !fixed;
	if (fixed)
		session->fixedContent = { size.width, size.height };
	const Size content = session->engine->resolve(*session->root, session->autoFit
		? Size { -1, -1 }
		: session->fixedContent);

	if (resizable)
	{
		// the measured content is the floor: the user can grow the window
		// but never shrink content into clipping
		const Size floor = session->minClient();
		dialog->setMinimumSize(floor.width, floor.height);
		dialog->resize(content.width, content.height);
	}
	else
	{
		// not user-resizable by default: the engine owns the window size
		dialog->setFixedSize(content.width, content.height);
	}

	session->engine->render(*session->root, content);

	// invalidation wiring -------------------------------------------------
	session->bindAutoGrow(*session->root);
	dialog->onResize = [session] { session->rearrange(); };
	QObject::connect(dialog, &QObject::destroyed, [session] { delete session; });

	dialog->show();

	// display change (DPI/screen move): full re-measure
	if (QWindow* window = dialog->windowHandle())
		QObject::connect(window, &QWindow::screenChanged, dialog,
			[session](QScreen*) { session->relayout(); });
}
