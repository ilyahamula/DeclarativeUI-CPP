#include "frameworks_core/DialogWrapper.hpp"
#include "frameworks_core/ControlWrappers.hpp"

#include "frameworks_core/LayoutEngine.hpp"
#include "frameworks_core/LayoutNode.hpp"
#include "frameworks_core/qt/EngineSession.hpp"
#include "frameworks_core/qt/LayoutBackend.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#include <QCloseEvent>
#include <QDialog>
#include <QGuiApplication>
#include <QResizeEvent>
#include <QScreen>
#include <QWindow>

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

// QDialog with resize and close hooks (virtual overrides — no moc needed, the
// same shape EngineWindow uses in WindowWrapper.cpp).
class EngineDialog : public QDialog
{
public:
	using QDialog::QDialog;
	std::function<void()> onResize;
	std::function<void()> onClosing;

protected:
	void resizeEvent(QResizeEvent* event) override
	{
		QDialog::resizeEvent(event);
		if (onResize)
			onResize();
	}

	void closeEvent(QCloseEvent* event) override
	{
		if (onClosing)
			onClosing();
		QDialog::closeEvent(event);
	}
};

} // unnamed namespace

void DialogWrapper::runLayoutEngine(const std::string& title, const Size& size,
	std::unique_ptr<LayoutNode> root, bool resizable, const std::optional<Position>& position,
	bool modal, std::function<void()> onClose, bool* open)
{
	auto* dialog = new EngineDialog(nullptr);
	dialog->setWindowTitle(QString::fromStdString(title));
	dialog->setAttribute(Qt::WA_DeleteOnClose);

	auto* session = new EngineSession;
	session->window = dialog;
	session->backend = std::make_unique<QtLayoutBackend>(dialog);
	session->engine = std::make_unique<LayoutEngine>(*session->backend);
	session->root = std::move(root);
	session->resizable = resizable;
	session->openFlag = open;
	session->onClose = std::move(onClose);

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
	session->bindInvalidation(*session->root);
	session->bindOpenFlag();
	dialog->onResize = [session] { session->rearrange(); };
	// The single close path: the user's close button lands here, and so does
	// bindOpenFlag()'s close() when the caller clears the flag.
	dialog->onClosing = [session] { session->notifyClosed(); };
	// And the path that does NOT send a close event: Escape reaches a QDialog as
	// reject(), which goes straight to done() -- so the close would otherwise go
	// unreported. finished() is the one signal every dismissal passes through.
	// deleteLater() beside it because done() only hides: a closed Dialog is gone,
	// as it is on wx, and the session has to go with it.
	QObject::connect(dialog, &QDialog::finished, [session, dialog](int) {
		session->notifyClosed();
		dialog->deleteLater();
	});
	QObject::connect(dialog, &QObject::destroyed, [session] {
		// destroyed without a close event (the app tearing down) still counts
		session->notifyClosed();
		delete session;
	});

	// after sizing, so a window sized from the engine still opens where the
	// caller asked rather than where the platform put the default-positioned one
	if (position)
		dialog->move(position->x, position->y);

	// show(), never exec(): the modality is a window property, so Qt locks the
	// other windows out while this call still returns immediately -- which is
	// what makes Modal() non-blocking on every backend.
	if (modal)
		dialog->setWindowModality(Qt::ApplicationModal);

	dialog->show();

	// display change (DPI/screen move): full re-measure
	if (QWindow* window = dialog->windowHandle())
		QObject::connect(window, &QWindow::screenChanged, dialog,
			[session](QScreen*) { session->relayout(); });
}
