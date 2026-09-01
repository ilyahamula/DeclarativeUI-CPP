#include "frameworks_core/ControlWrappers.hpp"
#include "frameworks_core/qt/RefSync.hpp"
#include <algorithm>
#include <cmath>

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

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
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QStyle>
#include <QTimeEdit>

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

} // unnamed namespace

// ButtonWrapper -----------------------------------------------------------

void ButtonWrapper::realize(void* parentWindow)
{
	auto* button = new QPushButton(qstr(m_label), static_cast<QWidget*>(parentWindow));
	m_nativeWidget = button;

	if (m_onClick)
		QObject::connect(button, &QPushButton::clicked, [cb = std::move(m_onClick)] { cb(); });
	else if (m_onClickWithWidget)
		QObject::connect(button, &QPushButton::clicked,
			[cb = std::move(m_onClickWithWidget), nw = m_nativeWidget] { cb(nw); });

}

// TextCtrlWrapper -----------------------------------------------------------

void TextCtrlWrapper::realize(void* parentWindow)
{
	const std::string& initial = m_externalRef ? m_externalRef->get() : m_ownedValue;
	auto* edit = new QLineEdit(qstr(initial), static_cast<QWidget*>(parentWindow));
	m_nativeWidget = edit;

	if (m_externalRef)
	{
		auto& value = m_externalRef->get();
		QObject::connect(edit, &QLineEdit::textChanged,
			[&value, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](const QString& text) {
				value = text.toStdString();
				if (cb) cb(value);
				else if (cbw) cbw(value, nw);
			});
		bindExternalRefSync(edit,
			[edit] { return edit->text().toStdString(); },
			[&value] { return value; },
			[edit](const std::string& v) { edit->setText(qstr(v)); });
	}
	else if (m_onChange)
		QObject::connect(edit, &QLineEdit::textChanged,
			[cb = std::move(m_onChange)](const QString& text) { cb(text.toStdString()); });
	else if (m_onChangeWithWidget)
		QObject::connect(edit, &QLineEdit::textChanged,
			[cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](const QString& text) { cbw(text.toStdString(), nw); });

}

// PasswordInputWrapper -----------------------------------------------------------

void PasswordInputWrapper::realize(void* parentWindow)
{
	const std::string& initial = m_externalRef ? m_externalRef->get() : m_ownedValue;
	auto* edit = new QLineEdit(qstr(initial), static_cast<QWidget*>(parentWindow));
	edit->setEchoMode(QLineEdit::Password);
	m_nativeWidget = edit;

	if (m_externalRef)
	{
		auto& value = m_externalRef->get();
		QObject::connect(edit, &QLineEdit::textChanged,
			[&value, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](const QString& text) {
				value = text.toStdString();
				if (cb) cb(value);
				else if (cbw) cbw(value, nw);
			});
		bindExternalRefSync(edit,
			[edit] { return edit->text().toStdString(); },
			[&value] { return value; },
			[edit](const std::string& v) { edit->setText(qstr(v)); });
	}
	else if (m_onChange)
		QObject::connect(edit, &QLineEdit::textChanged,
			[cb = std::move(m_onChange)](const QString& text) { cb(text.toStdString()); });
	else if (m_onChangeWithWidget)
		QObject::connect(edit, &QLineEdit::textChanged,
			[cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](const QString& text) { cbw(text.toStdString(), nw); });

}

// MultiLineTextCtrlWrapper -----------------------------------------------------------

void MultiLineTextCtrlWrapper::realize(void* parentWindow)
{
	const std::string& initial = m_externalRef ? m_externalRef->get() : m_ownedValue;
	auto* edit = new QPlainTextEdit(qstr(initial), static_cast<QWidget*>(parentWindow));
	m_nativeWidget = edit;

	if (m_externalRef)
	{
		auto& value = m_externalRef->get();
		QObject::connect(edit, &QPlainTextEdit::textChanged,
			[edit, &value, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget] {
				value = edit->toPlainText().toStdString();
				if (cb) cb(value);
				else if (cbw) cbw(value, nw);
			});
		bindExternalRefSync(edit,
			[edit] { return edit->toPlainText().toStdString(); },
			[&value] { return value; },
			[edit](const std::string& v) { edit->setPlainText(qstr(v)); });
	}
	else if (m_onChange)
		QObject::connect(edit, &QPlainTextEdit::textChanged,
			[edit, cb = std::move(m_onChange)] { cb(edit->toPlainText().toStdString()); });
	else if (m_onChangeWithWidget)
		QObject::connect(edit, &QPlainTextEdit::textChanged,
			[edit, cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget] { cbw(edit->toPlainText().toStdString(), nw); });

}

// ReadonlyTextCtrlWrapper -----------------------------------------------------------

void ReadonlyTextCtrlWrapper::realize(void* parentWindow)
{
	auto* edit = new QLineEdit(qstr(m_value), static_cast<QWidget*>(parentWindow));
	edit->setReadOnly(true);
	m_nativeWidget = edit;

}

// ClickableTextWrapper -----------------------------------------------------------

void ClickableTextWrapper::realize(void* parentWindow)
{
	auto* label = new ClickableLabel(qstr(m_text), static_cast<QWidget*>(parentWindow));
	m_nativeWidget = label;
	if (m_onClick)
		label->onClick = std::move(m_onClick);
	else if (m_onClickWithWidget)
		label->onClick = [cb = std::move(m_onClickWithWidget), nw = m_nativeWidget] { cb(nw); };

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
		QObject::connect(label, &QLabel::linkActivated, [cb = std::move(m_onClick)](const QString&) { cb(); });
	else if (m_onClickWithWidget)
		QObject::connect(label, &QLabel::linkActivated,
			[cb = std::move(m_onClickWithWidget), nw = m_nativeWidget](const QString&) { cb(nw); });

}

// StaticTextWrapper -----------------------------------------------------------

void StaticTextWrapper::realize(void* parentWindow)
{
	m_nativeWidget = new QLabel(qstr(m_text), static_cast<QWidget*>(parentWindow));

}

// DatePickerWrapper -----------------------------------------------------------

void DatePickerWrapper::realize(void* parentWindow)
{
	const Date& initial = m_externalRef ? m_externalRef->get() : m_ownedValue;
	auto* picker = new QDateEdit(QDate(initial.year, initial.month, initial.day), static_cast<QWidget*>(parentWindow));
	picker->setCalendarPopup(true);
	m_nativeWidget = picker;

	auto toDate = [](const QDate& d) { return Date { d.year(), d.month(), d.day() }; };
	if (m_externalRef)
	{
		auto& value = m_externalRef->get();
		QObject::connect(picker, &QDateEdit::dateChanged,
			[&value, toDate, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](QDate d) {
				value = toDate(d);
				if (cb) cb(value);
				else if (cbw) cbw(value, nw);
			});
		bindExternalRefSync(picker,
			[picker] { return picker->date(); },
			[&value] { return QDate(value.year, value.month, value.day); },
			[picker](const QDate& d) { picker->setDate(d); });
	}
	else if (m_onChange)
		QObject::connect(picker, &QDateEdit::dateChanged,
			[toDate, cb = std::move(m_onChange)](QDate d) { cb(toDate(d)); });
	else if (m_onChangeWithWidget)
		QObject::connect(picker, &QDateEdit::dateChanged,
			[toDate, cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](QDate d) { cbw(toDate(d), nw); });

}

// TimePickerWrapper -----------------------------------------------------------

void TimePickerWrapper::realize(void* parentWindow)
{
	const Time& initial = m_externalRef ? m_externalRef->get() : m_ownedValue;
	auto* picker = new QTimeEdit(QTime(initial.hour, initial.minute, initial.second), static_cast<QWidget*>(parentWindow));
	picker->setDisplayFormat(QStringLiteral("HH:mm:ss"));
	m_nativeWidget = picker;

	auto toTime = [](QTime t) { return Time { t.hour(), t.minute(), t.second() }; };
	if (m_externalRef)
	{
		auto& value = m_externalRef->get();
		QObject::connect(picker, &QTimeEdit::timeChanged,
			[&value, toTime, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](QTime t) {
				value = toTime(t);
				if (cb) cb(value);
				else if (cbw) cbw(value, nw);
			});
		bindExternalRefSync(picker,
			[picker] { return picker->time(); },
			[&value] { return QTime(value.hour, value.minute, value.second); },
			[picker](const QTime& t) { picker->setTime(t); });
	}
	else if (m_onChange)
		QObject::connect(picker, &QTimeEdit::timeChanged,
			[toTime, cb = std::move(m_onChange)](QTime t) { cb(toTime(t)); });
	else if (m_onChangeWithWidget)
		QObject::connect(picker, &QTimeEdit::timeChanged,
			[toTime, cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](QTime t) { cbw(toTime(t), nw); });

}

// SliderWrapper -----------------------------------------------------------

template <SliderValue T>
void SliderWrapper<T>::realize(void* parentWindow)
{
	const T& initial = m_externalRef ? m_externalRef->get() : m_ownedValue;
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
	if (m_externalRef)
	{
		auto& value = m_externalRef->get();
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
		QObject::connect(slider, &QSlider::valueChanged,
			[&value, toValue, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](int raw) {
				value = toValue(raw);
				if (cb) cb(value);
				else if (cbw) cbw(value, nw);
			});
	}
	else if (m_onChange)
		QObject::connect(slider, &QSlider::valueChanged,
			[toValue, cb = std::move(m_onChange)](int raw) { cb(toValue(raw)); });
	else if (m_onChangeWithWidget)
		QObject::connect(slider, &QSlider::valueChanged,
			[toValue, cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](int raw) { cbw(toValue(raw), nw); });

}

template class SliderWrapper<int>;
template class SliderWrapper<float>;

// SpinBoxWrapper -----------------------------------------------------------

template <SpinBoxValue T>
void SpinBoxWrapper<T>::realize(void* parentWindow)
{
	const T& initial = m_externalRef ? m_externalRef->get() : m_ownedValue;
	if constexpr (std::is_same_v<T, int>)
	{
		auto* spin = new QSpinBox(static_cast<QWidget*>(parentWindow));
		spin->setRange(m_range.min, m_range.max);
		spin->setSingleStep(static_cast<int>(m_range.step));
		spin->setValue(initial);
		m_nativeWidget = spin;

		if (m_externalRef)
		{
			auto& value = m_externalRef->get();
			QObject::connect(spin, &QSpinBox::valueChanged,
				[&value, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](int v) {
					value = v;
					if (cb) cb(value);
					else if (cbw) cbw(value, nw);
				});
			bindExternalRefSync(spin,
				[spin] { return spin->value(); },
				[&value] { return static_cast<int>(value); },
				[spin](int v) { spin->setValue(v); });
		}
		else if (m_onChange)
			QObject::connect(spin, &QSpinBox::valueChanged, [cb = std::move(m_onChange)](int v) { cb(v); });
		else if (m_onChangeWithWidget)
			QObject::connect(spin, &QSpinBox::valueChanged,
				[cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](int v) { cbw(v, nw); });
	}
	else
	{
		auto* spin = new QDoubleSpinBox(static_cast<QWidget*>(parentWindow));
		spin->setRange(m_range.min, m_range.max);
		spin->setSingleStep(m_range.step);
		spin->setValue(initial);
		m_nativeWidget = spin;

		if (m_externalRef)
		{
			auto& value = m_externalRef->get();
			QObject::connect(spin, &QDoubleSpinBox::valueChanged,
				[&value, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](double v) {
					value = static_cast<T>(v);
					if (cb) cb(value);
					else if (cbw) cbw(value, nw);
				});
			// Quantise to step units: QDoubleSpinBox rounds what it stores to its
			// decimals() setting, so a raw double compare would never settle.
			bindExternalRefSync(spin,
				[spin, step = m_range.step] { return std::lround(spin->value() / step); },
				[&value, step = m_range.step] { return std::lround(value / step); },
				[spin, step = m_range.step](long units) { spin->setValue(static_cast<double>(units) * step); });
		}
		else if (m_onChange)
			QObject::connect(spin, &QDoubleSpinBox::valueChanged,
				[cb = std::move(m_onChange)](double v) { cb(static_cast<T>(v)); });
		else if (m_onChangeWithWidget)
			QObject::connect(spin, &QDoubleSpinBox::valueChanged,
				[cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](double v) { cbw(static_cast<T>(v), nw); });
	}

}

template class SpinBoxWrapper<int>;
template class SpinBoxWrapper<float>;

// RadioButtonWrapper -----------------------------------------------------------

namespace
{
// Qt auto-groups radio buttons by parent widget; with flat parenting every
// radio would join one group. Mirror the wxRB_GROUP rule instead: a radio
// with index 0 starts a new QButtonGroup, later indices join it.
QButtonGroup* currentRadioGroup(QWidget* owner, bool startNew)
{
	static QButtonGroup* s_group = nullptr;
	if (startNew || s_group == nullptr)
		s_group = new QButtonGroup(owner);
	return s_group;
}
} // unnamed namespace

template <RadioButtonValue T>
void RadioButtonWrapper<T>::realize(void* parentWindow)
{
	const T& initial = m_externalRef ? m_externalRef->get() : m_ownedValue;
	auto* radio = new QRadioButton(qstr(m_label), static_cast<QWidget*>(parentWindow));
	m_nativeWidget = radio;

	QButtonGroup* group = currentRadioGroup(static_cast<QWidget*>(parentWindow), m_index == 0);
	group->addButton(radio, m_index);

	if constexpr (std::is_same_v<T, bool>)
		radio->setChecked(initial);
	else
		radio->setChecked(static_cast<int>(initial) == m_index);

	if (m_externalRef)
	{
		auto& value = m_externalRef->get();
		QObject::connect(radio, &QRadioButton::toggled,
			[&value, index = m_index, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](bool checked) {
				if (!checked)
					return;
				if constexpr (std::is_same_v<T, bool>)
					value = true;
				else
					value = index;
				if (cb) cb(value);
				else if (cbw) cbw(value, nw);
			});
		// Every radio syncs itself; QButtonGroup clears the siblings when one is set.
		bindExternalRefSync(radio,
			[radio] { return radio->isChecked(); },
			[&value, index = m_index] {
				if constexpr (std::is_same_v<T, bool>)
					return static_cast<bool>(value);
				else
					return static_cast<int>(value) == index;
			},
			[radio](bool on) { radio->setChecked(on); });
	}
	else if (m_onChange)
		QObject::connect(radio, &QRadioButton::toggled,
			[index = m_index, cb = std::move(m_onChange)](bool checked) {
				if (!checked)
					return;
				if constexpr (std::is_same_v<T, bool>)
					cb(true);
				else
					cb(static_cast<T>(index));
			});
	else if (m_onChangeWithWidget)
		QObject::connect(radio, &QRadioButton::toggled,
			[index = m_index, cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](bool checked) {
				if (!checked)
					return;
				if constexpr (std::is_same_v<T, bool>)
					cbw(true, nw);
				else
					cbw(static_cast<T>(index), nw);
			});

}

template class RadioButtonWrapper<bool>;
template class RadioButtonWrapper<int>;

// CheckBoxWrapper -----------------------------------------------------------

void CheckBoxWrapper::realize(void* parentWindow)
{
	const bool checked = m_externalRef ? m_externalRef->get() : m_ownedValue;
	auto* box = new QCheckBox(qstr(m_label), static_cast<QWidget*>(parentWindow));
	box->setChecked(checked);
	m_nativeWidget = box;

	if (m_externalRef)
	{
		auto& value = m_externalRef->get();
		QObject::connect(box, &QCheckBox::toggled,
			[&value, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](bool v) {
				value = v;
				if (cb) cb(value);
				else if (cbw) cbw(value, nw);
			});
		bindExternalRefSync(box,
			[box] { return box->isChecked(); },
			[&value] { return value; },
			[box](bool on) { box->setChecked(on); });
	}
	else if (m_onChange)
		QObject::connect(box, &QCheckBox::toggled, [cb = std::move(m_onChange)](bool v) { cb(v); });
	else if (m_onChangeWithWidget)
		QObject::connect(box, &QCheckBox::toggled,
			[cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](bool v) { cbw(v, nw); });

}

// ToggleButtonWrapper -----------------------------------------------------------

void ToggleButtonWrapper::realize(void* parentWindow)
{
	const bool toggled = m_externalRef ? m_externalRef->get() : m_ownedValue;
	auto* button = new QPushButton(qstr(m_label), static_cast<QWidget*>(parentWindow));
	button->setCheckable(true);
	button->setChecked(toggled);
	m_nativeWidget = button;

	if (m_externalRef)
	{
		auto& value = m_externalRef->get();
		QObject::connect(button, &QPushButton::toggled,
			[&value, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](bool v) {
				value = v;
				if (cb) cb(value);
				else if (cbw) cbw(value, nw);
			});
		bindExternalRefSync(button,
			[button] { return button->isChecked(); },
			[&value] { return value; },
			[button](bool on) { button->setChecked(on); });
	}
	else if (m_onChange)
		QObject::connect(button, &QPushButton::toggled, [cb = std::move(m_onChange)](bool v) { cb(v); });
	else if (m_onChangeWithWidget)
		QObject::connect(button, &QPushButton::toggled,
			[cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](bool v) { cbw(v, nw); });

}

// ImageWrapper -----------------------------------------------------------

void ImageWrapper::realize(void* parentWindow)
{
	auto* label = new ClickableLabel(static_cast<QWidget*>(parentWindow));
	QPixmap pixmap(qstr(m_filePath));
	if (!pixmap.isNull())
	{
		if (m_displayWidth > 0 && m_displayHeight > 0)
			pixmap = pixmap.scaled(m_displayWidth, m_displayHeight,
				Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		label->setPixmap(pixmap);
	}
	else
	{
		label->setText(QStringLiteral("[Image: failed to load]"));
	}
	m_nativeWidget = label;

	if (m_onClick)
		label->onClick = std::move(m_onClick);
	else if (m_onClickWithWidget)
		label->onClick = [cb = std::move(m_onClickWithWidget), nw = m_nativeWidget] { cb(nw); };
	if (m_onHover)
		label->onHover = std::move(m_onHover);
	else if (m_onHoverWithWidget)
		label->onHover = [cb = std::move(m_onHoverWithWidget), nw = m_nativeWidget] { cb(nw); };

}

// ColorPickerWrapper -----------------------------------------------------------

void ColorPickerWrapper::realize(void* parentWindow)
{
	const Color initial = m_externalRef ? m_externalRef->get() : m_ownedValue;
	auto* button = new QPushButton(static_cast<QWidget*>(parentWindow));
	auto sheetFor = [](const Color& c) {
		return QStringLiteral("background-color: rgba(%1,%2,%3,%4);")
			.arg((int)(c.r * 255)).arg((int)(c.g * 255)).arg((int)(c.b * 255)).arg((int)(c.a * 255));
	};
	auto applySwatch = [button, sheetFor](const Color& c) { button->setStyleSheet(sheetFor(c)); };
	applySwatch(initial);
	m_nativeWidget = button;

	// There is no getter for the swatch colour -- the stylesheet is the displayed
	// state, so compare on that. Byte-quantised, so no float equality either.
	if (m_externalRef)
	{
		auto& swatchValue = m_externalRef->get();
		bindExternalRefSync(button,
			[button] { return button->styleSheet(); },
			[&swatchValue, sheetFor] { return sheetFor(swatchValue); },
			[button](const QString& sheet) { button->setStyleSheet(sheet); });
	}

	// the button is the swatch; clicking opens the native color dialog
	auto external = m_externalRef;
	QObject::connect(button, &QPushButton::clicked,
		[button, applySwatch, external, initial,
			cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget]() {
			const Color current = external ? external->get() : initial;
			const QColor start((int)(current.r * 255), (int)(current.g * 255),
				(int)(current.b * 255), (int)(current.a * 255));
			const QColor picked = QColorDialog::getColor(start, button, QString(),
				QColorDialog::ShowAlphaChannel);
			if (!picked.isValid())
				return;
			const Color color { (float)picked.redF(), (float)picked.greenF(),
				(float)picked.blueF(), (float)picked.alphaF() };
			if (external)
				external->get() = color;
			applySwatch(color);
			if (cb) cb(color);
			else if (cbw) cbw(color, nw);
		});

}

// SeparatorWrapper -----------------------------------------------------------

void SeparatorWrapper::realize(void* parentWindow)
{
	auto* line = new QFrame(static_cast<QWidget*>(parentWindow));
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
	m_nativeWidget = line;

}

// ProgressBarWrapper -----------------------------------------------------------

void ProgressBarWrapper::realize(void* parentWindow)
{
	// The bound float is a 0..100 percentage, matching the bar's own integer range
	// (and what the wx and ImGui backends do -- Qt used to read it as a 0..1
	// fraction, which rendered the same tree differently here).
	const auto toBar = [](float v) { return static_cast<int>(std::clamp(v, 0.0f, 100.0f)); };

	const float initial = m_externalRef ? m_externalRef->get() : m_ownedValue;
	auto* bar = new QProgressBar(static_cast<QWidget*>(parentWindow));
	bar->setRange(0, 100);
	bar->setValue(toBar(initial));
	m_nativeWidget = bar;

	// A progress bar has no input of its own -- the bound float is only ever
	// written from outside -- so the sync is the whole story here.
	if (m_externalRef)
	{
		bindExternalRefSync(bar,
			[bar] { return bar->value(); },
			[&value = m_externalRef->get(), toBar] { return toBar(value); },
			[bar](int v) { bar->setValue(v); });
	}

}

// ComboBoxWrapper -----------------------------------------------------------

template <ComboBoxValue T>
void ComboBoxWrapper<T>::realize(void* parentWindow)
{
	auto* combo = new QComboBox(static_cast<QWidget*>(parentWindow));
	for (const auto& choice : m_choices)
		combo->addItem(qstr(choice));
	const T& selected = m_externalRef ? m_externalRef->get() : m_ownedSelected;
	if constexpr (std::is_same_v<T, std::string>)
		combo->setCurrentText(qstr(selected));
	else
		combo->setCurrentIndex(selected);
	m_nativeWidget = combo;

	if (m_externalRef)
	{
		auto& value = m_externalRef->get();
		if constexpr (std::is_same_v<T, std::string>)
			QObject::connect(combo, &QComboBox::currentTextChanged,
				[&value, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](const QString& text) {
					value = text.toStdString();
					if (cb) cb(value);
					else if (cbw) cbw(value, nw);
				});
		else
			QObject::connect(combo, &QComboBox::currentIndexChanged,
				[&value, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](int index) {
					value = index;
					if (cb) cb(value);
					else if (cbw) cbw(value, nw);
				});
		if constexpr (std::is_same_v<T, std::string>)
			bindExternalRefSync(combo,
				[combo] { return combo->currentText().toStdString(); },
				[&value] { return value; },
				[combo](const std::string& v) { combo->setCurrentText(qstr(v)); });
		else
			bindExternalRefSync(combo,
				[combo] { return combo->currentIndex(); },
				[&value] { return static_cast<int>(value); },
				[combo](int i) { combo->setCurrentIndex(i); });
	}
	else if (m_onChange)
	{
		if constexpr (std::is_same_v<T, std::string>)
			QObject::connect(combo, &QComboBox::currentTextChanged,
				[cb = std::move(m_onChange)](const QString& text) { cb(text.toStdString()); });
		else
			QObject::connect(combo, &QComboBox::currentIndexChanged,
				[cb = std::move(m_onChange)](int index) { cb(static_cast<T>(index)); });
	}
	else if (m_onChangeWithWidget)
	{
		if constexpr (std::is_same_v<T, std::string>)
			QObject::connect(combo, &QComboBox::currentTextChanged,
				[cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](const QString& text) { cbw(text.toStdString(), nw); });
		else
			QObject::connect(combo, &QComboBox::currentIndexChanged,
				[cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](int index) { cbw(static_cast<T>(index), nw); });
	}

}

template class ComboBoxWrapper<std::string>;
template class ComboBoxWrapper<int>;

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

template <ListBoxValue T>
void ListBoxWrapper<T>::realize(void* parentWindow)
{
	auto* list = new SizedListWidget(static_cast<QWidget*>(parentWindow));
	list->visibleRows = m_visibleRows;
	for (const auto& item : m_items)
		list->addItem(qstr(item));
	// ExtendedSelection is Qt's ctrl/shift-click mode; MultiSelection would
	// toggle on a plain click, which is not what a desktop list does.
	list->setSelectionMode(kMultiSelect
		? QAbstractItemView::ExtendedSelection
		: QAbstractItemView::SingleSelection);
	setListWidgetSelection(list, indicesFor(m_items, boundValue()));
	m_nativeWidget = list;

	if (m_externalRef)
	{
		auto& value = m_externalRef->get();
		QObject::connect(list, &QListWidget::itemSelectionChanged, list,
			[&value, list, items = m_items, cb = std::move(m_onChange),
				cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget]() {
				value = valueFor(items, listWidgetSelection(list));
				if (cb) cb(value);
				else if (cbw) cbw(value, nw);
			});
		bindExternalRefSync(list,
			[list] { return listWidgetSelection(list); },
			[&value, items = m_items] { return indicesFor(items, value); },
			[list](const std::vector<int>& indices) { setListWidgetSelection(list, indices); });
	}
	else if (m_onChange)
	{
		QObject::connect(list, &QListWidget::itemSelectionChanged, list,
			[list, items = m_items, cb = std::move(m_onChange)]() {
				cb(valueFor(items, listWidgetSelection(list)));
			});
	}
	else if (m_onChangeWithWidget)
	{
		QObject::connect(list, &QListWidget::itemSelectionChanged, list,
			[list, items = m_items, cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget]() {
				cbw(valueFor(items, listWidgetSelection(list)), nw);
			});
	}
}

template class ListBoxWrapper<int>;
template class ListBoxWrapper<std::string>;
template class ListBoxWrapper<std::vector<int>>;
template class ListBoxWrapper<std::vector<std::string>>;
