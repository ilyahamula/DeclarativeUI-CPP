#include "frameworks_core/ControlWrappers.hpp"
#include "frameworks_core/qt/DialogKeys.hpp"
#include "frameworks_core/qt/TextField.hpp"
#include "frameworks_core/qt/RefSync.hpp"
#include <algorithm>
#include <cmath>
#include <memory>

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#include <QAction>
#include <QIcon>
#include <QPen>
#include <QKeyEvent>
#include <QStatusBar>
#include <QToolBar>
#include <QButtonGroup>
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QStyle>
#include <QTableWidget>
#include <QTimeEdit>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>
#include <QHBoxLayout>

#include "frameworks_core/qt/FileDialogSupport.hpp"
#include "frameworks_core/qt/Labels.hpp"
#include "frameworks_core/qt/RichTextView.hpp"

// realize() creates the QWidget under the given parent window and connects
// its signals (plain lambda connects — no moc). The layout engine measures
// via sizeHint and places via setGeometry (qt/LayoutBackend.cpp).

namespace
{

inline QString qstr(const std::string& s)
{
	return QString::fromStdString(s);
}

// QLabel with click/hover callbacks via virtual overrides (no Q_OBJECT).
class ClickableLabel : public QLabel
{
public:
	using QLabel::QLabel;
	std::function<void()> onClick;
	std::function<void()> onHover;

protected:
	void mousePressEvent(QMouseEvent* event) override
	{
		if (onClick)
			onClick();
		QLabel::mousePressEvent(event);
	}
	void enterEvent(QEnterEvent* event) override
	{
		if (onHover)
			onHover();
		QLabel::enterEvent(event);
	}
};

// A QLabel that remembers the picture it was given.
//
// It has to: a QLabel scales its pixmap only through setScaledContents(),
// which is exactly ScaleMode::Stretch and nothing else, so every other mode
// needs the picture COMPOSED against the frame -- and composing repeatedly
// from an already-composed pixmap would lose whatever the last crop discarded.
//
// The frame arrives through ControlWrapper::placed(), the same hook the wx
// twin uses: realize() runs before the engine has decided anything, and a
// resizeEvent would be one backend learning it a different way.
class ScaledImageLabel : public ClickableLabel
{
public:
	using ClickableLabel::ClickableLabel;

	QPixmap source;
	ScaleMode mode = ScaleMode::Stretch;

	// Guarded on the size it last composed at, so a relayout that did not move
	// this picture costs no scaling -- and setPixmap's updateGeometry() can
	// never drive a second pass.
	void composeFor(const QSize& box)
	{
		if (source.isNull() || box.isEmpty() || box == m_composed)
			return;
		m_composed = box;

		const Rect target = scaledImageRect(mode,
			Size { source.width(), source.height() },
			Size { box.width(), box.height() });

		// Everything the mode does not cover stays transparent, so a Fit
		// letterbox shows the parent through it exactly as wx's does.
		QPixmap canvas(box);
		canvas.fill(Qt::transparent);
		QPainter painter(&canvas);
		painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
		// Clipped by the canvas on every side, which IS the crop for Fill and
		// Center: the target rect is deliberately allowed to overflow.
		painter.drawPixmap(QRect(target.x, target.y, target.width, target.height), source);
		painter.end();
		setPixmap(canvas);
	}

private:
	QSize m_composed;
};

// The Splitter's sash: a sunken line the user drags, with the mouse handling
// done by virtual overrides (no Q_OBJECT). Deliberately not a QSplitter, which
// would own its children's geometry and fight the layout engine.
class SashFrame : public QFrame
{
public:
	using QFrame::QFrame;

	SplitterState* state = nullptr;
	bool horizontal = true;

	// QFrame's own hint for a bare line is not the 6 px the engine budgeted, and
	// the cross axis must ask for nothing -- the engine stretches the sash
	// across both panes there.
	QSize sizeHint() const override
	{
		return horizontal ? QSize(SplitterState::kSashThickness, 0)
						  : QSize(0, SplitterState::kSashThickness);
	}

protected:
	// The drag anchors on the position arrange RESOLVED when the gesture began,
	// so a long drag cannot accumulate rounding the way a per-event delta would,
	// and global coordinates are used because the frame itself moves underneath
	// the pointer as the layout follows it.
	void mousePressEvent(QMouseEvent* event) override
	{
		if (event->button() == Qt::LeftButton && state != nullptr)
		{
			m_anchorScreen = globalMain(event);
			m_anchorPos = state->resolved;
			m_dragging = true;
		}
		QFrame::mousePressEvent(event);
	}

	void mouseMoveEvent(QMouseEvent* event) override
	{
		if (m_dragging && state != nullptr)
			state->position.set(std::clamp(m_anchorPos + globalMain(event) - m_anchorScreen,
				state->lowerBound, state->upperBound));
		QFrame::mouseMoveEvent(event);
	}

	void mouseReleaseEvent(QMouseEvent* event) override
	{
		m_dragging = false;
		QFrame::mouseReleaseEvent(event);
	}

private:
	int globalMain(const QMouseEvent* event) const
	{
		const QPointF global = event->globalPosition();
		return static_cast<int>(horizontal ? global.x() : global.y());
	}

	int m_anchorScreen = 0;
	int m_anchorPos = 0;
	bool m_dragging = false;
};

} // unnamed namespace

// ButtonWrapper -----------------------------------------------------------

void ButtonWrapper::realize(void* parentWindow)
{
	auto* button = new QPushButton(qtLabelText(m_label), static_cast<QWidget*>(parentWindow));
	// QPushButton is autoDefault inside a QDialog, so the first one built would come
	// up drawn as the dialog's default button (blue on macOS) and keep that highlight
	// for the life of the dialog -- and a focused autoDefault button clicks ITSELF on
	// Enter. Off, Enter always reaches the window's filter, which presses the button
	// that asked to be the default (DialogKeys.hpp); setDefault() is then only its look.
	button->setAutoDefault(false);
	qt_dialog_keys::markButton(button, m_dialogKeys);
	if (!m_iconPath.empty())
	{
		const QPixmap pixmap(qstr(m_iconPath));
		if (!pixmap.isNull())
		{
			button->setIcon(QIcon(pixmap));
			button->setIconSize(QSize(m_iconSize.width, m_iconSize.height));
		}
#ifdef USE_LOGGER
		else
		{
			Logger::instance().log("ButtonWrapper::realize()\t-> icon \""
				+ m_iconPath + "\" failed to load; text only\n");
		}
#endif
	}
	if ((m_dialogKeys & kDefaultButton) != 0)
		button->setDefault(true);
	m_nativeWidget = button;

	if (m_onClick)
		QObject::connect(button, &QPushButton::clicked, [cb = std::move(m_onClick), nw = m_nativeWidget] { cb(nw); });

}

// TextCtrlWrapper -----------------------------------------------------------

void TextCtrlWrapper::realize(void* parentWindow)
{
	const std::string& initial = m_value.get();
	auto* edit = new QLineEdit(qstr(initial), static_cast<QWidget*>(parentWindow));
	// QLineEdit's sizeHint is a fixed character count, so the hint text cannot
	// reach the layout here -- no pinning needed, unlike wx.
	if (!m_placeholder.empty())
		edit->setPlaceholderText(qstr(m_placeholder));
	m_nativeWidget = edit;

	QObject::connect(edit, &QLineEdit::textChanged,
		[commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](const QString& text) { commit(text.toStdString()); });
	// QLineEdit emits returnPressed and then IGNORES the key, so it travels on
	// to the window's filter and the default button is pressed after this.
	if (m_onEnter)
		QObject::connect(edit, &QLineEdit::returnPressed,
			[edit, cb = std::move(m_onEnter)] { cb(edit->text().toStdString(), edit); });
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		bindExternalRefSync(edit,
			[edit] { return edit->text().toStdString(); },
			[&value] { return value; },
			[edit](const std::string& v) { edit->setText(qstr(v)); });
	}

	qt_text_field::apply(edit, std::move(m_field));
}

// PasswordInputWrapper -----------------------------------------------------------

void PasswordInputWrapper::realize(void* parentWindow)
{
	const std::string& initial = m_value.get();
	auto* edit = new QLineEdit(qstr(initial), static_cast<QWidget*>(parentWindow));
	edit->setEchoMode(QLineEdit::Password);
	if (!m_placeholder.empty())
		edit->setPlaceholderText(qstr(m_placeholder));
	m_nativeWidget = edit;

	QObject::connect(edit, &QLineEdit::textChanged,
		[commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](const QString& text) { commit(text.toStdString()); });
	// QLineEdit emits returnPressed and then IGNORES the key, so it travels on
	// to the window's filter and the default button is pressed after this.
	if (m_onEnter)
		QObject::connect(edit, &QLineEdit::returnPressed,
			[edit, cb = std::move(m_onEnter)] { cb(edit->text().toStdString(), edit); });
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		bindExternalRefSync(edit,
			[edit] { return edit->text().toStdString(); },
			[&value] { return value; },
			[edit](const std::string& v) { edit->setText(qstr(v)); });
	}

	qt_text_field::apply(edit, std::move(m_field));
}

// SearchFieldWrapper -----------------------------------------------------------

namespace
{

// A QLineEdit that keeps Enter. QLineEdit emits returnPressed and then
// IGNORES the key, which would carry it on to the window's filter and press
// the default button (DialogKeys.hpp); a search field's Enter is its own, so
// it is accepted here instead. A virtual override, no Q_OBJECT.
class SearchLineEdit : public QLineEdit
{
public:
	using QLineEdit::QLineEdit;

	std::function<void()> onReturn;

protected:
	void keyPressEvent(QKeyEvent* event) override
	{
		const bool enter = event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter;
		if (enter && (event->modifiers() & ~Qt::KeypadModifier) == Qt::NoModifier)
		{
			if (onReturn)
				onReturn();
			event->accept();
			return;
		}
		QLineEdit::keyPressEvent(event);
	}
};

// Qt ships no portable search icon (QIcon::fromTheme is Linux-only in
// practice), so the magnifier is painted -- in the field's own text colour,
// dimmed, the way the placeholder is.
QIcon magnifierIcon(const QWidget* field)
{
	const qreal dpr = field->devicePixelRatioF();
	QPixmap pixmap(QSize(16, 16) * dpr);
	pixmap.setDevicePixelRatio(dpr);
	pixmap.fill(Qt::transparent);
	QPainter painter(&pixmap);
	painter.setRenderHint(QPainter::Antialiasing);
	QColor colour = field->palette().color(QPalette::Text);
	colour.setAlphaF(0.55);
	painter.setPen(QPen(colour, 1.6));
	painter.drawEllipse(QRectF(2.5, 2.5, 8.0, 8.0));
	painter.drawLine(QPointF(9.5, 9.5), QPointF(13.5, 13.5));
	return QIcon(pixmap);
}

} // unnamed namespace

void SearchFieldWrapper::realize(void* parentWindow)
{
	auto* edit = new SearchLineEdit(qstr(m_value.get()), static_cast<QWidget*>(parentWindow));
	// Neither the icon nor the hint changes QLineEdit's sizeHint (a fixed
	// character count), so nothing here can resize an auto-fit window.
	edit->setClearButtonEnabled(true);
	edit->addAction(magnifierIcon(edit), QLineEdit::LeadingPosition);
	if (!m_placeholder.empty())
		edit->setPlaceholderText(qstr(m_placeholder));
	m_nativeWidget = edit;

	QObject::connect(edit, &QLineEdit::textChanged,
		[commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](const QString& text) { commit(text.toStdString()); });
	if (m_onSearch)
		edit->onReturn = [edit, cb = std::move(m_onSearch)] { cb(edit->text().toStdString(), edit); };
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		bindExternalRefSync(edit,
			[edit] { return edit->text().toStdString(); },
			[&value] { return value; },
			[edit](const std::string& v) { edit->setText(qstr(v)); });
	}
}

// MultiLineTextCtrlWrapper -----------------------------------------------------------

void MultiLineTextCtrlWrapper::realize(void* parentWindow)
{
	const std::string& initial = m_value.get();
	auto* edit = new QPlainTextEdit(qstr(initial), static_cast<QWidget*>(parentWindow));
	m_nativeWidget = edit;

	QObject::connect(edit, &QPlainTextEdit::textChanged,
		[edit, commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)] { commit(edit->toPlainText().toStdString()); });
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		bindExternalRefSync(edit,
			[edit] { return edit->toPlainText().toStdString(); },
			[&value] { return value; },
			[edit](const std::string& v) { edit->setPlainText(qstr(v)); });
	}

	qt_text_field::apply(edit, std::move(m_field));
}

// ReadonlyTextCtrlWrapper -----------------------------------------------------------

void ReadonlyTextCtrlWrapper::realize(void* parentWindow)
{
	auto* edit = new QLineEdit(qstr(m_value.get()), static_cast<QWidget*>(parentWindow));
	edit->setReadOnly(true);
	m_nativeWidget = edit;

	// Bound: the field follows the caller's string.
	if (const std::string* bound = m_value.boundValue())
	{
		bindExternalRefSync(edit,
			[edit] { return edit->text().toStdString(); },
			[bound] { return *bound; },
			[edit](const std::string& v) { edit->setText(qstr(v)); });
	}
}

// ClickableTextWrapper -----------------------------------------------------------

void ClickableTextWrapper::realize(void* parentWindow)
{
	auto* label = new ClickableLabel(static_cast<QWidget*>(parentWindow));
	qtSetPlainText(label, m_text);
	m_nativeWidget = label;
	if (m_onClick)
		label->onClick = [cb = std::move(m_onClick), nw = m_nativeWidget] { cb(nw); };

}

// LinkTextWrapper -----------------------------------------------------------

void LinkTextWrapper::realize(void* parentWindow)
{
	auto* label = new QLabel(
		QStringLiteral("<a href=\"#\">%1</a>").arg(qstr(m_text).toHtmlEscaped()),
		static_cast<QWidget*>(parentWindow));
	label->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
	m_nativeWidget = label;

	if (m_onClick)
		QObject::connect(label, &QLabel::linkActivated, [cb = std::move(m_onClick), nw = m_nativeWidget](const QString&) { cb(nw); });

}

// StaticTextWrapper -----------------------------------------------------------

void StaticTextWrapper::realize(void* parentWindow)
{
	auto* label = new QLabel(static_cast<QWidget*>(parentWindow));
	qtSetPlainText(label, m_text.get());
	// AlignVCenter is QLabel's own default and is kept, so a Left label reads
	// exactly as it always did; only the horizontal half follows withAlign().
	const Qt::Alignment horizontal = m_align == TextAlign::Center ? Qt::AlignHCenter
		: m_align == TextAlign::Right ? Qt::AlignRight
		: Qt::AlignLeft;
	label->setAlignment(horizontal | Qt::AlignVCenter);
	m_nativeWidget = label;
	const QSize hint = label->sizeHint();
	m_initialSize = Size { hint.width(), hint.height() };

	// Bound: the label follows the caller's string. Its sizeHint would follow
	// too, which is why a bound label answers measure itself (the first
	// text's size) instead; a longer text is clipped by the label's geometry.
	if (const std::string* bound = m_text.boundValue())
	{
		bindExternalRefSync(label,
			[label] { return label->text().toStdString(); },
			[bound] { return *bound; },
			[label](const std::string& text) { label->setText(qstr(text)); });
	}
}

Size StaticTextWrapper::measureIntrinsic(const Constraints&)
{
	// Bound labels only (measuresItself): the size of the first text.
	return m_initialSize;
}

// RichTextWrapper -----------------------------------------------------------

void RichTextWrapper::realize(void* parentWindow)
{
	auto* view = new RichTextView(static_cast<QWidget*>(parentWindow), m_runs);
	m_nativeWidget = view;

	if (m_onLink)
		view->setOnLink([cb = std::move(m_onLink), nw = m_nativeWidget](const std::string& url) { cb(url, nw); });
}

Size RichTextWrapper::measureIntrinsic(const Constraints& c)
{
	const auto* view = static_cast<const RichTextView*>(m_nativeWidget);
	if (view == nullptr)
		return Size { 0, 0 };
	const RichTextLayout& layout = view->layoutFor(wrapWidth(c));
	return Size { layout.width, layout.height };
}

// DatePickerWrapper -----------------------------------------------------------

void DatePickerWrapper::realize(void* parentWindow)
{
	const Date& initial = m_value.get();
	auto* picker = new QDateEdit(QDate(initial.year, initial.month, initial.day), static_cast<QWidget*>(parentWindow));
	picker->setCalendarPopup(true);
	m_nativeWidget = picker;

	auto toDate = [](const QDate& d) { return Date { d.year(), d.month(), d.day() }; };
	QObject::connect(picker, &QDateEdit::dateChanged,
		[toDate, commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](QDate d) { commit(toDate(d)); });
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		bindExternalRefSync(picker,
			[picker] { return picker->date(); },
			[&value] { return QDate(value.year, value.month, value.day); },
			[picker](const QDate& d) { picker->setDate(d); });
	}

}

// TimePickerWrapper -----------------------------------------------------------

void TimePickerWrapper::realize(void* parentWindow)
{
	const Time& initial = m_value.get();
	auto* picker = new QTimeEdit(QTime(initial.hour, initial.minute, initial.second), static_cast<QWidget*>(parentWindow));
	picker->setDisplayFormat(QStringLiteral("HH:mm:ss"));
	m_nativeWidget = picker;

	auto toTime = [](QTime t) { return Time { t.hour(), t.minute(), t.second() }; };
	QObject::connect(picker, &QTimeEdit::timeChanged,
		[toTime, commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](QTime t) { commit(toTime(t)); });
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		bindExternalRefSync(picker,
			[picker] { return picker->time(); },
			[&value] { return QTime(value.hour, value.minute, value.second); },
			[picker](const QTime& t) { picker->setTime(t); });
	}

}

// SliderWrapper -----------------------------------------------------------

template <SliderValue T>
void SliderWrapper<T>::realize(void* parentWindow)
{
	const T& initial = m_value.get();
	auto* slider = new QSlider(Qt::Horizontal, static_cast<QWidget*>(parentWindow));
	if constexpr (std::is_floating_point_v<T>)
	{
		slider->setRange(static_cast<int>(m_range.min / m_range.step),
			static_cast<int>(m_range.max / m_range.step));
		slider->setValue(static_cast<int>(initial / m_range.step));
	}
	else
	{
		slider->setRange(m_range.min, m_range.max);
		slider->setValue(static_cast<int>(initial));
	}
	m_nativeWidget = slider;

	auto toValue = [step = m_range.step](int raw) {
		if constexpr (std::is_floating_point_v<T>)
			return static_cast<T>(raw) * step;
		else
			return static_cast<T>(raw);
	};
	QObject::connect(slider, &QSlider::valueChanged,
		[toValue, commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](int raw) { commit(toValue(raw)); });
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		// QSlider is integral; a float slider lives in step units, so compare there.
		bindExternalRefSync(slider,
			[slider] { return slider->value(); },
			[&value, step = m_range.step] {
				if constexpr (std::is_floating_point_v<T>)
					return static_cast<int>(value / step);
				else
					return static_cast<int>(value);
			},
			[slider](int raw) { slider->setValue(raw); });
	}

}

template class SliderWrapper<int>;
template class SliderWrapper<float>;

// SpinBoxWrapper -----------------------------------------------------------

template <SpinBoxValue T>
void SpinBoxWrapper<T>::realize(void* parentWindow)
{
	const T& initial = m_value.get();
	if constexpr (std::is_same_v<T, int>)
	{
		auto* spin = new QSpinBox(static_cast<QWidget*>(parentWindow));
		spin->setRange(m_range.min, m_range.max);
		spin->setSingleStep(static_cast<int>(m_range.step));
		spin->setValue(initial);
		m_nativeWidget = spin;

		QObject::connect(spin, &QSpinBox::valueChanged, [commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](int v) { commit(v); });
		if (m_value.isBound())
		{
			auto& value = m_value.get();
			bindExternalRefSync(spin,
				[spin] { return spin->value(); },
				[&value] { return static_cast<int>(value); },
				[spin](int v) { spin->setValue(v); });
		}
	}
	else
	{
		auto* spin = new QDoubleSpinBox(static_cast<QWidget*>(parentWindow));
		spin->setRange(m_range.min, m_range.max);
		spin->setSingleStep(m_range.step);
		spin->setValue(initial);
		m_nativeWidget = spin;

		QObject::connect(spin, &QDoubleSpinBox::valueChanged,
			[commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](double v) { commit(static_cast<T>(v)); });
		if (m_value.isBound())
		{
			auto& value = m_value.get();
			// Quantise to step units: QDoubleSpinBox rounds what it stores to its
			// decimals() setting, so a raw double compare would never settle.
			bindExternalRefSync(spin,
				[spin, step = m_range.step] { return std::lround(spin->value() / step); },
				[&value, step = m_range.step] { return std::lround(value / step); },
				[spin, step = m_range.step](long units) { spin->setValue(static_cast<double>(units) * step); });
		}
	}

}

template class SpinBoxWrapper<int>;
template class SpinBoxWrapper<float>;

// RadioButtonWrapper -----------------------------------------------------------

namespace
{
// A radio that belongs to no native group. Qt makes sibling radios
// auto-exclusive by parent widget, and every leaf here is parented flat to its
// dialog or page -- so two groups in one box would merge natively. The bound
// int is the group instead (see RadioButtonWrapper).
//
// Non-exclusive, a QRadioButton would toggle OFF when clicked while checked,
// which no radio does. nextCheckState() is what a click calls, so overriding
// it to only ever check is the whole fix -- a virtual override, no Q_OBJECT.
class OptionRadioButton : public QRadioButton
{
public:
	using QRadioButton::QRadioButton;

protected:
	void nextCheckState() override
	{
		if (!isChecked())
			setChecked(true);
	}
};
} // unnamed namespace

template <RadioButtonValue T>
void RadioButtonWrapper<T>::realize(void* parentWindow)
{
	auto* radio = new OptionRadioButton(qtLabelText(m_label), static_cast<QWidget*>(parentWindow));
	radio->setAutoExclusive(false);
	radio->setChecked(isChecked(m_value.get(), m_option));
	m_nativeWidget = radio;

	// toggled(true) is connected only after the initial setChecked above, so
	// the starting state is not read as a pick.
	QObject::connect(radio, &QRadioButton::toggled,
		[choice = picked(m_option), commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](bool checked) {
			if (checked)
				commit(choice);
		});
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		// Every radio on the int mirrors it: the one just picked is already
		// checked, and the rest see the int move away from their option and
		// uncheck themselves. The push runs under RefSync's QSignalBlocker, so
		// an uncheck never reaches the handler above.
		bindExternalRefSync(radio,
			[radio] { return radio->isChecked(); },
			[&value, option = m_option] { return isChecked(value, option); },
			[radio](bool on) { radio->setChecked(on); });
	}
}

template class RadioButtonWrapper<bool>;
template class RadioButtonWrapper<int>;

// CheckBoxWrapper -----------------------------------------------------------

void CheckBoxWrapper::realize(void* parentWindow)
{
	const bool checked = m_value.get();
	auto* box = new QCheckBox(qtLabelText(m_label), static_cast<QWidget*>(parentWindow));
	box->setChecked(checked);
	m_nativeWidget = box;

	QObject::connect(box, &QCheckBox::toggled,
		[commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](bool v) { commit(v); });
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		bindExternalRefSync(box,
			[box] { return box->isChecked(); },
			[&value] { return value; },
			[box](bool on) { box->setChecked(on); });
	}

}

// ToggleButtonWrapper -----------------------------------------------------------

void ToggleButtonWrapper::realize(void* parentWindow)
{
	const bool toggled = m_value.get();
	auto* button = new QPushButton(qtLabelText(m_label), static_cast<QWidget*>(parentWindow));
	button->setCheckable(true);
	button->setChecked(toggled);
	button->setAutoDefault(false);
	m_nativeWidget = button;

	QObject::connect(button, &QPushButton::toggled,
		[commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](bool v) { commit(v); });
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		bindExternalRefSync(button,
			[button] { return button->isChecked(); },
			[&value] { return value; },
			[button](bool on) { button->setChecked(on); });
	}

}

// ImageWrapper -----------------------------------------------------------

void ImageWrapper::realize(void* parentWindow)
{
	auto* label = new ScaledImageLabel(static_cast<QWidget*>(parentWindow));
	QPixmap pixmap(qstr(m_filePath));
	if (!pixmap.isNull())
	{
		label->source = pixmap;
		label->mode = m_scaleMode;
		// The ORIGINAL goes in so sizeHint() reports the natural size, which is
		// what the measure pass reads; an explicit withSize() overrides it per
		// axis in measureContent(). The composed picture replaces it the moment
		// the engine places the label.
		label->setPixmap(pixmap);
	}
	else
	{
		label->setText(QStringLiteral("[Image: failed to load]"));
	}
	m_nativeWidget = label;

	if (m_onClick)
		label->onClick = [cb = std::move(m_onClick), nw = m_nativeWidget] { cb(nw); };
	if (m_onHover)
		label->onHover = [cb = std::move(m_onHover), nw = m_nativeWidget] { cb(nw); };

}

void ImageWrapper::placed(const Rect& frame)
{
	// static_cast, not qobject_cast: there is no Q_OBJECT anywhere in this
	// backend, and realize() creates a ScaledImageLabel unconditionally -- a
	// picture that failed to load is one holding a null source, not a QLabel.
	if (auto* label = static_cast<ScaledImageLabel*>(m_nativeWidget))
		label->composeFor(QSize(frame.width, frame.height));
}

// ToolBarWrapper -----------------------------------------------------------

void ToolBarWrapper::realize(void* parentWindow)
{
	// A CHILD QToolBar, deliberately not QMainWindow::addToolBar(): that one
	// docks itself into the window's chrome and would be invisible to the
	// engine, and it would make a toolbar impossible inside a Dialog.
	auto* bar = new QToolBar(static_cast<QWidget*>(parentWindow));
	bar->setIconSize(QSize(m_iconSize.width, m_iconSize.height));
	bar->setMovable(false);
	bar->setFloatable(false);
	m_nativeWidget = bar;

	const bool anyLabels = m_labelsForced
		|| std::any_of(m_tools.begin(), m_tools.end(), [](const ToolItem& tool) {
			return !tool.isSeparator && tool.iconPath.empty();
		});
	bar->setToolButtonStyle(anyLabels
		? Qt::ToolButtonTextBesideIcon
		: Qt::ToolButtonIconOnly);

	for (ToolItem& tool : m_tools)
	{
		if (tool.isSeparator)
		{
			bar->addSeparator();
			continue;
		}

		// A tool with no usable icon shows its label instead, so the row is
		// never blank.
		QIcon icon;
		if (!tool.iconPath.empty())
		{
			const QPixmap pixmap(qstr(tool.iconPath));
			if (!pixmap.isNull())
				icon = QIcon(pixmap);
#ifdef USE_LOGGER
			else
				Logger::instance().log("ToolBarWrapper::realize()\t-> icon \""
					+ tool.iconPath + "\" failed to load; falling back to the label\n");
#endif
		}

		QAction* action = icon.isNull()
			? bar->addAction(qtLabelText(tool.label))
			: bar->addAction(icon, qtLabelText(tool.label));
		if (!tool.tooltip.empty())
			action->setToolTip(qstr(tool.tooltip));

		ToolItem* model = &tool;
		if (model->toggledFlag)
		{
			action->setCheckable(true);
			action->setChecked(model->toggledFlag->get());
		}
		action->setEnabled(!model->disabledFlag.get());

		QObject::connect(action, &QAction::triggered, bar, [model, action](bool) {
			// Value first, then the callback.
			if (model->toggledFlag)
				model->toggledFlag->set(action->isChecked());
			if (model->clickHandler)
				model->clickHandler();
		});

		// Bound flags are polled, never pushed on disagreement. setChecked()
		// emits toggled(), not triggered(), so a mirrored write cannot re-enter
		// the handler above.
		if (model->toggledFlag && model->toggledFlag->isBound())
		{
			const bool* flag = model->toggledFlag->boundValue();
			bindExternalRefSync(bar,
				[action] { return action->isChecked(); },
				[flag] { return *flag; },
				[action](bool value) { action->setChecked(value); });
		}
		if (model->disabledFlag.isBound())
		{
			const bool* flag = model->disabledFlag.boundValue();
			bindExternalRefSync(bar,
				[action] { return action->isEnabled(); },
				[flag] { return !*flag; },
				[action](bool enabled) { action->setEnabled(enabled); });
		}
	}
}

// StatusBarWrapper -----------------------------------------------------------

void StatusBarWrapper::realize(void* parentWindow)
{
	// A CHILD QStatusBar, deliberately not QMainWindow::setStatusBar(): that one
	// docks itself into the window's chrome, out of the engine's sight.
	auto* bar = new QStatusBar(static_cast<QWidget*>(parentWindow));
	bar->setSizeGripEnabled(false);
	m_nativeWidget = bar;

	for (StatusField& field : m_fields)
	{
		auto* label = new QLabel(bar);
		qtSetPlainText(label, field.text.get());
		if (field.width > 0)
		{
			label->setFixedWidth(field.width);
			bar->addWidget(label, 0); // fixed: no share of the leftover
		}
		else
		{
			// Stretch fields share what the fixed ones leave over, equally.
			//
			// Ignored horizontally on purpose: a QLabel's sizeHint follows its
			// text, so without this an arriving status message would widen the
			// bar's sizeHint and, through it, an auto-fit window. The floor
			// below is what the bar measures instead -- content-independent,
			// like wx's own best size and like the ImGui row.
			label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
			label->setMinimumWidth(kDefaultStatusFieldWidth);
			bar->addWidget(label, 1);
		}

		// A bound field is written from elsewhere and Qt has no notification
		// for that, so it is polled like any other external ref.
		if (field.text.isBound())
		{
			const std::string* bound = field.text.boundValue();
			bindExternalRefSync(label,
				[label] { return label->text().toStdString(); },
				[bound] { return *bound; },
				[label](const std::string& text) { label->setText(qstr(text)); });
		}
	}
}

Size StatusBarWrapper::measureIntrinsic(const Constraints&)
{
	const auto* bar = static_cast<const QStatusBar*>(m_nativeWidget);
	return Size { statusBarContentWidth(m_fields), bar != nullptr ? bar->sizeHint().height() : 0 };
}

// ColorPickerWrapper -----------------------------------------------------------

void ColorPickerWrapper::realize(void* parentWindow)
{
	const Color initial = m_value.get();
	auto* button = new QPushButton(static_cast<QWidget*>(parentWindow));
	button->setAutoDefault(false);
	auto sheetFor = [](const Color& c) {
		return QStringLiteral("background-color: rgba(%1,%2,%3,%4);")
			.arg((int)(c.r * 255)).arg((int)(c.g * 255)).arg((int)(c.b * 255)).arg((int)(c.a * 255));
	};
	auto applySwatch = [button, sheetFor](const Color& c) { button->setStyleSheet(sheetFor(c)); };
	applySwatch(initial);
	m_nativeWidget = button;

	// There is no getter for the swatch colour -- the stylesheet is the displayed
	// state, so compare on that. Byte-quantised, so no float equality either.
	if (m_value.isBound())
	{
		auto& swatchValue = m_value.get();
		bindExternalRefSync(button,
			[button] { return button->styleSheet(); },
			[&swatchValue, sheetFor] { return sheetFor(swatchValue); },
			[button](const QString& sheet) { button->setStyleSheet(sheet); });
	}

	// the button is the swatch; clicking opens the native color dialog
	// The bound variable outlives the wrapper, so the handler may hold a pointer to
	// it; a snapshot cannot be written back, hence the null and the `initial` copy.
	Color* bound = m_value.isBound() ? &m_value.get() : nullptr;
	QObject::connect(button, &QPushButton::clicked,
		[button, applySwatch, bound, initial,
			cb = std::move(m_onChange), nw = m_nativeWidget]() {
			const Color current = bound ? *bound : initial;
			const QColor start((int)(current.r * 255), (int)(current.g * 255),
				(int)(current.b * 255), (int)(current.a * 255));
			const QColor picked = QColorDialog::getColor(start, button, QString(),
				QColorDialog::ShowAlphaChannel);
			if (!picked.isValid())
				return;
			const Color color { (float)picked.redF(), (float)picked.greenF(),
				(float)picked.blueF(), (float)picked.alphaF() };
			if (bound)
				*bound = color;
			applySwatch(color);
			cb(color, nw);
		});

}

// FilePickerWrapper -----------------------------------------------------------

void FilePickerWrapper::realize(void* parentWindow)
{
	// Qt has no picker control, so the composite is built by hand: a field and
	// a Browse button in one QWidget, which is the single leaf the engine sees.
	// The QHBoxLayout is what makes place()'s setGeometry re-lay the two halves
	// -- the engine sizes the composite and Qt distributes inside it.
	auto* composite = new QWidget(static_cast<QWidget*>(parentWindow));
	auto* row = new QHBoxLayout(composite);
	row->setContentsMargins(0, 0, 0, 0);
	row->setSpacing(4);

	auto* edit = new QLineEdit(qstr(m_value.get()), composite);
	auto* browse = new QToolButton(composite);
	browse->setText(QStringLiteral("..."));
	browse->setFocusPolicy(Qt::TabFocus);
	row->addWidget(edit, 1);
	row->addWidget(browse, 0);
	m_nativeWidget = composite;

	// One commit path for both halves: typing and picking both end in
	// commitTo(), which writes the bound string (when there is one) and then
	// reports. That is what makes a typed path and a picked one
	// indistinguishable downstream (R11.4).
	//
	// textEdited, not textChanged: it fires for user typing only, so the
	// dialog's own setText below does not report the pick a second time.
	QObject::connect(edit, &QLineEdit::textEdited,
		[commit = commitTo(m_value, m_onChange, m_nativeWidget)](const QString& text) {
			commit(text.toStdString());
		});
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		bindExternalRefSync(edit,
			[edit] { return edit->text().toStdString(); },
			[&value] { return value; },
			[edit](const std::string& v) { edit->setText(qstr(v)); });
	}

	// The dialog leg: the field shows the pick, then the pick is committed.
	// Everything it needs is captured by value -- the wrapper is not, since its
	// teardown order against the widget is not fixed.
	QObject::connect(browse, &QToolButton::clicked,
		[composite, edit, mode = m_mode, filters = m_filters, title = m_dialogTitle,
			commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)]() {
			const std::string current = edit->text().toStdString();
			const std::string chosen = qtRunFileDialog(composite, title, mode, filters, current);
			if (chosen.empty())
				return; // cancel leaves the path alone -- it is not a selection of ""
			edit->setText(qstr(chosen));
			commit(chosen);
		});
}

// SeparatorWrapper -----------------------------------------------------------

void SeparatorWrapper::realize(void* parentWindow)
{
	auto* line = new QFrame(static_cast<QWidget*>(parentWindow));
	line->setFrameShape(m_orient == Orientation::Vertical ? QFrame::VLine : QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
	m_nativeWidget = line;

}

// SplitterSashWrapper -----------------------------------------------------------

void SplitterSashWrapper::realize(void* parentWindow)
{
	const bool horizontal = m_state->orientation == Orientation::Horizontal;

	auto* sash = new SashFrame(static_cast<QWidget*>(parentWindow));
	sash->state = m_state;
	sash->horizontal = horizontal;
	sash->setFrameShape(horizontal ? QFrame::VLine : QFrame::HLine);
	sash->setFrameShadow(QFrame::Sunken);
	sash->setCursor(horizontal ? Qt::SplitHCursor : Qt::SplitVCursor);
	m_nativeWidget = sash;

}

// ExpanderHeaderWrapper -----------------------------------------------------------

void ExpanderHeaderWrapper::realize(void* parentWindow)
{
	// Qt has no collapsible-header control, so the header is a checkable tool
	// button with an arrow beside its text -- the shape QTreeView section
	// headers and every Qt settings dialog use, and the closest thing to
	// wxCollapsibleHeaderCtrl that needs no painting of our own.
	auto* header = new QToolButton(static_cast<QWidget*>(parentWindow));
	header->setText(qtLabelText(m_label));
	header->setCheckable(true);
	header->setChecked(m_state->expanded.get());
	header->setAutoRaise(true);
	header->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	header->setArrowType(m_state->expanded.get() ? Qt::DownArrow : Qt::RightArrow);
	m_nativeWidget = header;

	// The ExpanderState lives in the node tree, which the engine session owns
	// and which outlives every window here; the wrapper must never be captured.
	ExpanderState* state = m_state;
	QObject::connect(header, &QToolButton::toggled, header, [header, state](bool open) {
		header->setArrowType(open ? Qt::DownArrow : Qt::RightArrow);
		state->expanded.set(open);
	});

	// A bound flag can be written from anywhere, and the button applies its own
	// state only when clicked -- so it is mirrored like any other external ref.
	// The relayout that follows is armed separately, by the session's
	// poll: this only keeps the header itself honest.
	//
	// The arrow is set HERE as well as in the toggled handler above, and has to
	// be: every RefSync push runs under a QSignalBlocker, so a programmatic
	// setChecked() deliberately does not re-enter that handler.
	if (m_state->expanded.isBound())
	{
		bindExternalRefSync(header,
			[header] { return header->isChecked(); },
			[state] { return state->expanded.get(); },
			[header](bool open) {
				header->setChecked(open);
				header->setArrowType(open ? Qt::DownArrow : Qt::RightArrow);
			});
	}

}

// ProgressBarWrapper -----------------------------------------------------------

void ProgressBarWrapper::realize(void* parentWindow)
{
	// The bound float is a 0..100 percentage, matching the bar's own integer range
	// (and what the wx and ImGui backends do -- Qt used to read it as a 0..1
	// fraction, which rendered the same tree differently here).
	const auto toBar = [](float v) { return static_cast<int>(std::clamp(v, 0.0f, 100.0f)); };

	const float initial = m_value.get();
	auto* bar = new QProgressBar(static_cast<QWidget*>(parentWindow));
	m_nativeWidget = bar;

	if (m_indeterminate)
	{
		// An empty range is Qt's busy indicator: the bar animates itself, draws no
		// percentage and ignores setValue(), so there is no value to mirror and
		// nothing to bind.
		bar->setRange(0, 0);
		return;
	}

	bar->setRange(0, 100);
	bar->setValue(toBar(initial));

	// A progress bar has no input of its own -- the bound float is only ever
	// written from outside -- so the sync is the whole story here.
	if (m_value.isBound())
	{
		bindExternalRefSync(bar,
			[bar] { return bar->value(); },
			[&value = m_value.get(), toBar] { return toBar(value); },
			[bar](int v) { bar->setValue(v); });
	}

}

// ComboBoxWrapper -----------------------------------------------------------

namespace
{

ItemList comboItems(const QComboBox* combo)
{
	ItemList items;
	items.reserve(combo->count());
	for (int i = 0; i < combo->count(); ++i)
		items.push_back(combo->itemText(i).toStdString());
	return items;
}

void setComboItems(QComboBox* combo, const ItemList& items)
{
	combo->clear();
	for (const auto& item : items)
		combo->addItem(qstr(item));
}

} // unnamed namespace

template <ComboBoxValue T>
void ComboBoxWrapper<T>::realize(void* parentWindow)
{
	auto* combo = new QComboBox(static_cast<QWidget*>(parentWindow));
	setComboItems(combo, m_choices.get());
	const auto select = [combo](const T& value) {
		if constexpr (std::is_same_v<T, std::string>)
			combo->setCurrentIndex(combo->findText(qstr(value)));
		else
			combo->setCurrentIndex(value >= 0 && value < combo->count() ? value : -1);
	};
	const auto current = [combo]() -> T {
		if constexpr (std::is_same_v<T, std::string>)
			return combo->currentText().toStdString();
		else
			return combo->currentIndex();
	};
	select(m_value.get());
	m_nativeWidget = combo;
	const QSize hint = combo->sizeHint();
	m_initialSize = Size { hint.width(), hint.height() };

	// currentIndexChanged for both spellings: it is the one signal a pick
	// always raises, and the RefSync pushes below run under a QSignalBlocker.
	QObject::connect(combo, &QComboBox::currentIndexChanged,
		[current, commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](int) { commit(current()); });

	// Bound choices: repopulate when the caller's vector changes, keeping the
	// selection by value -- the bound one when there is one, else whatever was
	// picked. Registered before the selection sync, so a tick that changes
	// both sees the new list first.
	if (const ItemList* boundItems = m_choices.boundValue())
	{
		const T* boundValue = m_value.boundValue();
		bindWatchedRefSync(combo, watchRefs(boundItems),
			[combo] { return comboItems(combo); },
			[boundItems] { return *boundItems; },
			[combo, select, current, boundValue](const ItemList& items) {
				const T keep = boundValue != nullptr ? *boundValue : current();
				setComboItems(combo, items);
				select(keep);
			});
	}
	if (const T* boundValue = m_value.boundValue())
	{
		bindExternalRefSync(combo,
			current,
			[boundValue] { return *boundValue; },
			select);
	}
}

template <ComboBoxValue T>
Size ComboBoxWrapper<T>::measureIntrinsic(const Constraints&)
{
	return m_initialSize; // bound choices only (measuresItself)
}

template class ComboBoxWrapper<std::string>;
template class ComboBoxWrapper<int>;

// EditableComboWrapper -----------------------------------------------------------

void EditableComboWrapper::realize(void* parentWindow)
{
	auto* combo = new QComboBox(static_cast<QWidget*>(parentWindow));
	combo->setEditable(true);
	// NoInsert: by default an editable QComboBox appends the typed text to its
	// list on Enter, which wx and ImGui never do -- and the list is the
	// caller's. With nothing to insert, the key is ignored and travels on to
	// the window's default button (DialogKeys.hpp), as in a TextCtrl.
	combo->setInsertPolicy(QComboBox::NoInsert);
	setComboItems(combo, m_items.get());
	combo->setEditText(qstr(m_value.get()));
	if (!m_placeholder.empty())
		combo->lineEdit()->setPlaceholderText(qstr(m_placeholder));
	m_nativeWidget = combo;
	const QSize hint = combo->sizeHint();
	m_initialSize = Size { hint.width(), hint.height() };

	// editTextChanged covers both: typing, and a pick (which sets the text).
	QObject::connect(combo, &QComboBox::editTextChanged,
		[commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](const QString& text) { commit(text.toStdString()); });

	// Bound items: repopulate, keeping the text. Under RefSync's blocker.
	if (const ItemList* boundItems = m_items.boundValue())
	{
		bindWatchedRefSync(combo, watchRefs(boundItems),
			[combo] { return comboItems(combo); },
			[boundItems] { return *boundItems; },
			[combo](const ItemList& items) {
				const QString keep = combo->currentText();
				setComboItems(combo, items);
				combo->setEditText(keep);
			});
	}
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		bindExternalRefSync(combo,
			[combo] { return combo->currentText().toStdString(); },
			[&value] { return value; },
			[combo](const std::string& v) { combo->setEditText(qstr(v)); });
	}
}

Size EditableComboWrapper::measureIntrinsic(const Constraints&)
{
	return m_initialSize; // bound items only (measuresItself)
}

// ListBoxWrapper -----------------------------------------------------------

namespace
{

// QListWidget's own sizeHint is a fixed ~256x192 that ignores both the item text
// and the item count, so it would size the same tree differently from wx and
// ImGui. This one reports content width and visibleRows of height instead --
// a virtual override, no Q_OBJECT needed.
class SizedListWidget : public QListWidget
{
public:
	using QListWidget::QListWidget;

	int visibleRows = 1;

	QSize sizeHint() const override
	{
		const int rowHeight = count() > 0 ? sizeHintForRow(0) : fontMetrics().height();
		const int contentWidth = count() > 0 ? sizeHintForColumn(0) : 0;
		const int chrome = frameWidth() * 2;
		return QSize(contentWidth + chrome + style()->pixelMetric(QStyle::PM_ScrollBarExtent),
			rowHeight * visibleRows + chrome);
	}
};

std::vector<int> listWidgetSelection(const QListWidget* list)
{
	std::vector<int> indices;
	for (int i = 0; i < list->count(); ++i)
	{
		if (list->item(i)->isSelected())
			indices.push_back(i);
	}
	return indices;
}

void setListWidgetSelection(QListWidget* list, const std::vector<int>& indices)
{
	for (int i = 0; i < list->count(); ++i)
		list->item(i)->setSelected(std::find(indices.begin(), indices.end(), i) != indices.end());
}

} // unnamed namespace

namespace
{

ItemList listItems(const QListWidget* list)
{
	ItemList items;
	items.reserve(list->count());
	for (int i = 0; i < list->count(); ++i)
		items.push_back(list->item(i)->text().toStdString());
	return items;
}

} // unnamed namespace

template <ListBoxValue T>
void ListBoxWrapper<T>::realize(void* parentWindow)
{
	auto* list = new SizedListWidget(static_cast<QWidget*>(parentWindow));
	list->visibleRows = m_visibleRows;
	for (const auto& item : m_items.get())
		list->addItem(qstr(item));
	// ExtendedSelection is Qt's ctrl/shift-click mode; MultiSelection would
	// toggle on a plain click, which is not what a desktop list does.
	list->setSelectionMode(kMultiSelect
		? QAbstractItemView::ExtendedSelection
		: QAbstractItemView::SingleSelection);
	setListWidgetSelection(list, indicesFor(m_items.get(), boundValue()));
	m_nativeWidget = list;
	const QSize hint = list->sizeHint();
	m_initialSize = Size { hint.width(), hint.height() };

	const ItemsView items(m_items);
	QObject::connect(list, &QListWidget::itemSelectionChanged, list,
		[list, items, commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)]() {
			commit(valueFor(items(), listWidgetSelection(list)));
		});

	// Bound items: repopulate, keeping the selection by value (see ComboBox).
	if (const ItemList* boundItems = items.bound())
	{
		const T* boundValue = m_value.boundValue();
		bindWatchedRefSync(list, watchRefs(boundItems),
			[list] { return listItems(list); },
			[boundItems] { return *boundItems; },
			[list, boundValue](const ItemList& next) {
				const T keep = boundValue != nullptr
					? *boundValue
					: valueFor(listItems(list), listWidgetSelection(list));
				list->clear();
				for (const auto& item : next)
					list->addItem(qstr(item));
				setListWidgetSelection(list, indicesFor(next, keep));
			});
	}
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		bindWatchedRefSync(list, watchRefs(&value, items.bound()),
			[list] { return listWidgetSelection(list); },
			[&value, items] { return indicesFor(items(), value); },
			[list](const std::vector<int>& indices) { setListWidgetSelection(list, indices); });
	}
}

template <ListBoxValue T>
Size ListBoxWrapper<T>::measureIntrinsic(const Constraints&)
{
	return m_initialSize; // bound items only (measuresItself)
}

template class ListBoxWrapper<int>;
template class ListBoxWrapper<std::string>;
template class ListBoxWrapper<std::vector<int>>;
template class ListBoxWrapper<std::vector<std::string>>;

// CheckListBoxWrapper -----------------------------------------------------------

namespace
{

std::vector<int> checkListChecked(const QListWidget* list)
{
	std::vector<int> indices;
	for (int i = 0; i < list->count(); ++i)
	{
		if (list->item(i)->checkState() == Qt::Checked)
			indices.push_back(i);
	}
	return indices;
}

void setCheckListChecked(QListWidget* list, const std::vector<int>& indices)
{
	for (int i = 0; i < list->count(); ++i)
	{
		const bool checked = std::find(indices.begin(), indices.end(), i) != indices.end();
		list->item(i)->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
	}
}

} // unnamed namespace

namespace
{

void setCheckListItems(QListWidget* list, const ItemList& items, const std::vector<int>& checked)
{
	list->clear();
	for (int i = 0; i < static_cast<int>(items.size()); ++i)
	{
		auto* item = new QListWidgetItem(qstr(items[i]), list);
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(std::find(checked.begin(), checked.end(), i) != checked.end()
			? Qt::Checked : Qt::Unchecked);
	}
}

} // unnamed namespace

template <CheckListValue T>
void CheckListBoxWrapper<T>::realize(void* parentWindow)
{
	// The same sizeHint override ListBoxWrapper needs, for the same reason:
	// QListWidget's own hint is a fixed ~256x192 that reads neither the item
	// text nor the item count.
	auto* list = new SizedListWidget(static_cast<QWidget*>(parentWindow));
	list->visibleRows = m_visibleRows;
	// Single-SELECTION, whatever the checked set holds: the highlight and the
	// ticks are independent, as on wx.
	list->setSelectionMode(QAbstractItemView::SingleSelection);
	setCheckListItems(list, m_items.get(), indicesFor(m_items.get(), boundValue()));
	m_nativeWidget = list;
	const QSize hint = list->sizeHint();
	m_initialSize = Size { hint.width(), hint.height() };

	// itemChanged is connected only AFTER the population above: every
	// setCheckState() there emits it, so connecting first would read the
	// initial state as a series of user ticks. Repopulating later runs under
	// RefSync's QSignalBlocker for the same reason.
	const ItemsView items(m_items);
	QObject::connect(list, &QListWidget::itemChanged, list,
		[list, items, commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](QListWidgetItem*) {
			commit(valueFor(items(), checkListChecked(list)));
		});

	// Bound items: repopulate, keeping the ticks by value (see ComboBox).
	if (const ItemList* boundItems = items.bound())
	{
		const T* boundValue = m_value.boundValue();
		bindWatchedRefSync(list, watchRefs(boundItems),
			[list] { return listItems(list); },
			[boundItems] { return *boundItems; },
			[list, boundValue](const ItemList& next) {
				const T keep = boundValue != nullptr
					? *boundValue
					: valueFor(listItems(list), checkListChecked(list));
				setCheckListItems(list, next, indicesFor(next, keep));
			});
	}
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		bindWatchedRefSync(list, watchRefs(&value, items.bound()),
			[list] { return checkListChecked(list); },
			[&value, items] { return indicesFor(items(), value); },
			[list](const std::vector<int>& indices) { setCheckListChecked(list, indices); });
	}
}

template <CheckListValue T>
Size CheckListBoxWrapper<T>::measureIntrinsic(const Constraints&)
{
	return m_initialSize; // bound items only (measuresItself)
}

template class CheckListBoxWrapper<std::vector<int>>;
template class CheckListBoxWrapper<std::vector<std::string>>;

// TreeViewWrapper -----------------------------------------------------------

namespace
{

// An item's full path, stashed on the item itself so a selection decodes with a
// single lookup rather than by walking parents back up through their labels --
// which would also be ambiguous once two siblings share a name.
constexpr int kTreePathRole = Qt::UserRole;

// QTreeWidget inherits QAbstractScrollArea's fixed ~256x192 sizeHint, which
// ignores both the item text and the item count, so it would size the same tree
// differently from wx and ImGui. This one reports the content width measured at
// realize time and visibleRows of height -- a virtual override, no Q_OBJECT.
class SizedTreeWidget : public QTreeWidget
{
public:
	using QTreeWidget::QTreeWidget;

	int visibleRows = 1;
	int contentWidth = 0;

	QSize sizeHint() const override
	{
		const int rowHeight = topLevelItemCount() > 0
			? sizeHintForRow(0)
			: fontMetrics().height();
		const int chrome = frameWidth() * 2;
		return QSize(contentWidth + chrome + style()->pixelMetric(QStyle::PM_ScrollBarExtent),
			rowHeight * visibleRows + chrome);
	}
};

std::string treeItemPath(const QTreeWidgetItem* item)
{
	return item ? item->data(0, kTreePathRole).toString().toStdString() : std::string {};
}

std::vector<std::string> treeSelection(const QTreeWidget* tree)
{
	std::vector<std::string> paths;
	for (QTreeWidgetItemIterator it(const_cast<QTreeWidget*>(tree)); *it; ++it)
	{
		if ((*it)->isSelected())
			paths.push_back(treeItemPath(*it));
	}
	return paths;
}

void setTreeSelection(QTreeWidget* tree, const std::vector<std::string>& paths)
{
	for (QTreeWidgetItemIterator it(tree); *it; ++it)
	{
		const std::string path = treeItemPath(*it);
		(*it)->setSelected(std::find(paths.begin(), paths.end(), path) != paths.end());
	}
}

// `parent` is null for top-level items -- QTreeWidget is natively multi-root, so
// unlike wx there is no placeholder root to hang them off.
void addTreeItems(QTreeWidget* tree, QTreeWidgetItem* parent,
	const std::vector<TreeItem>& items, const std::string& parentPath, char separator)
{
	for (const TreeItem& item : items)
	{
		const std::string path = parentPath.empty()
			? item.label
			: parentPath + separator + item.label;
		auto* node = parent ? new QTreeWidgetItem(parent) : new QTreeWidgetItem(tree);
		node->setText(0, qstr(item.label));
		node->setData(0, kTreePathRole, qstr(path));
		addTreeItems(tree, node, item.children, path, separator);
		if (item.expanded && node->childCount() > 0)
			node->setExpanded(true);
	}
}

// Every item path in the tree, and the ones currently open -- what a refill
// needs to keep the user's open/closed state (TreeViewWrapper::openAfterRefill).
void treeExpansion(QTreeWidget* tree, std::vector<std::string>& all, std::vector<std::string>& open)
{
	for (QTreeWidgetItemIterator it(tree); *it; ++it)
	{
		const std::string path = treeItemPath(*it);
		all.push_back(path);
		if ((*it)->isExpanded())
			open.push_back(path);
	}
}

void applyTreeExpansion(QTreeWidget* tree, const std::vector<std::string>& open)
{
	for (QTreeWidgetItemIterator it(tree); *it; ++it)
	{
		if ((*it)->childCount() > 0)
			(*it)->setExpanded(std::find(open.begin(), open.end(), treeItemPath(*it)) != open.end());
	}
}

} // unnamed namespace

template <TreeViewValue T>
void TreeViewWrapper<T>::realize(void* parentWindow)
{
	auto* tree = new SizedTreeWidget(static_cast<QWidget*>(parentWindow));
	tree->visibleRows = m_visibleRows;
	tree->setColumnCount(1);
	// Single unnamed column: a TreeItem carries one label, and a header would
	// eat a row of height the engine has not budgeted for.
	tree->setHeaderHidden(true);
	addTreeItems(tree, nullptr, m_items.get(), std::string {}, kPathSeparator);

	// ExtendedSelection is Qt's ctrl/shift-click mode; MultiSelection would
	// toggle on a plain click, which is not what a desktop tree does.
	tree->setSelectionMode(m_multiSelect
		? QAbstractItemView::ExtendedSelection
		: QAbstractItemView::SingleSelection);

	// Measured here rather than in sizeHint(): indentation is per-level and
	// only the item tree knows each item's depth.
	int widest = 0;
	const QFontMetrics metrics = tree->fontMetrics();
	forEachItem(m_items.get(), [&](const TreeItem& item, const std::string&, int depth) {
		widest = std::max(widest,
			tree->indentation() * (depth + 1) + metrics.horizontalAdvance(qstr(item.label)));
	});
	tree->contentWidth = widest;
	const QSize hint = tree->sizeHint();
	m_initialSize = Size { hint.width(), hint.height() };

	setTreeSelection(tree, pathsFor(boundValue()));
	m_nativeWidget = tree;

	QObject::connect(tree, &QTreeWidget::itemSelectionChanged, tree,
		[tree, commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)]() {
			commit(valueFor(treeSelection(tree)));
		});

	// Bound items: refill when the caller's tree changes, registered BEFORE the
	// selection sync. The selection is kept by path (the bound one, else what
	// was picked) and every item that is still there keeps the open/closed
	// state the user left it at. The push runs under RefSync's QSignalBlocker,
	// so the refill reports no selection change. The tree keeps its first size.
	if (const std::vector<TreeItem>* boundItems = m_items.boundValue())
	{
		auto shown = std::make_shared<std::vector<TreeItem>>(*boundItems);
		const T* boundSelection = m_value.boundValue();
		bindWatchedRefSync(tree, watchRefs(boundItems),
			[shown] { return *shown; },
			[boundItems] { return *boundItems; },
			[tree, shown, boundSelection](const std::vector<TreeItem>& next) {
				const std::vector<std::string> keep = boundSelection != nullptr
					? pathsFor(*boundSelection)
					: treeSelection(tree);
				std::vector<std::string> before;
				std::vector<std::string> openBefore;
				treeExpansion(tree, before, openBefore);
				tree->clear();
				addTreeItems(tree, nullptr, next, std::string {}, kPathSeparator);
				applyTreeExpansion(tree, openAfterRefill(next, before, openBefore));
				setTreeSelection(tree, keep);
				*shown = next;
			});
	}
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		// setSelected() emits itemSelectionChanged for programmatic writes too,
		// but the ref sync already wraps every push in a QSignalBlocker on the
		// widget, so mirroring never re-enters the handler above.
		bindWatchedRefSync(tree, watchRefs(&value),
			[tree] { return treeSelection(tree); },
			[&value] { return pathsFor(value); },
			[tree](const std::vector<std::string>& paths) { setTreeSelection(tree, paths); });
	}
}

template <TreeViewValue T>
Size TreeViewWrapper<T>::measureIntrinsic(const Constraints&)
{
	return m_initialSize; // bound items only (measuresItself)
}

template class TreeViewWrapper<std::string>;
template class TreeViewWrapper<std::vector<std::string>>;

// TableWrapper -----------------------------------------------------------

namespace
{

// An item's ORIGINAL row index, stashed on its column-0 item. QTableWidget's
// sort moves whole rows, so the role travels with its row and stays correct
// once the view order and the data order have parted company.
constexpr int kTableRowRole = Qt::UserRole;

// QTableWidget inherits QAbstractScrollArea's fixed ~256x192 sizeHint, which
// ignores both the cell text and the row count, so it would size the same table
// differently from wx and ImGui. This one reports the summed column widths
// measured at realize time, and a header plus visibleRows of body -- a virtual
// override, no Q_OBJECT.
class SizedTableWidget : public QTableWidget
{
public:
	using QTableWidget::QTableWidget;

	int visibleRows = 1;
	int contentWidth = 0;

	QSize sizeHint() const override
	{
		const int rowHeight = rowCount() > 0 ? sizeHintForRow(0) : fontMetrics().height();
		const int chrome = frameWidth() * 2;
		// sizeHint() rather than height(): the header has not been laid out yet
		// the first time the engine measures us.
		const int headerHeight = horizontalHeader()->sizeHint().height();
		return QSize(contentWidth + chrome + style()->pixelMetric(QStyle::PM_ScrollBarExtent),
			headerHeight + rowHeight * visibleRows + chrome);
	}
};

int tableRowIndex(const QTableWidget* table, int viewRow)
{
	const QTableWidgetItem* item = table->item(viewRow, 0);
	return item ? item->data(kTableRowRole).toInt() : -1;
}

std::vector<int> tableSelection(const QTableWidget* table)
{
	std::vector<int> indices;
	for (int viewRow = 0; viewRow < table->rowCount(); ++viewRow)
	{
		const QTableWidgetItem* item = table->item(viewRow, 0);
		if (item && item->isSelected())
		{
			if (const int row = tableRowIndex(table, viewRow); row >= 0)
				indices.push_back(row);
		}
	}
	// Reported in original-index order rather than view order, so a multi-select
	// binding reads the same on all three backends however the table is sorted.
	std::sort(indices.begin(), indices.end());
	return indices;
}

// The rows the table holds, by ORIGINAL index -- whatever order a sort put
// them in on screen.
TableRows tableRows(const QTableWidget* table, int columnCount)
{
	const int count = table->rowCount();
	TableRows rows(static_cast<std::size_t>(count), TableRow(static_cast<std::size_t>(columnCount)));
	for (int viewRow = 0; viewRow < count; ++viewRow)
	{
		const int row = tableRowIndex(table, viewRow);
		if (row < 0 || row >= count)
			continue;
		for (int column = 0; column < columnCount; ++column)
		{
			if (const QTableWidgetItem* item = table->item(viewRow, column))
				rows[row][column] = item->text().toStdString();
		}
	}
	return rows;
}

// Replace every cell. Column 0 carries the original index (kTableRowRole);
// editability is per column, through each item's flags. The caller holds the
// syncing guard: setItem() emits itemChanged.
void fillTable(QTableWidget* table, const TableRows& rows, const std::vector<TableColumn>& columns)
{
	const int columnCount = static_cast<int>(columns.size());
	table->setRowCount(static_cast<int>(rows.size()));
	for (int row = 0; row < static_cast<int>(rows.size()); ++row)
	{
		for (int column = 0; column < columnCount; ++column)
		{
			auto* cell = new QTableWidgetItem(qstr(TableWrapper<int>::cellText(rows, row, column)));
			Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
			if (columns[column].editable)
				flags |= Qt::ItemIsEditable;
			cell->setFlags(flags);
			if (column == 0)
				cell->setData(kTableRowRole, row);
			table->setItem(row, column, cell);
		}
	}
}

// The sort the user last clicked, if any -- re-applied after a refill.
struct TableSort
{
	int column = -1;
	Qt::SortOrder order = Qt::AscendingOrder;
};

void setTableSelection(QTableWidget* table, const std::vector<int>& indices)
{
	QItemSelection selection;
	const int lastColumn = table->columnCount() - 1;
	for (int viewRow = 0; viewRow < table->rowCount(); ++viewRow)
	{
		const int row = tableRowIndex(table, viewRow);
		if (row >= 0 && std::find(indices.begin(), indices.end(), row) != indices.end())
		{
			selection.select(table->model()->index(viewRow, 0),
				table->model()->index(viewRow, lastColumn));
		}
	}
	table->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
}

} // unnamed namespace

template <TableValue T>
void TableWrapper<T>::realize(void* parentWindow)
{
	auto* table = new SizedTableWidget(static_cast<QWidget*>(parentWindow));
	m_nativeWidget = table;

	const TableRows& rows = m_rows.get();
	const int columnCount = static_cast<int>(m_columns.size());
	table->visibleRows = m_visibleRows;
	table->setColumnCount(columnCount);

	// A table's rows are its identity, so the row header would only ever show a
	// position the bindings deliberately do not use.
	table->verticalHeader()->setVisible(false);
	// Rows, not cells: a Table binds a row selection, and a cell-range selection
	// would have nothing to report through it.
	table->setSelectionBehavior(QAbstractItemView::SelectRows);
	// ExtendedSelection is Qt's ctrl/shift-click mode; MultiSelection would
	// toggle on a plain click, which is not what a desktop table does.
	table->setSelectionMode(kMultiSelect
		? QAbstractItemView::ExtendedSelection
		: QAbstractItemView::SingleSelection);

	QStringList headers;
	for (const TableColumn& column : m_columns)
		headers << qstr(column.label);
	table->setHorizontalHeaderLabels(headers);

	const QFontMetrics metrics = table->fontMetrics();
	const auto measureText = [&metrics](const std::string& text) {
		return metrics.horizontalAdvance(qstr(text));
	};

	constexpr int kCellPadding = 12; // Qt insets the cell's text on both sides
	int contentWidth = 0;
	for (int column = 0; column < columnCount; ++column)
	{
		const int width = columnWidth(m_columns, rows, column, measureText) + kCellPadding;
		contentWidth += width;
		table->setColumnWidth(column, width);
	}
	table->contentWidth = contentWidth;

	const bool anyEditable = std::any_of(m_columns.begin(), m_columns.end(),
		[](const TableColumn& column) { return column.editable; });
	// Editability is per column, applied through each item's flags; the view's
	// triggers only decide the gesture that opens whichever cells allow it.
	table->setEditTriggers(anyEditable
		? (QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed)
		: QAbstractItemView::NoEditTriggers);

	fillTable(table, rows, m_columns);

	setTableSelection(table, rowIndicesFor(rows, boundValue()));

	// Rows the handlers read: the caller's own while bound, so an edit made
	// anywhere is visible here, and a shared snapshot otherwise. Never the
	// wrapper's -- wrapper and window teardown order is not fixed.
	TableRows* editTarget = boundRows();
	auto snapshot = std::make_shared<const TableRows>(rows);
	const auto liveRows = [editTarget, snapshot]() -> const TableRows& {
		return editTarget ? *editTarget : *snapshot;
	};

	// Held across a programmatic sort, which moves items and reselects rows.
	// Without it the sort would look like a user edit and a user selection.
	auto syncing = std::make_shared<bool>(false);
	auto sort = std::make_shared<TableSort>();

	// Sorting is driven by hand rather than through setSortingEnabled(), which
	// is all-or-nothing: every header would sort, and TableColumn::sortable is
	// per column. sortItems() compares QTableWidgetItem text, the same
	// lexicographic order wx sorts with and ImGui's sortedOrder() reproduces.
	if (std::any_of(m_columns.begin(), m_columns.end(),
		[](const TableColumn& column) { return column.sortable; }))
	{
		QHeaderView* header = table->horizontalHeader();
		header->setSectionsClickable(true);
		header->setSortIndicatorShown(true);
		QObject::connect(header, &QHeaderView::sectionClicked, table,
			[table, header, syncing, sort, columns = m_columns](int section) {
				if (section < 0 || section >= static_cast<int>(columns.size())
					|| !columns[section].sortable)
					return;

				const bool wasAscendingHere = header->sortIndicatorSection() == section
					&& header->sortIndicatorOrder() == Qt::AscendingOrder;
				const Qt::SortOrder order = wasAscendingHere ? Qt::DescendingOrder : Qt::AscendingOrder;

				// Qt reselects by view position while sorting, which would drag
				// the binding onto whatever rows happen to land under the old
				// selection. Carry it across by original index instead.
				const std::vector<int> selected = tableSelection(table);
				*syncing = true;
				table->sortItems(section, order);
				setTableSelection(table, selected);
				*syncing = false;
				header->setSortIndicator(section, order);
				*sort = TableSort { section, order };
			});
	}

	if (anyEditable)
	{
		// Connected only now that every cell exists: setItem() emits itemChanged
		// for each one, and a handler bound earlier would take the whole
		// population pass for a series of user edits.
		QObject::connect(table, &QTableWidget::itemChanged, table,
			[table, editTarget, syncing, cb = std::move(m_onCellChange)](QTableWidgetItem* item) {
				if (*syncing || !item)
					return;
				const int row = tableRowIndex(table, item->row());
				if (row < 0)
					return;
				const std::string text = item->text().toStdString();
				applyCellEdit(editTarget, row, item->column(), text);
				if (cb)
					cb(row, item->column(), text);
			});
	}

	QObject::connect(table, &QTableWidget::itemSelectionChanged, table,
		[table, syncing, liveRows, commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)]() {
			if (!*syncing)
				commit(valueFor(liveRows(), tableSelection(table)));
		});

	// Bound rows: refill when the caller's data changes -- registered BEFORE the
	// selection sync, which then reads indices against the new rows. Compared in
	// the table's own shape, so a cell edit (already written through above, and
	// already on screen) is no change. Column widths stay what the first rows
	// made them, as a bound list keeps its first width; the sort the user chose
	// is re-applied and the selection kept by value.
	if (editTarget != nullptr)
	{
		const T* boundSelection = m_value.boundValue();
		bindWatchedRefSync(table, watchRefs(editTarget),
			[table, columnCount] { return tableRows(table, columnCount); },
			[editTarget, columnCount] { return normalizedRows(*editTarget, columnCount); },
			[table, syncing, sort, boundSelection, columns = m_columns, columnCount](const TableRows& next) {
				const T keep = boundSelection != nullptr
					? *boundSelection
					: valueFor(tableRows(table, columnCount), tableSelection(table));
				*syncing = true;
				fillTable(table, next, columns);
				if (sort->column >= 0)
					table->sortItems(sort->column, sort->order);
				setTableSelection(table, rowIndicesFor(next, keep));
				*syncing = false;
			});
	}
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		// select() emits itemSelectionChanged for programmatic writes too, but
		// the ref sync already wraps every push in a QSignalBlocker on the
		// widget, so mirroring never re-enters the handler above.
		bindWatchedRefSync(table, watchRefs(&value, editTarget),
			[table] { return tableSelection(table); },
			[&value, liveRows] { return rowIndicesFor(liveRows(), value); },
			[table](const std::vector<int>& indices) { setTableSelection(table, indices); });
	}
}

template class TableWrapper<int>;
template class TableWrapper<std::string>;
template class TableWrapper<std::vector<int>>;
template class TableWrapper<std::vector<std::string>>;
