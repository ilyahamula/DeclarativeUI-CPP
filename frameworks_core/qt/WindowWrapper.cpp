#include "frameworks_core/WindowWrapper.hpp"

#include "frameworks_core/LayoutEngine.hpp"
#include "frameworks_core/LayoutNode.hpp"
#include "frameworks_core/qt/EngineSession.hpp"
#include "frameworks_core/qt/LayoutBackend.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#include <QCloseEvent>
#include <QGuiApplication>
#include <QMainWindow>
#include <QResizeEvent>
#include <QScreen>
#include <QWidget>
#include <QWindow>

namespace
{

// QMainWindow with resize and close hooks (virtual overrides -- no moc needed,
// the same shape EngineDialog uses in DialogWrapper.cpp).
class EngineWindow : public QMainWindow
{
public:
	using QMainWindow::QMainWindow;
	std::function<void()> onResize;
	std::function<void()> onClosing;

protected:
	void resizeEvent(QResizeEvent* event) override
	{
		QMainWindow::resizeEvent(event);
		if (onResize)
			onResize();
	}

	void closeEvent(QCloseEvent* event) override
	{
		if (onClosing)
			onClosing();
		QMainWindow::closeEvent(event);
	}
};

} // unnamed namespace

void WindowWrapper::runLayoutEngine(const std::string& title, const Size& size,
	std::unique_ptr<LayoutNode> root, bool resizable, std::function<void()> onClose, bool* open)
{
	auto* window = new EngineWindow(nullptr);
	window->setWindowTitle(QString::fromStdString(title));
	window->setAttribute(Qt::WA_DeleteOnClose);

	// The engine lays out into the CENTRAL WIDGET, not the window: that is what
	// keeps the menu bar (T2.2) outside the content space, since QMainWindow
	// stacks its chrome above and below the central widget. With no chrome yet
	// the central widget is exactly the window's contents rect, which is why
	// the session can still size against the window itself below.
	auto* host = new QWidget(window);
	window->setCentralWidget(host);

	auto* session = new EngineSession;
	session->window = window;
	session->backend = std::make_unique<QtLayoutBackend>(host);
	session->engine = std::make_unique<LayoutEngine>(*session->backend);
	session->root = std::move(root);
	session->resizable = resizable;
	session->openFlag = open;
	session->onClose = std::move(onClose);

	if (const QScreen* screen = QGuiApplication::primaryScreen())
		session->engine->setMaxAutoFitWidth((screen->availableGeometry().width() * 9) / 10);

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
		window->setMinimumSize(floor.width, floor.height);
		window->resize(content.width, content.height);
	}
	else
	{
		// Fixed(): the engine owns the window size
		window->setFixedSize(content.width, content.height);
	}

	session->engine->render(*session->root, content);

	// invalidation wiring -------------------------------------------------
	session->bindInvalidation(*session->root);
	session->bindOpenFlag();
	window->onResize = [session] { session->rearrange(); };
	// The single close path: the user's close button lands here, and so does
	// bindOpenFlag()'s close() when the caller clears the flag.
	window->onClosing = [session] { session->notifyClosed(); };
	QObject::connect(window, &QObject::destroyed, [session] {
		// destroyed without a close event (the app tearing down) still counts
		session->notifyClosed();
		delete session;
	});

	window->show();

	// display change (DPI/screen move): full re-measure
	if (QWindow* handle = window->windowHandle())
		QObject::connect(handle, &QWindow::screenChanged, window,
			[session](QScreen*) { session->relayout(); });
}
