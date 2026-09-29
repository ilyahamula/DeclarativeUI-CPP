#include "frameworks_core/UiThread.hpp"

#include <QCoreApplication>
#include <QMetaObject>

void postToUi(std::function<void()> task)
{
	// A queued invocation on an object that lives in the UI thread runs there,
	// whichever thread queued it.
	if (QCoreApplication* app = QCoreApplication::instance(); app != nullptr && task)
		QMetaObject::invokeMethod(app, std::move(task), Qt::QueuedConnection);
}
