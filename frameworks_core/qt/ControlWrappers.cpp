#include "frameworks_core/ControlWrappers.hpp"
#include "frameworks_core/qt/RefSync.hpp"
#include <algorithm>
#include <cmath>
#include <memory>

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#include <QAction>
#include <QIcon>
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
	auto* button = new QPushButton(qstr(m_label), static_cast<QWidget*>(parentWindow));
	// QPushButton is autoDefault inside a QDialog, so the first one built would come
	// up drawn as the dialog's default button (blue on macOS) and keep that highlight
	// for the life of the dialog. wx and ImGui highlight nothing, so neither do we.
	button->setAutoDefault(false);
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
	const std::string& initial = m_value.get();
	auto* edit = new QLineEdit(qstr(initial), static_cast<QWidget*>(parentWindow));
	// QLineEdit's sizeHint is a fixed character count, so the hint text cannot
	// reach the layout here -- no pinning needed, unlike wx.
	if (!m_placeholder.empty())
		edit->setPlaceholderText(qstr(m_placeholder));
	m_nativeWidget = edit;

	if (m_value.isBound())
	{
		auto& value = m_value.get();
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
	const std::string& initial = m_value.get();
	auto* edit = new QLineEdit(qstr(initial), static_cast<QWidget*>(parentWindow));
	edit->setEchoMode(QLineEdit::Password);
	if (!m_placeholder.empty())
		edit->setPlaceholderText(qstr(m_placeholder));
	m_nativeWidget = edit;

	if (m_value.isBound())
	{
		auto& value = m_value.get();
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
	const std::string& initial = m_value.get();
	auto* edit = new QPlainTextEdit(qstr(initial), static_cast<QWidget*>(parentWindow));
	m_nativeWidget = edit;

	if (m_value.isBound())
	{
		auto& value = m_value.get();
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
	const Date& initial = m_value.get();
	auto* picker = new QDateEdit(QDate(initial.year, initial.month, initial.day), static_cast<QWidget*>(parentWindow));
	picker->setCalendarPopup(true);
	m_nativeWidget = picker;

	auto toDate = [](const QDate& d) { return Date { d.year(), d.month(), d.day() }; };
	if (m_value.isBound())
	{
		auto& value = m_value.get();
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
	const Time& initial = m_value.get();
	auto* picker = new QTimeEdit(QTime(initial.hour, initial.minute, initial.second), static_cast<QWidget*>(parentWindow));
	picker->setDisplayFormat(QStringLiteral("HH:mm:ss"));
	m_nativeWidget = picker;

	auto toTime = [](QTime t) { return Time { t.hour(), t.minute(), t.second() }; };
	if (m_value.isBound())
	{
		auto& value = m_value.get();
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
	const T& initial = m_value.get();
	if constexpr (std::is_same_v<T, int>)
	{
		auto* spin = new QSpinBox(static_cast<QWidget*>(parentWindow));
		spin->setRange(m_range.min, m_range.max);
		spin->setSingleStep(static_cast<int>(m_range.step));
		spin->setValue(initial);
		m_nativeWidget = spin;

		if (m_value.isBound())
		{
			auto& value = m_value.get();
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

		if (m_value.isBound())
		{
			auto& value = m_value.get();
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
	const T& initial = m_value.get();
	auto* radio = new QRadioButton(qstr(m_label), static_cast<QWidget*>(parentWindow));
	m_nativeWidget = radio;

	QButtonGroup* group = currentRadioGroup(static_cast<QWidget*>(parentWindow), m_index == 0);
	group->addButton(radio, m_index);

	if constexpr (std::is_same_v<T, bool>)
		radio->setChecked(initial);
	else
		radio->setChecked(static_cast<int>(initial) == m_index);

	if (m_value.isBound())
	{
		auto& value = m_value.get();
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
	const bool checked = m_value.get();
	auto* box = new QCheckBox(qstr(m_label), static_cast<QWidget*>(parentWindow));
	box->setChecked(checked);
	m_nativeWidget = box;

	if (m_value.isBound())
	{
		auto& value = m_value.get();
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
	const bool toggled = m_value.get();
	auto* button = new QPushButton(qstr(m_label), static_cast<QWidget*>(parentWindow));
	button->setCheckable(true);
	button->setChecked(toggled);
	button->setAutoDefault(false);
	m_nativeWidget = button;

	if (m_value.isBound())
	{
		auto& value = m_value.get();
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
			? bar->addAction(qstr(tool.label))
			: bar->addAction(icon, qstr(tool.label));
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
		auto* label = new QLabel(qstr(field.text.get()), bar);
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
			cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget]() {
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
			if (cb) cb(color);
			else if (cbw) cbw(color, nw);
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

	// One commit path for both halves: the dialog writes the field, and the
	// field is what everything else reads. That is what makes a typed path and
	// a picked one indistinguishable downstream (R11.4).
	auto commit = [edit](const std::string& path) { edit->setText(qstr(path)); };

	if (m_value.isBound())
	{
		auto& value = m_value.get();
		// textEdited, not textChanged: it fires for user typing only, so the
		// dialog's own setText below re-enters this through exactly one route
		// rather than two.
		QObject::connect(edit, &QLineEdit::textEdited,
			[&value, cb = m_onChange, cbw = m_onChangeWithWidget, nw = m_nativeWidget](const QString& text) {
				value = text.toStdString();
				if (cb) cb(value);
				else if (cbw) cbw(value, nw);
			});
		bindExternalRefSync(edit,
			[edit] { return edit->text().toStdString(); },
			[&value] { return value; },
			[edit](const std::string& v) { edit->setText(qstr(v)); });
	}

	// The dialog leg. It writes the field and then reports, in that order, so a
	// handler reading the bound value already sees the new one (rules.md C4).
	// Everything it needs is captured by value -- the wrapper is not, since its
	// teardown order against the widget is not fixed.
	std::string* bound = m_value.isBound() ? &m_value.get() : nullptr;
	QObject::connect(browse, &QToolButton::clicked,
		[composite, edit, commit, bound, mode = m_mode, filters = m_filters,
			title = m_dialogTitle, cb = m_onChange, cbw = m_onChangeWithWidget,
			nw = m_nativeWidget]() {
			const std::string current = edit->text().toStdString();
			const std::string chosen = qtRunFileDialog(composite, title, mode, filters, current);
			if (chosen.empty())
				return; // cancel leaves the path alone -- it is not a selection of ""
			commit(chosen);
			if (bound)
				*bound = chosen;
			if (cb) cb(chosen);
			else if (cbw) cbw(chosen, nw);
		});

	// Unbound and with a callback: the field is still the value, so typing has
	// to report too. (Bound values took this leg above.)
	if (!m_value.isBound())
	{
		if (m_onChange)
			QObject::connect(edit, &QLineEdit::textEdited,
				[cb = std::move(m_onChange)](const QString& text) { cb(text.toStdString()); });
		else if (m_onChangeWithWidget)
			QObject::connect(edit, &QLineEdit::textEdited,
				[cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](const QString& text) {
					cbw(text.toStdString(), nw);
				});
	}

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
	header->setText(qstr(m_label));
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
	// bindInvalidation: this only keeps the header itself honest.
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
	bar->setRange(0, 100);
	bar->setValue(toBar(initial));
	m_nativeWidget = bar;

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

template <ComboBoxValue T>
void ComboBoxWrapper<T>::realize(void* parentWindow)
{
	auto* combo = new QComboBox(static_cast<QWidget*>(parentWindow));
	for (const auto& choice : m_choices)
		combo->addItem(qstr(choice));
	const T& selected = m_value.get();
	if constexpr (std::is_same_v<T, std::string>)
		combo->setCurrentText(qstr(selected));
	else
		combo->setCurrentIndex(selected);
	m_nativeWidget = combo;

	if (m_value.isBound())
	{
		auto& value = m_value.get();
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

	if (m_value.isBound())
	{
		auto& value = m_value.get();
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

	const std::vector<int> checked = indicesFor(m_items, boundValue());
	for (int i = 0; i < static_cast<int>(m_items.size()); ++i)
	{
		auto* item = new QListWidgetItem(qstr(m_items[i]), list);
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(std::find(checked.begin(), checked.end(), i) != checked.end()
			? Qt::Checked : Qt::Unchecked);
	}
	m_nativeWidget = list;

	// itemChanged is connected only AFTER the population above: every
	// setCheckState() there emits it, so connecting first would read the
	// initial state as a series of user ticks.
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		QObject::connect(list, &QListWidget::itemChanged, list,
			[&value, list, items = m_items, cb = std::move(m_onChange),
				cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](QListWidgetItem*) {
				value = valueFor(items, checkListChecked(list));
				if (cb) cb(value);
				else if (cbw) cbw(value, nw);
			});
		bindExternalRefSync(list,
			[list] { return checkListChecked(list); },
			[&value, items = m_items] { return indicesFor(items, value); },
			[list](const std::vector<int>& indices) { setCheckListChecked(list, indices); });
	}
	else if (m_onChange)
	{
		QObject::connect(list, &QListWidget::itemChanged, list,
			[list, items = m_items, cb = std::move(m_onChange)](QListWidgetItem*) {
				cb(valueFor(items, checkListChecked(list)));
			});
	}
	else if (m_onChangeWithWidget)
	{
		QObject::connect(list, &QListWidget::itemChanged, list,
			[list, items = m_items, cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](QListWidgetItem*) {
				cbw(valueFor(items, checkListChecked(list)), nw);
			});
	}
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
	addTreeItems(tree, nullptr, m_items, std::string {}, kPathSeparator);

	// ExtendedSelection is Qt's ctrl/shift-click mode; MultiSelection would
	// toggle on a plain click, which is not what a desktop tree does.
	tree->setSelectionMode(m_multiSelect
		? QAbstractItemView::ExtendedSelection
		: QAbstractItemView::SingleSelection);

	// Measured here rather than in sizeHint(): indentation is per-level and
	// only the item tree knows each item's depth.
	int widest = 0;
	const QFontMetrics metrics = tree->fontMetrics();
	forEachItem(m_items, [&](const TreeItem& item, const std::string&, int depth) {
		widest = std::max(widest,
			tree->indentation() * (depth + 1) + metrics.horizontalAdvance(qstr(item.label)));
	});
	tree->contentWidth = widest;

	setTreeSelection(tree, pathsFor(boundValue()));
	m_nativeWidget = tree;

	if (m_value.isBound())
	{
		auto& value = m_value.get();
		QObject::connect(tree, &QTreeWidget::itemSelectionChanged, tree,
			[&value, tree, cb = std::move(m_onChange),
				cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget]() {
				value = valueFor(treeSelection(tree));
				if (cb) cb(value);
				else if (cbw) cbw(value, nw);
			});
		// setSelected() emits itemSelectionChanged for programmatic writes too,
		// but the ref sync already wraps every push in a QSignalBlocker on the
		// widget, so mirroring never re-enters the handler above.
		bindExternalRefSync(tree,
			[tree] { return treeSelection(tree); },
			[&value] { return pathsFor(value); },
			[tree](const std::vector<std::string>& paths) { setTreeSelection(tree, paths); });
	}
	else if (m_onChange)
	{
		QObject::connect(tree, &QTreeWidget::itemSelectionChanged, tree,
			[tree, cb = std::move(m_onChange)]() {
				cb(valueFor(treeSelection(tree)));
			});
	}
	else if (m_onChangeWithWidget)
	{
		QObject::connect(tree, &QTreeWidget::itemSelectionChanged, tree,
			[tree, cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget]() {
				cbw(valueFor(treeSelection(tree)), nw);
			});
	}
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
	table->setRowCount(static_cast<int>(rows.size()));

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

	for (int row = 0; row < static_cast<int>(rows.size()); ++row)
	{
		for (int column = 0; column < columnCount; ++column)
		{
			auto* cell = new QTableWidgetItem(qstr(cellText(rows, row, column)));
			Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
			if (m_columns[column].editable)
				flags |= Qt::ItemIsEditable;
			cell->setFlags(flags);
			if (column == 0)
				cell->setData(kTableRowRole, row);
			table->setItem(row, column, cell);
		}
	}

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
			[table, header, syncing, columns = m_columns](int section) {
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

	if (m_value.isBound())
	{
		auto& value = m_value.get();
		QObject::connect(table, &QTableWidget::itemSelectionChanged, table,
			[&value, table, syncing, liveRows, cb = std::move(m_onChange),
				cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget]() {
				if (*syncing)
					return;
				value = valueFor(liveRows(), tableSelection(table));
				if (cb) cb(value);
				else if (cbw) cbw(value, nw);
			});
		// select() emits itemSelectionChanged for programmatic writes too, but
		// the ref sync already wraps every push in a QSignalBlocker on the
		// widget, so mirroring never re-enters the handler above.
		bindExternalRefSync(table,
			[table] { return tableSelection(table); },
			[&value, liveRows] { return rowIndicesFor(liveRows(), value); },
			[table](const std::vector<int>& indices) { setTableSelection(table, indices); });
	}
	else if (m_onChange)
	{
		QObject::connect(table, &QTableWidget::itemSelectionChanged, table,
			[table, syncing, liveRows, cb = std::move(m_onChange)]() {
				if (!*syncing)
					cb(valueFor(liveRows(), tableSelection(table)));
			});
	}
	else if (m_onChangeWithWidget)
	{
		QObject::connect(table, &QTableWidget::itemSelectionChanged, table,
			[table, syncing, liveRows, cbw = std::move(m_onChangeWithWidget),
				nw = m_nativeWidget]() {
				if (!*syncing)
					cbw(valueFor(liveRows(), tableSelection(table)), nw);
			});
	}
}

template class TableWrapper<int>;
template class TableWrapper<std::string>;
template class TableWrapper<std::vector<int>>;
template class TableWrapper<std::vector<std::string>>;
