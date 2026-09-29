#include "frameworks_core/ToastWrapper.hpp"
#include "frameworks_core/qt/ToastWindow.hpp"

#include <QApplication>
#include <QFontMetrics>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include <QScreen>
#include <QTimer>

#include <algorithm>

namespace
{

// The geometry every backend's toast shares (imgui/ToastQueue.cpp), so the
// three look alike.
constexpr int kMargin = 16;
constexpr int kSpacing = 8;
constexpr int kPadX = 12;
constexpr int kPadY = 10;
constexpr int kAccentW = 4;
constexpr int kMaxTextW = 360;
constexpr int kRounding = 6;

QString accentColor(MessageBoxStyle style)
{
	switch (style)
	{
	case MessageBoxStyle::Warning:  return QStringLiteral("rgb(255, 204, 0)");
	case MessageBoxStyle::Error:    return QStringLiteral("rgb(255, 51, 51)");
	case MessageBoxStyle::Question: return QStringLiteral("rgb(102, 204, 102)");
	case MessageBoxStyle::Info:     break;
	}
	return QStringLiteral("rgb(51, 153, 255)");
}

// Where the stack hangs from: the client area of the active window, else of any
// visible application window, else the primary screen. A toast never activates,
// so it can never be the active window itself; the cast keeps it from being the
// fallback either.
QRect anchorRect()
{
	if (QWidget* active = QApplication::activeWindow())
		return active->geometry();
	for (QWidget* widget : QApplication::topLevelWidgets())
	{
		if (widget->isVisible() && widget->windowType() != Qt::ToolTip
			&& dynamic_cast<ToastWindow*>(widget) == nullptr)
			return widget->geometry();
	}
	if (QScreen* screen = QGuiApplication::primaryScreen())
		return screen->availableGeometry();
	return {};
}

} // unnamed namespace

ToastWindow::ToastWindow(const std::string& message, MessageBoxStyle style, int durationMs)
	: QWidget(nullptr, Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint
		| Qt::WindowDoesNotAcceptFocus)
	, m_durationMs(durationMs > 0 ? durationMs : ToastWrapper::kDefaultDurationMs)
{
	setAttribute(Qt::WA_ShowWithoutActivating);
	setAttribute(Qt::WA_DeleteOnClose);
	// Takes no input, as on ImGui: a click lands on whatever is underneath.
	setAttribute(Qt::WA_TransparentForMouseEvents);
	// The rounded frame below draws the visible shape; the window's own
	// rectangle stays see-through around its corners.
	setAttribute(Qt::WA_TranslucentBackground);

	auto* frame = new QFrame(this);
	frame->setObjectName(QStringLiteral("toastFrame"));
	frame->setStyleSheet(QStringLiteral(
		"#toastFrame { background: palette(window); border: 1px solid palette(mid);"
		" border-radius: %1px; }").arg(kRounding));

	auto* accent = new QFrame(frame);
	accent->setFixedWidth(kAccentW);
	accent->setStyleSheet(QStringLiteral(
		"background: %1; border: none; border-top-left-radius: %2px;"
		" border-bottom-left-radius: %2px;").arg(accentColor(style)).arg(kRounding));

	auto* label = new QLabel(QString::fromStdString(message), frame);
	label->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
	label->setContentsMargins(kPadX, kPadY, kPadX, kPadY);
	// A word-wrapped QLabel picks its own width heuristically, so it only wraps
	// once the text is genuinely past the cap -- and then at exactly the cap.
	if (QFontMetrics(label->font()).horizontalAdvance(label->text()) > kMaxTextW)
	{
		label->setWordWrap(true);
		label->setFixedWidth(kMaxTextW + kPadX * 2);
	}

	auto* row = new QHBoxLayout(frame);
	row->setContentsMargins(0, 0, 0, 0);
	row->setSpacing(0);
	row->addWidget(accent);
	row->addWidget(label);

	auto* outer = new QHBoxLayout(this);
	outer->setContentsMargins(0, 0, 0, 0);
	outer->addWidget(frame);
}

ToastWindow::~ToastWindow()
{
	auto& live = stack();
	live.erase(std::remove(live.begin(), live.end(), this), live.end());
	reflow();
}

void ToastWindow::popUp()
{
	adjustSize();
	stack().push_back(this);
	reflow();
	show();

	// Hold, then fade over the last kToastFadeMs, then go. Both the timer and
	// the animation are owned by the toast, so nothing can fire into a toast
	// that has already been deleted.
	const int fadeMs = std::min(m_durationMs, ToastWrapper::kToastFadeMs);
	QTimer::singleShot(m_durationMs - fadeMs, this, [this, fadeMs]() {
		auto* fade = new QPropertyAnimation(this, "windowOpacity", this);
		fade->setDuration(fadeMs);
		fade->setStartValue(1.0);
		fade->setEndValue(0.0);
		QObject::connect(fade, &QPropertyAnimation::finished, this, [this]() { close(); });
		fade->start();
	});
}

void ToastWindow::reflow()
{
	const auto& live = stack();
	if (live.empty())
		return;

	const QRect anchor = anchorRect();
	const int right = anchor.right() - kMargin;
	int bottom = anchor.bottom() - kMargin;
	for (ToastWindow* toast : live)
	{
		const QSize size = toast->size();
		toast->move(right - size.width() + 1, bottom - size.height() + 1);
		bottom -= size.height() + kSpacing;
	}
}

std::vector<ToastWindow*>& ToastWindow::stack()
{
	static std::vector<ToastWindow*> live;
	return live;
}

void ToastWrapper::show(const std::string& message, MessageBoxStyle style, int durationMs)
{
	(new ToastWindow(message, style, durationMs))->popUp();
}
