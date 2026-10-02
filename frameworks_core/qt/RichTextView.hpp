#pragma once

#include "frameworks_core/CoreTypes/RichTextRuns.hpp"

#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QWidget>

#include <array>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// The native half of a RichText on Qt: a plain QWidget that paints the
// fragments layoutRichText() places, in QFont variants of its own font.
//
// Not a QLabel fed HTML: a rich-text QLabel wraps by Qt's text engine rather
// than the shared rule, and its heightForWidth is unreliable enough that an
// auto-fit window cannot trust it. A virtual-override subclass, no Q_OBJECT.
class RichTextView : public QWidget
{
public:
	RichTextView(QWidget* parent, std::vector<TextRun> runs)
		: QWidget(parent)
		, m_runs(std::move(runs))
	{
		setMouseTracking(true);
		setFocusPolicy(Qt::NoFocus);
	}

	void setOnLink(std::function<void(const std::string&)> onLink)
	{
		m_onLink = std::move(onLink);
	}

	// Cached by width: paint, every mouse move (the link hit test) and the
	// measure pass all ask. The width is the key; a font or style change
	// clears it (changeEvent below), since the metrics are the widget's own.
	const RichTextLayout& layoutFor(int width) const
	{
		if (m_cachedWidth == width && m_cached)
			return *m_cached;
		std::array<QFontMetrics, 4> metrics {
			QFontMetrics(fontFor(false, false, false)), QFontMetrics(fontFor(true, false, false)),
			QFontMetrics(fontFor(false, true, false)), QFontMetrics(fontFor(true, true, false)),
		};
		int lineHeight = 0;
		for (const QFontMetrics& m : metrics)
			lineHeight = std::max(lineHeight, m.height());

		m_cachedWidth = width;
		m_cached = layoutRichText(m_runs, width, lineHeight,
			[this](const TextRun& run, std::string_view text) {
				return QFontMetrics(fontFor(run)).horizontalAdvance(
					QString::fromUtf8(text.data(), (qsizetype)text.size()));
			});
		return *m_cached;
	}

protected:
	void paintEvent(QPaintEvent*) override
	{
		QPainter painter(this);
		const RichTextLayout& layout = layoutFor(width());
		for (const RichTextFragment& f : layout.fragments)
		{
			const TextRun& run = m_runs[f.run];
			const QFont font = fontFor(run);
			painter.setFont(font);
			painter.setPen(colourFor(run));
			painter.drawText(QPoint(f.x, f.y + QFontMetrics(font).ascent()),
				QString::fromUtf8(f.text.data(), (qsizetype)f.text.size()));
		}
	}

	void changeEvent(QEvent* event) override
	{
		if (event->type() == QEvent::FontChange || event->type() == QEvent::StyleChange)
			m_cached.reset();
		QWidget::changeEvent(event);
	}

	void resizeEvent(QResizeEvent* event) override
	{
		// The wrap depends on the width, so any resize can move every line.
		update();
		QWidget::resizeEvent(event);
	}

	void mousePressEvent(QMouseEvent* event) override
	{
		if (event->button() == Qt::LeftButton)
			m_pressed = linkUnder(event->position().toPoint());
		QWidget::mousePressEvent(event);
	}

	// A link fires when the press and the release both land on it: the rule
	// ImGui and wx follow too.
	void mouseReleaseEvent(QMouseEvent* event) override
	{
		const std::string* released = event->button() == Qt::LeftButton
			? linkUnder(event->position().toPoint())
			: nullptr;
		const bool fire = released != nullptr && released == m_pressed;
		m_pressed = nullptr;
		QWidget::mouseReleaseEvent(event);
		if (fire && m_onLink)
		{
			const std::string url = *released;
			m_onLink(url);
		}
	}

	void mouseMoveEvent(QMouseEvent* event) override
	{
		if (linkUnder(event->position().toPoint()) != nullptr)
			setCursor(Qt::PointingHandCursor);
		else
			unsetCursor();
		QWidget::mouseMoveEvent(event);
	}

private:
	QFont fontFor(bool bold, bool italic, bool underline) const
	{
		QFont f = font();
		f.setBold(bold);
		f.setItalic(italic);
		f.setUnderline(underline);
		return f;
	}

	QFont fontFor(const TextRun& run) const
	{
		return fontFor(run.bold, run.italic, run.isLink());
	}

	// QWidget repaints itself on an EnabledChange, so reading isEnabled() here
	// is all a bound disabled flag needs.
	QColor colourFor(const TextRun& run) const
	{
		if (!isEnabled())
			return palette().color(QPalette::Disabled, QPalette::WindowText);
		if (run.colour)
			return QColor::fromRgbF(run.colour->r, run.colour->g, run.colour->b, run.colour->a);
		if (run.isLink())
			return palette().color(QPalette::Link);
		return palette().color(QPalette::WindowText);
	}

	const std::string* linkUnder(const QPoint& p) const
	{
		return linkAt(layoutFor(width()), m_runs, p.x(), p.y());
	}

	std::vector<TextRun> m_runs;
	std::function<void(const std::string&)> m_onLink;
	const std::string* m_pressed = nullptr;
	mutable int m_cachedWidth = -1;
	mutable std::optional<RichTextLayout> m_cached;
};
