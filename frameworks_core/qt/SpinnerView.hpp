#pragma once

#include "frameworks_core/CoreTypes/Spinner.hpp"

#include <QElapsedTimer>
#include <QPainter>
#include <QPixmap>
#include <QTimer>
#include <QWidget>

#include <algorithm>
#include <cmath>

// The native half of a Spinner on Qt. Qt has no activity indicator (a
// QProgressBar with an empty range is a bar, not a spinner), so it is painted:
// a three-quarter arc in the text colour, or -- withImage() -- the picture
// turned about its centre. The palette's current colour group follows the
// enabled state, so a disabled spinner draws dimmed for free.
//
// The QTimer is a child, so it dies with the widget, and runs only while the
// spinner does. A virtual override, no Q_OBJECT.
class SpinnerView : public QWidget
{
public:
	SpinnerView(QWidget* parent, QPixmap image)
		: QWidget(parent)
		, m_image(std::move(image))
		, m_timer(new QTimer(this))
	{
		QObject::connect(m_timer, &QTimer::timeout, this, [this] { update(); });
	}

	void setRunning(bool running)
	{
		if (running == m_running)
			return;
		m_running = running;
		if (running)
		{
			m_clock.start();
			m_timer->start(kSpinnerFrameMs);
		}
		else
		{
			m_timer->stop();
		}
		update();
	}

	bool isRunning() const { return m_running; }
	bool hasImage() const { return !m_image.isNull(); }
	bool timerActive() const { return m_timer->isActive(); }

protected:
	void paintEvent(QPaintEvent*) override
	{
		const bool turning = m_running;
		if (!turning && m_image.isNull())
			return; // stopped: the default spinner draws nothing
		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setRenderHint(QPainter::SmoothPixmapTransform);
		const double side = std::min(width(), height());
		const double degrees = turning ? spinnerAngle(m_clock.elapsed()) * 180.0 / 3.141592653589793 : 0.0;
		painter.translate(width() * 0.5, height() * 0.5);
		painter.rotate(degrees);
		if (!m_image.isNull())
		{
			const double scale = side / std::max(m_image.width(), m_image.height());
			const QSizeF size(m_image.width() * scale, m_image.height() * scale);
			if (!isEnabled())
				painter.setOpacity(0.4);
			painter.drawPixmap(QRectF(QPointF(-size.width() * 0.5, -size.height() * 0.5), size), m_image,
				QRectF(m_image.rect()));
			return;
		}
		const double pen = std::max(2.0, side / 9.0);
		const double radius = side * 0.5 - pen;
		painter.setPen(QPen(palette().color(QPalette::WindowText), pen, Qt::SolidLine, Qt::RoundCap));
		// QPainter angles are 1/16 degree, counter-clockwise from 3 o'clock.
		painter.drawArc(QRectF(-radius, -radius, radius * 2.0, radius * 2.0), 0, 270 * 16);
	}

private:
	QPixmap m_image;
	QTimer* m_timer;
	QElapsedTimer m_clock;
	bool m_running = false;
};
