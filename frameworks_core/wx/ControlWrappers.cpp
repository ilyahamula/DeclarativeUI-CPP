#include "frameworks_core/ControlWrappers.hpp"
#include "frameworks_core/LayoutWrapper.hpp"
#include <algorithm>

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#include <wx/wx.h>
#include <wx/hyperlink.h>
#include <wx/spinctrl.h>
#include <wx/datectrl.h>
#include <wx/timectrl.h>
#include <wx/dateevt.h>
#include <wx/tglbtn.h>
#include <wx/gauge.h>
#include <wx/statline.h>
#include <wx/clrpicker.h>

// ButtonWrapper -----------------------------------------------------------

ButtonWrapper::ButtonWrapper(ControlWrapper* parent, const std::string& label,
	const Position& pos, const Size& size, long style,
	std::function<void()> onClick)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ButtonWrapper::ButtonWrapper()\t-> new wxButton()\n");
#endif
	auto* btn = new wxButton(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, label,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	if (onClick)
		btn->Bind(wxEVT_BUTTON, [cb = std::move(onClick)](wxCommandEvent&) { cb(); });
	m_nativeWidget = btn;
}

// TextCtrlWrapper -----------------------------------------------------------

TextCtrlWrapper::TextCtrlWrapper(ControlWrapper* parent, std::string& value,
	const Position& pos, const Size& size, long style,
	std::function<void(const std::string&)> onChange)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "TextCtrlWrapper::TextCtrlWrapper()\t-> new wxTextCtrl()\n");
#endif
	auto* tc = new wxTextCtrl(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, value,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	tc->Bind(wxEVT_TEXT, [&value, cb = std::move(onChange)](wxCommandEvent& evt) {
		value = evt.GetString().ToStdString();
		if (cb) cb(value);
	});
	m_nativeWidget = tc;
}

TextCtrlWrapper::TextCtrlWrapper(ControlWrapper* parent, const std::string& initialValue,
	const Position& pos, const Size& size, long style,
	std::function<void(const std::string&)> onChange)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "TextCtrlWrapper::TextCtrlWrapper(unbound)\t-> new wxTextCtrl()\n");
#endif
	auto* tc = new wxTextCtrl(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, initialValue,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	if (onChange)
		tc->Bind(wxEVT_TEXT, [cb = std::move(onChange)](wxCommandEvent& evt) {
			cb(evt.GetString().ToStdString());
		});
	m_nativeWidget = tc;
}

// PasswordInputWrapper -----------------------------------------------------------

PasswordInputWrapper::PasswordInputWrapper(ControlWrapper* parent, std::string& value,
	const Position& pos, const Size& size, long style,
	std::function<void(const std::string&)> onChange)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "PasswordInputWrapper::PasswordInputWrapper()\t-> new wxTextCtrl(wxTE_PASSWORD)\n");
#endif
	auto* tc = new wxTextCtrl(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, value,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style | wxTE_PASSWORD);
	tc->Bind(wxEVT_TEXT, [&value, cb = std::move(onChange)](wxCommandEvent& evt) {
		value = evt.GetString().ToStdString();
		if (cb) cb(value);
	});
	m_nativeWidget = tc;
}

PasswordInputWrapper::PasswordInputWrapper(ControlWrapper* parent, const std::string& initialValue,
	const Position& pos, const Size& size, long style,
	std::function<void(const std::string&)> onChange)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "PasswordInputWrapper::PasswordInputWrapper(unbound)\t-> new wxTextCtrl(wxTE_PASSWORD)\n");
#endif
	auto* tc = new wxTextCtrl(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, initialValue,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style | wxTE_PASSWORD);
	if (onChange)
		tc->Bind(wxEVT_TEXT, [cb = std::move(onChange)](wxCommandEvent& evt) {
			cb(evt.GetString().ToStdString());
		});
	m_nativeWidget = tc;
}

// MultiLineTextCtrlWrapper -----------------------------------------------------------

MultiLineTextCtrlWrapper::MultiLineTextCtrlWrapper(ControlWrapper* parent, std::string& value,
	const Position& pos, const Size& size, long style,
	std::function<void(const std::string&)> onChange)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "MultiLineTextCtrlWrapper::MultiLineTextCtrlWrapper()\t-> new wxTextCtrl(wxTE_MULTILINE)\n");
#endif
	auto* tc = new wxTextCtrl(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, value,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style | wxTE_MULTILINE);
	tc->Bind(wxEVT_TEXT, [&value, cb = std::move(onChange)](wxCommandEvent& evt) {
		value = evt.GetString().ToStdString();
		if (cb) cb(value);
	});
	m_nativeWidget = tc;
}

MultiLineTextCtrlWrapper::MultiLineTextCtrlWrapper(ControlWrapper* parent, const std::string& initialValue,
	const Position& pos, const Size& size, long style,
	std::function<void(const std::string&)> onChange)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "MultiLineTextCtrlWrapper::MultiLineTextCtrlWrapper(unbound)\t-> new wxTextCtrl(wxTE_MULTILINE)\n");
#endif
	auto* tc = new wxTextCtrl(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, initialValue,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style | wxTE_MULTILINE);
	if (onChange)
		tc->Bind(wxEVT_TEXT, [cb = std::move(onChange)](wxCommandEvent& evt) {
			cb(evt.GetString().ToStdString());
		});
	m_nativeWidget = tc;
}

// ReadonlyTextCtrlWrapper -----------------------------------------------------------

ReadonlyTextCtrlWrapper::ReadonlyTextCtrlWrapper(ControlWrapper* parent, const std::string& value,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ReadonlyTextCtrlWrapper::ReadonlyTextCtrlWrapper()\t-> new wxTextCtrl(wxTE_READONLY)\n");
#endif
	m_nativeWidget = new wxTextCtrl(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, value,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style | wxTE_READONLY);
}

// ClickableTextWrapper -----------------------------------------------------------

ClickableTextWrapper::ClickableTextWrapper(ControlWrapper* parent, const std::string& text,
	const Position& pos, const Size& size, long style,
	std::function<void()> onClick)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ClickableTextWrapper::ClickableTextWrapper()\t-> new wxStaticText()\n");
#endif
	auto* st = new wxStaticText(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, text,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	if (onClick)
		st->Bind(wxEVT_LEFT_DOWN, [cb = std::move(onClick)](wxMouseEvent&) { cb(); });
	m_nativeWidget = st;
}

// LinkTextWrapper -----------------------------------------------------------

LinkTextWrapper::LinkTextWrapper(ControlWrapper* parent, const std::string& text,
	const Position& pos, const Size& size, long style,
	std::function<void()> onClick)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "LinkTextWrapper::LinkTextWrapper()\t-> new wxHyperlinkCtrl()\n");
#endif
	auto* hl = new wxHyperlinkCtrl(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, text, wxEmptyString,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	if (onClick)
		hl->Bind(wxEVT_HYPERLINK, [cb = std::move(onClick)](wxHyperlinkEvent& evt) { cb(); });
	m_nativeWidget = hl;
}

// DatePickerWrapper -----------------------------------------------------------

DatePickerWrapper::DatePickerWrapper(ControlWrapper* parent, Date& value,
	const Position& pos, const Size& size, long style,
	std::function<void(const Date&)> onChange)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "DatePickerWrapper::DatePickerWrapper()\t-> new wxDatePickerCtrl()\n");
#endif
	wxDateTime dt;
	dt.Set(static_cast<wxDateTime::wxDateTime_t>(value.day),
		static_cast<wxDateTime::Month>(value.month - 1),
		value.year);
	auto* dp = new wxDatePickerCtrl(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, dt,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	dp->Bind(wxEVT_DATE_CHANGED, [&value, cb = std::move(onChange)](wxDateEvent& evt) {
		const wxDateTime& d = evt.GetDate();
		value.year  = d.GetYear();
		value.month = static_cast<int>(d.GetMonth()) + 1;
		value.day   = d.GetDay();
		if (cb) cb(value);
	});
	m_nativeWidget = dp;
}

DatePickerWrapper::DatePickerWrapper(ControlWrapper* parent, const Date& initialValue,
	const Position& pos, const Size& size, long style,
	std::function<void(const Date&)> onChange)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "DatePickerWrapper::DatePickerWrapper(unbound)\t-> new wxDatePickerCtrl()\n");
#endif
	wxDateTime dt;
	dt.Set(static_cast<wxDateTime::wxDateTime_t>(initialValue.day),
		static_cast<wxDateTime::Month>(initialValue.month - 1),
		initialValue.year);
	auto* dp = new wxDatePickerCtrl(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, dt,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	if (onChange)
		dp->Bind(wxEVT_DATE_CHANGED, [cb = std::move(onChange)](wxDateEvent& evt) {
			const wxDateTime& d = evt.GetDate();
			Date date;
			date.year  = d.GetYear();
			date.month = static_cast<int>(d.GetMonth()) + 1;
			date.day   = d.GetDay();
			cb(date);
		});
	m_nativeWidget = dp;
}

// TimePickerWrapper -----------------------------------------------------------

TimePickerWrapper::TimePickerWrapper(ControlWrapper* parent, Time& value,
	const Position& pos, const Size& size, long style,
	std::function<void(const Time&)> onChange)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "TimePickerWrapper::TimePickerWrapper()\t-> new wxTimePickerCtrl()\n");
#endif
	wxDateTime dt = wxDateTime::Now();
	dt.SetHour(value.hour);
	dt.SetMinute(value.minute);
	dt.SetSecond(value.second);
	auto* tp = new wxTimePickerCtrl(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, dt,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	tp->Bind(wxEVT_TIME_CHANGED, [&value, cb = std::move(onChange)](wxDateEvent& evt) {
		const wxDateTime& d = evt.GetDate();
		value.hour   = d.GetHour();
		value.minute = d.GetMinute();
		value.second = d.GetSecond();
		if (cb) cb(value);
	});
	m_nativeWidget = tp;
}

TimePickerWrapper::TimePickerWrapper(ControlWrapper* parent, const Time& initialValue,
	const Position& pos, const Size& size, long style,
	std::function<void(const Time&)> onChange)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "TimePickerWrapper::TimePickerWrapper(unbound)\t-> new wxTimePickerCtrl()\n");
#endif
	wxDateTime dt = wxDateTime::Now();
	dt.SetHour(initialValue.hour);
	dt.SetMinute(initialValue.minute);
	dt.SetSecond(initialValue.second);
	auto* tp = new wxTimePickerCtrl(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, dt,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	if (onChange)
		tp->Bind(wxEVT_TIME_CHANGED, [cb = std::move(onChange)](wxDateEvent& evt) {
			const wxDateTime& d = evt.GetDate();
			Time time;
			time.hour   = d.GetHour();
			time.minute = d.GetMinute();
			time.second = d.GetSecond();
			cb(time);
		});
	m_nativeWidget = tp;
}

// StaticTextWrapper -----------------------------------------------------------

StaticTextWrapper::StaticTextWrapper(ControlWrapper* parent, const std::string& text,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "StaticTextWrapper::StaticTextWrapper()\t-> new wxStaticText()\n");
#endif
	m_nativeWidget = new wxStaticText(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, text,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
}

// SliderWrapper -----------------------------------------------------------

template <SliderValue T>
SliderWrapper<T>::SliderWrapper(ControlWrapper* parent, Range<T> range, T& value,
	const Position& pos, const Size& size, long style,
	std::function<void(T)> onChange)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "SliderWrapper::SliderWrapper()\t-> new wxSlider()\n");
#endif
	if constexpr (std::is_floating_point_v<T>)
	{
		int iMin = static_cast<int>(range.min / range.step);
		int iMax = static_cast<int>(range.max / range.step);
		int iVal = static_cast<int>(value / range.step);
		auto* sl = new wxSlider(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY,
			iVal, iMin, iMax,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
		sl->Bind(wxEVT_SLIDER, [&value, step = range.step, cb = std::move(onChange)](wxCommandEvent& evt) {
			value = static_cast<T>(evt.GetInt()) * step;
			if (cb) cb(value);
		});
		m_nativeWidget = sl;
	}
	else
	{
		auto* sl = new wxSlider(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY,
			value, range.min, range.max,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
		sl->Bind(wxEVT_SLIDER, [&value, cb = std::move(onChange)](wxCommandEvent& evt) {
			value = evt.GetInt();
			if (cb) cb(value);
		});
		m_nativeWidget = sl;
	}
}

template <SliderValue T>
SliderWrapper<T>::SliderWrapper(ControlWrapper* parent, Range<T> range, const T& initialValue,
	const Position& pos, const Size& size, long style,
	std::function<void(T)> onChange)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "SliderWrapper::SliderWrapper(unbound)\t-> new wxSlider()\n");
#endif
	if constexpr (std::is_floating_point_v<T>)
	{
		int iMin = static_cast<int>(range.min / range.step);
		int iMax = static_cast<int>(range.max / range.step);
		int iVal = static_cast<int>(initialValue / range.step);
		auto* sl = new wxSlider(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY,
			iVal, iMin, iMax,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
		if (onChange)
			sl->Bind(wxEVT_SLIDER, [step = range.step, cb = std::move(onChange)](wxCommandEvent& evt) {
				cb(static_cast<T>(evt.GetInt()) * step);
			});
		m_nativeWidget = sl;
	}
	else
	{
		auto* sl = new wxSlider(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY,
			static_cast<int>(initialValue), range.min, range.max,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
		if (onChange)
			sl->Bind(wxEVT_SLIDER, [cb = std::move(onChange)](wxCommandEvent& evt) {
				cb(static_cast<T>(evt.GetInt()));
			});
		m_nativeWidget = sl;
	}
}

template class SliderWrapper<int>;
template class SliderWrapper<float>;

// SpinBoxWrapper -----------------------------------------------------------

template <SpinBoxValue T>
SpinBoxWrapper<T>::SpinBoxWrapper(ControlWrapper* parent, Range<T> range, T& value,
	const Position& pos, const Size& size, long style,
	std::function<void(T)> onChange)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "SpinBoxWrapper::SpinBoxWrapper()\t-> new wxSpinCtrl[Double]()\n");
#endif
	if constexpr (std::is_same_v<T, int>)
	{
		auto* sc = new wxSpinCtrl(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, wxEmptyString,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style,
			range.min, range.max, value);
		sc->Bind(wxEVT_SPINCTRL, [&value, cb = std::move(onChange)](wxSpinEvent& evt) {
			value = evt.GetInt();
			if (cb) cb(value);
		});
		m_nativeWidget = sc;
	}
	else
	{
		auto* sc = new wxSpinCtrlDouble(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, wxEmptyString,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style,
			range.min, range.max, value, range.step);
		sc->Bind(wxEVT_SPINCTRLDOUBLE, [&value, cb = std::move(onChange)](wxSpinDoubleEvent& evt) {
			value = static_cast<T>(evt.GetValue());
			if (cb) cb(value);
		});
		m_nativeWidget = sc;
	}
}

template <SpinBoxValue T>
SpinBoxWrapper<T>::SpinBoxWrapper(ControlWrapper* parent, Range<T> range, const T& initialValue,
	const Position& pos, const Size& size, long style,
	std::function<void(T)> onChange)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "SpinBoxWrapper::SpinBoxWrapper(unbound)\t-> new wxSpinCtrl[Double]()\n");
#endif
	if constexpr (std::is_same_v<T, int>)
	{
		auto* sc = new wxSpinCtrl(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, wxEmptyString,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style,
			range.min, range.max, static_cast<int>(initialValue));
		if (onChange)
			sc->Bind(wxEVT_SPINCTRL, [cb = std::move(onChange)](wxSpinEvent& evt) { cb(evt.GetInt()); });
		m_nativeWidget = sc;
	}
	else
	{
		auto* sc = new wxSpinCtrlDouble(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, wxEmptyString,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style,
			range.min, range.max, static_cast<double>(initialValue), range.step);
		if (onChange)
			sc->Bind(wxEVT_SPINCTRLDOUBLE, [cb = std::move(onChange)](wxSpinDoubleEvent& evt) {
				cb(static_cast<T>(evt.GetValue()));
			});
		m_nativeWidget = sc;
	}
}

template class SpinBoxWrapper<int>;
template class SpinBoxWrapper<float>;

// RadioButtonWrapper -----------------------------------------------------------

template <RadioButtonValue T>
RadioButtonWrapper<T>::RadioButtonWrapper(ControlWrapper* parent, const std::string& label,
	T& value, const Position& pos, const Size& size, long style,
	std::function<void(T)> onChange)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "RadioButtonWrapper::RadioButtonWrapper()\t-> new wxRadioButton()\n");
#endif
	if constexpr (std::is_same_v<T, bool>)
	{
		auto* rb = new wxRadioButton(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, label,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
		rb->SetValue(value);
		rb->Bind(wxEVT_RADIOBUTTON, [&value, cb = std::move(onChange)](wxCommandEvent&) {
			value = true;
			if (cb) cb(value);
		});
		m_nativeWidget = rb;
	}
	else
	{
		if (&value != s_lastGroup)
		{
			s_radioButtonId = 0;
			s_lastGroup = &value;
		}
		int index = s_radioButtonId++;
		long groupStyle = (index == 0) ? wxRB_GROUP : 0;
		auto* rb = new wxRadioButton(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, label,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style | groupStyle);
		rb->SetValue(value == index);
		rb->Bind(wxEVT_RADIOBUTTON, [&value, index, cb = std::move(onChange)](wxCommandEvent&) {
			value = index;
			if (cb) cb(value);
		});
		m_nativeWidget = rb;
	}
}

template <RadioButtonValue T>
RadioButtonWrapper<T>::RadioButtonWrapper(ControlWrapper* parent, const std::string& label,
	const T& initialValue, const Position& pos, const Size& size, long style,
	std::function<void(T)> onChange)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "RadioButtonWrapper::RadioButtonWrapper(unbound)\t-> new wxRadioButton()\n");
#endif
	if constexpr (std::is_same_v<T, bool>)
	{
		auto* rb = new wxRadioButton(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, label,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
		rb->SetValue(initialValue);
		if (onChange)
			rb->Bind(wxEVT_RADIOBUTTON, [cb = std::move(onChange)](wxCommandEvent&) { cb(true); });
		m_nativeWidget = rb;
	}
	else
	{
		// Unbound int RadioButton: standalone (each is its own group)
		s_radioButtonId = 0;
		s_lastGroup = nullptr;
		int index = s_radioButtonId++;
		auto* rb = new wxRadioButton(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, label,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style | wxRB_GROUP);
		rb->SetValue(static_cast<int>(initialValue) == index);
		if (onChange)
			rb->Bind(wxEVT_RADIOBUTTON, [index, cb = std::move(onChange)](wxCommandEvent&) { cb(static_cast<T>(index)); });
		m_nativeWidget = rb;
	}
}

template class RadioButtonWrapper<bool>;
template class RadioButtonWrapper<int>;

// CheckBoxWrapper -----------------------------------------------------------

CheckBoxWrapper::CheckBoxWrapper(ControlWrapper* parent, const std::string& label,
	const Position& pos, const Size& size, long style,
	bool& checked, std::function<void(bool)> onChange)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "CheckBoxWrapper::CheckBoxWrapper()\t-> new wxCheckBox()\n");
#endif
	auto* cb = new wxCheckBox(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, label,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	cb->SetValue(checked);
	cb->Bind(wxEVT_CHECKBOX, [&checked, cbk = std::move(onChange)](wxCommandEvent& evt) {
		checked = evt.IsChecked();
		if (cbk) cbk(checked);
	});
	m_nativeWidget = cb;
}

CheckBoxWrapper::CheckBoxWrapper(ControlWrapper* parent, const std::string& label,
	const Position& pos, const Size& size, long style,
	const bool& initialChecked, std::function<void(bool)> onChange)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "CheckBoxWrapper::CheckBoxWrapper(unbound)\t-> new wxCheckBox()\n");
#endif
	auto* cb = new wxCheckBox(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, label,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	cb->SetValue(initialChecked);
	if (onChange)
		cb->Bind(wxEVT_CHECKBOX, [cbk = std::move(onChange)](wxCommandEvent& evt) {
			cbk(evt.IsChecked());
		});
	m_nativeWidget = cb;
}

// ToggleButtonWrapper -----------------------------------------------------------

ToggleButtonWrapper::ToggleButtonWrapper(ControlWrapper* parent, const std::string& label,
	bool& toggled, const Position& pos, const Size& size, long style,
	std::function<void(bool)> onChange)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ToggleButtonWrapper::ToggleButtonWrapper()\t-> new wxToggleButton()\n");
#endif
	auto* btn = new wxToggleButton(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, label,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	btn->SetValue(toggled);
	btn->Bind(wxEVT_TOGGLEBUTTON, [&toggled, cb = std::move(onChange)](wxCommandEvent& evt) {
		toggled = evt.IsChecked();
		if (cb) cb(toggled);
	});
	m_nativeWidget = btn;
}

ToggleButtonWrapper::ToggleButtonWrapper(ControlWrapper* parent, const std::string& label,
	const bool& initialToggled, const Position& pos, const Size& size, long style,
	std::function<void(bool)> onChange)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ToggleButtonWrapper::ToggleButtonWrapper(unbound)\t-> new wxToggleButton()\n");
#endif
	auto* btn = new wxToggleButton(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, label,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	btn->SetValue(initialToggled);
	if (onChange)
		btn->Bind(wxEVT_TOGGLEBUTTON, [cb = std::move(onChange)](wxCommandEvent& evt) {
			cb(evt.IsChecked());
		});
	m_nativeWidget = btn;
}

// ImageWrapper -----------------------------------------------------------

ImageWrapper::ImageWrapper(ControlWrapper* parent, const std::string& filePath,
	const Position& pos, const Size& size, long style,
	std::function<void()> onClick,
	std::function<void()> onHover)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ImageWrapper::ImageWrapper()\t-> new wxStaticBitmap()\n");
#endif
	static bool s_handlersInit = false;
	if (!s_handlersInit)
	{
		wxInitAllImageHandlers();
		s_handlersInit = true;
	}
	wxImage wxImg(filePath, wxBITMAP_TYPE_ANY);
	if (wxImg.IsOk() && size.width > 0 && size.height > 0)
		wxImg = wxImg.Scale(size.width, size.height, wxIMAGE_QUALITY_HIGH);
	wxBitmap bmp(wxImg.IsOk() ? wxImg : wxImage(16, 16));
	auto* bmpCtrl = new wxStaticBitmap(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, bmp,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	if (onClick)
		bmpCtrl->Bind(wxEVT_LEFT_DOWN, [cb = std::move(onClick)](wxMouseEvent&) { cb(); });
	if (onHover)
		bmpCtrl->Bind(wxEVT_ENTER_WINDOW, [cb = std::move(onHover)](wxMouseEvent&) { cb(); });
	m_nativeWidget = bmpCtrl;
}

// ColorPickerWrapper -----------------------------------------------------------

ColorPickerWrapper::ColorPickerWrapper(ControlWrapper* parent, Color& value,
	const Position& pos, const Size& size, long style,
	std::function<void(const Color&)> onChange)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ColorPickerWrapper::ColorPickerWrapper()\t-> new wxColourPickerCtrl()\n");
#endif
	auto* picker = new wxColourPickerCtrl(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY,
		wxColour(static_cast<unsigned char>(value.r * 255),
		         static_cast<unsigned char>(value.g * 255),
		         static_cast<unsigned char>(value.b * 255),
		         static_cast<unsigned char>(value.a * 255)),
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	picker->Bind(wxEVT_COLOURPICKER_CHANGED, [&value, cb = std::move(onChange)](wxColourPickerEvent& evt) {
		const wxColour& c = evt.GetColour();
		value = Color{ c.Red() / 255.0f, c.Green() / 255.0f, c.Blue() / 255.0f, c.Alpha() / 255.0f };
		if (cb) cb(value);
	});
	m_nativeWidget = picker;
}

ColorPickerWrapper::ColorPickerWrapper(ControlWrapper* parent, const Color& value,
	const Position& pos, const Size& size, long style,
	std::function<void(const Color&)> onChange)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ColorPickerWrapper::ColorPickerWrapper(unbound)\t-> new wxColourPickerCtrl()\n");
#endif
	auto* picker = new wxColourPickerCtrl(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY,
		wxColour(static_cast<unsigned char>(value.r * 255),
		         static_cast<unsigned char>(value.g * 255),
		         static_cast<unsigned char>(value.b * 255),
		         static_cast<unsigned char>(value.a * 255)),
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	if (onChange)
		picker->Bind(wxEVT_COLOURPICKER_CHANGED, [cb = std::move(onChange)](wxColourPickerEvent& evt) {
			const wxColour& c = evt.GetColour();
			cb(Color{ c.Red() / 255.0f, c.Green() / 255.0f, c.Blue() / 255.0f, c.Alpha() / 255.0f });
		});
	m_nativeWidget = picker;
}

// SeparatorWrapper -----------------------------------------------------------

SeparatorWrapper::SeparatorWrapper(ControlWrapper* parent,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "SeparatorWrapper::SeparatorWrapper()\t-> new wxStaticLine()\n");
#endif
	auto* line = new wxStaticLine(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style | wxLI_HORIZONTAL);
	m_nativeWidget = line;
}

// ProgressBarWrapper -----------------------------------------------------------

ProgressBarWrapper::ProgressBarWrapper(ControlWrapper* parent, const float& value,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ProgressBarWrapper::ProgressBarWrapper()\t-> new wxGauge()\n");
#endif
	auto* gauge = new wxGauge(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, 100,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style | wxGA_HORIZONTAL | wxGA_SMOOTH);
	gauge->SetValue(static_cast<int>(std::clamp(value, 0.0f, 1.0f) * 100));
	m_nativeWidget = gauge;
}

ProgressBarWrapper::ProgressBarWrapper(ControlWrapper* parent, float& value,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ProgressBarWrapper::ProgressBarWrapper()\t-> new wxGauge()\n");
#endif
	auto* gauge = new wxGauge(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, 100,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style | wxGA_HORIZONTAL | wxGA_SMOOTH);
	gauge->SetValue(static_cast<int>(std::clamp(value, 0.0f, 1.0f) * 100));
	m_nativeWidget = gauge;
}

// ComboBoxWrapper -----------------------------------------------------------

template <ComboBoxValue T>
ComboBoxWrapper<T>::ComboBoxWrapper(ControlWrapper* parent, std::vector<std::string> choices,
	T& selected, const Position& pos, const Size& size, long style,
	std::function<void(const T&)> onChange)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ComboBoxWrapper::ComboBoxWrapper()\t-> new wxComboBox()\n");
#endif
	wxArrayString items;
	for (const auto& c : choices)
		items.Add(c);
	auto* combo = new wxComboBox(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, "",
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), items, style);

	if constexpr (std::is_same_v<T, std::string>)
	{
		combo->SetValue(selected);
		combo->Bind(wxEVT_COMBOBOX, [&selected, cb = std::move(onChange)](wxCommandEvent& evt) {
			selected = evt.GetString().ToStdString();
			if (cb) cb(selected);
		});
	}
	else
	{
		combo->SetSelection(selected);
		combo->Bind(wxEVT_COMBOBOX, [&selected, cb = std::move(onChange)](wxCommandEvent& evt) {
			selected = evt.GetSelection();
			if (cb) cb(selected);
		});
	}
	m_nativeWidget = combo;
}

template <ComboBoxValue T>
ComboBoxWrapper<T>::ComboBoxWrapper(ControlWrapper* parent, std::vector<std::string> choices,
	const T& selected, const Position& pos, const Size& size, long style,
	std::function<void(const T&)> onChange)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ComboBoxWrapper::ComboBoxWrapper(unbound)\t-> new wxComboBox()\n");
#endif
	wxArrayString items;
	for (const auto& c : choices)
		items.Add(c);
	auto* combo = new wxComboBox(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, "",
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), items, style);

	if constexpr (std::is_same_v<T, std::string>)
	{
		combo->SetValue(selected);
		if (onChange)
			combo->Bind(wxEVT_COMBOBOX, [cb = std::move(onChange)](wxCommandEvent& evt) {
				cb(evt.GetString().ToStdString());
			});
	}
	else
	{
		combo->SetSelection(selected);
		if (onChange)
			combo->Bind(wxEVT_COMBOBOX, [cb = std::move(onChange)](wxCommandEvent& evt) {
				T val = static_cast<T>(evt.GetSelection());
				cb(val);
			});
	}
	m_nativeWidget = combo;
}

template class ComboBoxWrapper<std::string>;
template class ComboBoxWrapper<int>;
