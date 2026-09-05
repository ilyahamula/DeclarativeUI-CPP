#include "frameworks_core/DialogWrapper.hpp"
#include "frameworks_core/ControlWrappers.hpp"

#include "frameworks_core/LayoutEngine.hpp"
#include "frameworks_core/LayoutNode.hpp"
#include "frameworks_core/qt/EngineSession.hpp"
#include "frameworks_core/qt/LayoutBackend.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#include <QDialog>
#include <QGuiApplication>
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

} // unnamed namespace

void DialogWrapper::runLayoutEngine(const std::string& title, const Size& size,
	std::unique_ptr<LayoutNode> root, bool resizable)
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
	dialog->onResize = [session] { session->rearrange(); };
	QObject::connect(dialog, &QObject::destroyed, [session] { delete session; });

	dialog->show();

	// display change (DPI/screen move): full re-measure
	if (QWindow* window = dialog->windowHandle())
		QObject::connect(window, &QWindow::screenChanged, dialog,
			[session](QScreen*) { session->relayout(); });
}
