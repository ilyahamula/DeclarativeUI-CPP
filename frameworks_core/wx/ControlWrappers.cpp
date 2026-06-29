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

void ButtonWrapper::createNativeObject(ControlWrapper* parent, const std::string& label,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ButtonWrapper::ButtonWrapper()\t-> new wxButton()\n");
#endif
	auto* btn = new wxButton(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, label,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	m_nativeWidget = btn;
}

ButtonWrapper::ButtonWrapper(ControlWrapper* parent, const std::string& label,
	const Position& pos, const Size& size, long style,
	std::function<void()> onClick)
{
	createNativeObject(parent, label, pos, size, style);
	if (onClick)
		reinterpret_cast<wxButton*>(m_nativeWidget)->Bind(wxEVT_BUTTON, [cb = std::move(onClick)](wxCommandEvent&) { cb(); });
	
}

ButtonWrapper::ButtonWrapper(ControlWrapper* parent, const std::string& label,
	const Position& pos, const Size& size, long style,
	std::function<void(void*)> onClick)
{
	createNativeObject(parent, label, pos, size, style);
	if (onClick)
		reinterpret_cast<wxButton*>(m_nativeWidget)->Bind(wxEVT_BUTTON,
			[cb = std::move(onClick), nativeWgd = m_nativeWidget](wxCommandEvent&) { cb(nativeWgd); });
	
}

// TextCtrlWrapper -----------------------------------------------------------

void TextCtrlWrapper::createNativeObject(ControlWrapper* parent, const std::string& value,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "TextCtrlWrapper::TextCtrlWrapper()\t-> new wxTextCtrl()\n");
#endif
	m_nativeWidget = new wxTextCtrl(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, value,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
}

TextCtrlWrapper::TextCtrlWrapper(ControlWrapper* parent, std::string& value,
	const Position& pos, const Size& size, long style,
	std::function<void(const std::string&)> onChange)
{
	createNativeObject(parent, value, pos, size, style);
	reinterpret_cast<wxTextCtrl*>(m_nativeWidget)->Bind(wxEVT_TEXT, [&value, cb = std::move(onChange)](wxCommandEvent& evt) {
		value = evt.GetString().ToStdString();
		if (cb) cb(value);
	});
}

TextCtrlWrapper::TextCtrlWrapper(ControlWrapper* parent, const std::string& initialValue,
	const Position& pos, const Size& size, long style,
	std::function<void(const std::string&)> onChange)
{
	createNativeObject(parent, initialValue, pos, size, style);
	if (onChange)
		reinterpret_cast<wxTextCtrl*>(m_nativeWidget)->Bind(wxEVT_TEXT, [cb = std::move(onChange)](wxCommandEvent& evt) {
			cb(evt.GetString().ToStdString());
		});
}

TextCtrlWrapper::TextCtrlWrapper(ControlWrapper* parent, std::string& value,
	const Position& pos, const Size& size, long style,
	std::function<void(const std::string&, void*)> onChange)
{
	createNativeObject(parent, value, pos, size, style);
	reinterpret_cast<wxTextCtrl*>(m_nativeWidget)->Bind(wxEVT_TEXT, [&value, cb = std::move(onChange), nativeWgd = m_nativeWidget](wxCommandEvent& evt) {
		value = evt.GetString().ToStdString();
		if (cb) cb(value, nativeWgd);
	});
}

TextCtrlWrapper::TextCtrlWrapper(ControlWrapper* parent, const std::string& initialValue,
	const Position& pos, const Size& size, long style,
	std::function<void(const std::string&, void*)> onChange)
{
	createNativeObject(parent, initialValue, pos, size, style);
	if (onChange)
		reinterpret_cast<wxTextCtrl*>(m_nativeWidget)->Bind(wxEVT_TEXT, [cb = std::move(onChange), nativeWgd = m_nativeWidget](wxCommandEvent& evt) {
			cb(evt.GetString().ToStdString(), nativeWgd);
		});
}

// PasswordInputWrapper -----------------------------------------------------------

void PasswordInputWrapper::createNativeObject(ControlWrapper* parent, const std::string& value,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "PasswordInputWrapper::PasswordInputWrapper()\t-> new wxTextCtrl(wxTE_PASSWORD)\n");
#endif
	m_nativeWidget = new wxTextCtrl(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, value,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style | wxTE_PASSWORD);
}

PasswordInputWrapper::PasswordInputWrapper(ControlWrapper* parent, std::string& value,
	const Position& pos, const Size& size, long style,
	std::function<void(const std::string&)> onChange)
{
	createNativeObject(parent, value, pos, size, style);
	reinterpret_cast<wxTextCtrl*>(m_nativeWidget)->Bind(wxEVT_TEXT, [&value, cb = std::move(onChange)](wxCommandEvent& evt) {
		value = evt.GetString().ToStdString();
		if (cb) cb(value);
	});
}

PasswordInputWrapper::PasswordInputWrapper(ControlWrapper* parent, const std::string& initialValue,
	const Position& pos, const Size& size, long style,
	std::function<void(const std::string&)> onChange)
{
	createNativeObject(parent, initialValue, pos, size, style);
	if (onChange)
		reinterpret_cast<wxTextCtrl*>(m_nativeWidget)->Bind(wxEVT_TEXT, [cb = std::move(onChange)](wxCommandEvent& evt) {
			cb(evt.GetString().ToStdString());
		});
}

PasswordInputWrapper::PasswordInputWrapper(ControlWrapper* parent, std::string& value,
	const Position& pos, const Size& size, long style,
	std::function<void(const std::string&, void*)> onChange)
{
	createNativeObject(parent, value, pos, size, style);
	reinterpret_cast<wxTextCtrl*>(m_nativeWidget)->Bind(wxEVT_TEXT, [&value, cb = std::move(onChange), nativeWgd = m_nativeWidget](wxCommandEvent& evt) {
		value = evt.GetString().ToStdString();
		if (cb) cb(value, nativeWgd);
	});
}

PasswordInputWrapper::PasswordInputWrapper(ControlWrapper* parent, const std::string& initialValue,
	const Position& pos, const Size& size, long style,
	std::function<void(const std::string&, void*)> onChange)
{
	createNativeObject(parent, initialValue, pos, size, style);
	if (onChange)
		reinterpret_cast<wxTextCtrl*>(m_nativeWidget)->Bind(wxEVT_TEXT, [cb = std::move(onChange), nativeWgd = m_nativeWidget](wxCommandEvent& evt) {
			cb(evt.GetString().ToStdString(), nativeWgd);
		});
}

// MultiLineTextCtrlWrapper -----------------------------------------------------------

void MultiLineTextCtrlWrapper::createNativeObject(ControlWrapper* parent, const std::string& value,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "MultiLineTextCtrlWrapper::MultiLineTextCtrlWrapper()\t-> new wxTextCtrl(wxTE_MULTILINE)\n");
#endif
	m_nativeWidget = new wxTextCtrl(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, value,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style | wxTE_MULTILINE);
}

MultiLineTextCtrlWrapper::MultiLineTextCtrlWrapper(ControlWrapper* parent, std::string& value,
	const Position& pos, const Size& size, long style,
	std::function<void(const std::string&)> onChange)
{
	createNativeObject(parent, value, pos, size, style);
	reinterpret_cast<wxTextCtrl*>(m_nativeWidget)->Bind(wxEVT_TEXT, [&value, cb = std::move(onChange)](wxCommandEvent& evt) {
		value = evt.GetString().ToStdString();
		if (cb) cb(value);
	});
}

MultiLineTextCtrlWrapper::MultiLineTextCtrlWrapper(ControlWrapper* parent, const std::string& initialValue,
	const Position& pos, const Size& size, long style,
	std::function<void(const std::string&)> onChange)
{
	createNativeObject(parent, initialValue, pos, size, style);
	if (onChange)
		reinterpret_cast<wxTextCtrl*>(m_nativeWidget)->Bind(wxEVT_TEXT, [cb = std::move(onChange)](wxCommandEvent& evt) {
			cb(evt.GetString().ToStdString());
		});
}

MultiLineTextCtrlWrapper::MultiLineTextCtrlWrapper(ControlWrapper* parent, std::string& value,
	const Position& pos, const Size& size, long style,
	std::function<void(const std::string&, void*)> onChange)
{
	createNativeObject(parent, value, pos, size, style);
	reinterpret_cast<wxTextCtrl*>(m_nativeWidget)->Bind(wxEVT_TEXT, [&value, cb = std::move(onChange), nativeWgd = m_nativeWidget](wxCommandEvent& evt) {
		value = evt.GetString().ToStdString();
		if (cb) cb(value, nativeWgd);
	});
}

MultiLineTextCtrlWrapper::MultiLineTextCtrlWrapper(ControlWrapper* parent, const std::string& initialValue,
	const Position& pos, const Size& size, long style,
	std::function<void(const std::string&, void*)> onChange)
{
	createNativeObject(parent, initialValue, pos, size, style);
	if (onChange)
		reinterpret_cast<wxTextCtrl*>(m_nativeWidget)->Bind(wxEVT_TEXT, [cb = std::move(onChange), nativeWgd = m_nativeWidget](wxCommandEvent& evt) {
			cb(evt.GetString().ToStdString(), nativeWgd);
		});
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

void ClickableTextWrapper::createNativeObject(ControlWrapper* parent, const std::string& text,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ClickableTextWrapper::ClickableTextWrapper()\t-> new wxStaticText()\n");
#endif
	m_nativeWidget = new wxStaticText(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, text,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
}

ClickableTextWrapper::ClickableTextWrapper(ControlWrapper* parent, const std::string& text,
	const Position& pos, const Size& size, long style,
	std::function<void()> onClick)
{
	createNativeObject(parent, text, pos, size, style);
	if (onClick)
		reinterpret_cast<wxStaticText*>(m_nativeWidget)->Bind(wxEVT_LEFT_DOWN, [cb = std::move(onClick)](wxMouseEvent&) { cb(); });
}

ClickableTextWrapper::ClickableTextWrapper(ControlWrapper* parent, const std::string& text,
	const Position& pos, const Size& size, long style,
	std::function<void(void*)> onClick)
{
	createNativeObject(parent, text, pos, size, style);
	if (onClick)
		reinterpret_cast<wxStaticText*>(m_nativeWidget)->Bind(wxEVT_LEFT_DOWN, [cb = std::move(onClick), nativeWgd = m_nativeWidget](wxMouseEvent&) { cb(nativeWgd); });
}

// LinkTextWrapper -----------------------------------------------------------

void LinkTextWrapper::createNativeObject(ControlWrapper* parent, const std::string& text,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "LinkTextWrapper::LinkTextWrapper()\t-> new wxHyperlinkCtrl()\n");
#endif
	m_nativeWidget = new wxHyperlinkCtrl(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, text, wxEmptyString,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
}

LinkTextWrapper::LinkTextWrapper(ControlWrapper* parent, const std::string& text,
	const Position& pos, const Size& size, long style,
	std::function<void()> onClick)
{
	createNativeObject(parent, text, pos, size, style);
	if (onClick)
		reinterpret_cast<wxHyperlinkCtrl*>(m_nativeWidget)->Bind(wxEVT_HYPERLINK, [cb = std::move(onClick)](wxHyperlinkEvent& evt) { cb(); });
}

LinkTextWrapper::LinkTextWrapper(ControlWrapper* parent, const std::string& text,
	const Position& pos, const Size& size, long style,
	std::function<void(void*)> onClick)
{
	createNativeObject(parent, text, pos, size, style);
	if (onClick)
		reinterpret_cast<wxHyperlinkCtrl*>(m_nativeWidget)->Bind(wxEVT_HYPERLINK, [cb = std::move(onClick), nativeWgd = m_nativeWidget](wxHyperlinkEvent& evt) { cb(nativeWgd); });
}

// DatePickerWrapper -----------------------------------------------------------

void DatePickerWrapper::createNativeObject(ControlWrapper* parent, const Date& value,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "DatePickerWrapper::DatePickerWrapper()\t-> new wxDatePickerCtrl()\n");
#endif
	wxDateTime dt;
	dt.Set(static_cast<wxDateTime::wxDateTime_t>(value.day),
		static_cast<wxDateTime::Month>(value.month - 1),
		value.year);
	m_nativeWidget = new wxDatePickerCtrl(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, dt,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
}

DatePickerWrapper::DatePickerWrapper(ControlWrapper* parent, Date& value,
	const Position& pos, const Size& size, long style,
	std::function<void(const Date&)> onChange)
{
	createNativeObject(parent, value, pos, size, style);
	reinterpret_cast<wxDatePickerCtrl*>(m_nativeWidget)->Bind(wxEVT_DATE_CHANGED, [&value, cb = std::move(onChange)](wxDateEvent& evt) {
		const wxDateTime& d = evt.GetDate();
		value.year  = d.GetYear();
		value.month = static_cast<int>(d.GetMonth()) + 1;
		value.day   = d.GetDay();
		if (cb) cb(value);
	});
}

DatePickerWrapper::DatePickerWrapper(ControlWrapper* parent, const Date& initialValue,
	const Position& pos, const Size& size, long style,
	std::function<void(const Date&)> onChange)
{
	createNativeObject(parent, initialValue, pos, size, style);
	if (onChange)
		reinterpret_cast<wxDatePickerCtrl*>(m_nativeWidget)->Bind(wxEVT_DATE_CHANGED, [cb = std::move(onChange)](wxDateEvent& evt) {
			const wxDateTime& d = evt.GetDate();
			Date date;
			date.year  = d.GetYear();
			date.month = static_cast<int>(d.GetMonth()) + 1;
			date.day   = d.GetDay();
			cb(date);
		});
}

DatePickerWrapper::DatePickerWrapper(ControlWrapper* parent, Date& value,
	const Position& pos, const Size& size, long style,
	std::function<void(const Date&, void*)> onChange)
{
	createNativeObject(parent, value, pos, size, style);
	reinterpret_cast<wxDatePickerCtrl*>(m_nativeWidget)->Bind(wxEVT_DATE_CHANGED, [&value, cb = std::move(onChange), nativeWgd = m_nativeWidget](wxDateEvent& evt) {
		const wxDateTime& d = evt.GetDate();
		value.year  = d.GetYear();
		value.month = static_cast<int>(d.GetMonth()) + 1;
		value.day   = d.GetDay();
		if (cb) cb(value, nativeWgd);
	});
}

DatePickerWrapper::DatePickerWrapper(ControlWrapper* parent, const Date& initialValue,
	const Position& pos, const Size& size, long style,
	std::function<void(const Date&, void*)> onChange)
{
	createNativeObject(parent, initialValue, pos, size, style);
	if (onChange)
		reinterpret_cast<wxDatePickerCtrl*>(m_nativeWidget)->Bind(wxEVT_DATE_CHANGED, [cb = std::move(onChange), nativeWgd = m_nativeWidget](wxDateEvent& evt) {
			const wxDateTime& d = evt.GetDate();
			Date date;
			date.year  = d.GetYear();
			date.month = static_cast<int>(d.GetMonth()) + 1;
			date.day   = d.GetDay();
			cb(date, nativeWgd);
		});
}

// TimePickerWrapper -----------------------------------------------------------

void TimePickerWrapper::createNativeObject(ControlWrapper* parent, const Time& value,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "TimePickerWrapper::TimePickerWrapper()\t-> new wxTimePickerCtrl()\n");
#endif
	wxDateTime dt = wxDateTime::Now();
	dt.SetHour(value.hour);
	dt.SetMinute(value.minute);
	dt.SetSecond(value.second);
	m_nativeWidget = new wxTimePickerCtrl(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, dt,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
}

TimePickerWrapper::TimePickerWrapper(ControlWrapper* parent, Time& value,
	const Position& pos, const Size& size, long style,
	std::function<void(const Time&)> onChange)
{
	createNativeObject(parent, value, pos, size, style);
	reinterpret_cast<wxTimePickerCtrl*>(m_nativeWidget)->Bind(wxEVT_TIME_CHANGED, [&value, cb = std::move(onChange)](wxDateEvent& evt) {
		const wxDateTime& d = evt.GetDate();
		value.hour   = d.GetHour();
		value.minute = d.GetMinute();
		value.second = d.GetSecond();
		if (cb) cb(value);
	});
}

TimePickerWrapper::TimePickerWrapper(ControlWrapper* parent, const Time& initialValue,
	const Position& pos, const Size& size, long style,
	std::function<void(const Time&)> onChange)
{
	createNativeObject(parent, initialValue, pos, size, style);
	if (onChange)
		reinterpret_cast<wxTimePickerCtrl*>(m_nativeWidget)->Bind(wxEVT_TIME_CHANGED, [cb = std::move(onChange)](wxDateEvent& evt) {
			const wxDateTime& d = evt.GetDate();
			Time time;
			time.hour   = d.GetHour();
			time.minute = d.GetMinute();
			time.second = d.GetSecond();
			cb(time);
		});
}

TimePickerWrapper::TimePickerWrapper(ControlWrapper* parent, Time& value,
	const Position& pos, const Size& size, long style,
	std::function<void(const Time&, void*)> onChange)
{
	createNativeObject(parent, value, pos, size, style);
	reinterpret_cast<wxTimePickerCtrl*>(m_nativeWidget)->Bind(wxEVT_TIME_CHANGED, [&value, cb = std::move(onChange), nativeWgd = m_nativeWidget](wxDateEvent& evt) {
		const wxDateTime& d = evt.GetDate();
		value.hour   = d.GetHour();
		value.minute = d.GetMinute();
		value.second = d.GetSecond();
		if (cb) cb(value, nativeWgd);
	});
}

TimePickerWrapper::TimePickerWrapper(ControlWrapper* parent, const Time& initialValue,
	const Position& pos, const Size& size, long style,
	std::function<void(const Time&, void*)> onChange)
{
	createNativeObject(parent, initialValue, pos, size, style);
	if (onChange)
		reinterpret_cast<wxTimePickerCtrl*>(m_nativeWidget)->Bind(wxEVT_TIME_CHANGED, [cb = std::move(onChange), nativeWgd = m_nativeWidget](wxDateEvent& evt) {
			const wxDateTime& d = evt.GetDate();
			Time time;
			time.hour   = d.GetHour();
			time.minute = d.GetMinute();
			time.second = d.GetSecond();
			cb(time, nativeWgd);
		});
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
void SliderWrapper<T>::createNativeObject(ControlWrapper* parent, Range<T> range, const T& value,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "SliderWrapper::SliderWrapper()\t-> new wxSlider()\n");
#endif
	if constexpr (std::is_floating_point_v<T>)
	{
		int iMin = static_cast<int>(range.min / range.step);
		int iMax = static_cast<int>(range.max / range.step);
		int iVal = static_cast<int>(value / range.step);
		m_nativeWidget = new wxSlider(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY,
			iVal, iMin, iMax,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	}
	else
	{
		m_nativeWidget = new wxSlider(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY,
			static_cast<int>(value), range.min, range.max,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	}
}

template <SliderValue T>
SliderWrapper<T>::SliderWrapper(ControlWrapper* parent, Range<T> range, T& value,
	const Position& pos, const Size& size, long style,
	std::function<void(T)> onChange)
{
	createNativeObject(parent, range, value, pos, size, style);
	auto* sl = reinterpret_cast<wxSlider*>(m_nativeWidget);
	if constexpr (std::is_floating_point_v<T>)
		sl->Bind(wxEVT_SLIDER, [&value, step = range.step, cb = std::move(onChange)](wxCommandEvent& evt) {
			value = static_cast<T>(evt.GetInt()) * step;
			if (cb) cb(value);
		});
	else
		sl->Bind(wxEVT_SLIDER, [&value, cb = std::move(onChange)](wxCommandEvent& evt) {
			value = evt.GetInt();
			if (cb) cb(value);
		});
}

template <SliderValue T>
SliderWrapper<T>::SliderWrapper(ControlWrapper* parent, Range<T> range, const T& initialValue,
	const Position& pos, const Size& size, long style,
	std::function<void(T)> onChange)
{
	createNativeObject(parent, range, initialValue, pos, size, style);
	if (!onChange)
		return;
	auto* sl = reinterpret_cast<wxSlider*>(m_nativeWidget);
	if constexpr (std::is_floating_point_v<T>)
		sl->Bind(wxEVT_SLIDER, [step = range.step, cb = std::move(onChange)](wxCommandEvent& evt) {
			cb(static_cast<T>(evt.GetInt()) * step);
		});
	else
		sl->Bind(wxEVT_SLIDER, [cb = std::move(onChange)](wxCommandEvent& evt) {
			cb(static_cast<T>(evt.GetInt()));
		});
}

template <SliderValue T>
SliderWrapper<T>::SliderWrapper(ControlWrapper* parent, Range<T> range, T& value,
	const Position& pos, const Size& size, long style,
	std::function<void(T, void*)> onChange)
{
	createNativeObject(parent, range, value, pos, size, style);
	auto* sl = reinterpret_cast<wxSlider*>(m_nativeWidget);
	if constexpr (std::is_floating_point_v<T>)
		sl->Bind(wxEVT_SLIDER, [&value, step = range.step, cb = std::move(onChange), nativeWgd = m_nativeWidget](wxCommandEvent& evt) {
			value = static_cast<T>(evt.GetInt()) * step;
			if (cb) cb(value, nativeWgd);
		});
	else
		sl->Bind(wxEVT_SLIDER, [&value, cb = std::move(onChange), nativeWgd = m_nativeWidget](wxCommandEvent& evt) {
			value = evt.GetInt();
			if (cb) cb(value, nativeWgd);
		});
}

template <SliderValue T>
SliderWrapper<T>::SliderWrapper(ControlWrapper* parent, Range<T> range, const T& initialValue,
	const Position& pos, const Size& size, long style,
	std::function<void(T, void*)> onChange)
{
	createNativeObject(parent, range, initialValue, pos, size, style);
	if (!onChange)
		return;
	auto* sl = reinterpret_cast<wxSlider*>(m_nativeWidget);
	if constexpr (std::is_floating_point_v<T>)
		sl->Bind(wxEVT_SLIDER, [step = range.step, cb = std::move(onChange), nativeWgd = m_nativeWidget](wxCommandEvent& evt) {
			cb(static_cast<T>(evt.GetInt()) * step, nativeWgd);
		});
	else
		sl->Bind(wxEVT_SLIDER, [cb = std::move(onChange), nativeWgd = m_nativeWidget](wxCommandEvent& evt) {
			cb(static_cast<T>(evt.GetInt()), nativeWgd);
		});
}

template class SliderWrapper<int>;
template class SliderWrapper<float>;

// SpinBoxWrapper -----------------------------------------------------------

template <SpinBoxValue T>
void SpinBoxWrapper<T>::createNativeObject(ControlWrapper* parent, Range<T> range, const T& value,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "SpinBoxWrapper::SpinBoxWrapper()\t-> new wxSpinCtrl[Double]()\n");
#endif
	if constexpr (std::is_same_v<T, int>)
	{
		m_nativeWidget = new wxSpinCtrl(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, wxEmptyString,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style,
			range.min, range.max, static_cast<int>(value));
	}
	else
	{
		m_nativeWidget = new wxSpinCtrlDouble(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, wxEmptyString,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style,
			range.min, range.max, static_cast<double>(value), range.step);
	}
}

template <SpinBoxValue T>
SpinBoxWrapper<T>::SpinBoxWrapper(ControlWrapper* parent, Range<T> range, T& value,
	const Position& pos, const Size& size, long style,
	std::function<void(T)> onChange)
{
	createNativeObject(parent, range, value, pos, size, style);
	if constexpr (std::is_same_v<T, int>)
		reinterpret_cast<wxSpinCtrl*>(m_nativeWidget)->Bind(wxEVT_SPINCTRL, [&value, cb = std::move(onChange)](wxSpinEvent& evt) {
			value = evt.GetInt();
			if (cb) cb(value);
		});
	else
		reinterpret_cast<wxSpinCtrlDouble*>(m_nativeWidget)->Bind(wxEVT_SPINCTRLDOUBLE, [&value, cb = std::move(onChange)](wxSpinDoubleEvent& evt) {
			value = static_cast<T>(evt.GetValue());
			if (cb) cb(value);
		});
}

template <SpinBoxValue T>
SpinBoxWrapper<T>::SpinBoxWrapper(ControlWrapper* parent, Range<T> range, const T& initialValue,
	const Position& pos, const Size& size, long style,
	std::function<void(T)> onChange)
{
	createNativeObject(parent, range, initialValue, pos, size, style);
	if (!onChange)
		return;
	if constexpr (std::is_same_v<T, int>)
		reinterpret_cast<wxSpinCtrl*>(m_nativeWidget)->Bind(wxEVT_SPINCTRL, [cb = std::move(onChange)](wxSpinEvent& evt) { cb(evt.GetInt()); });
	else
		reinterpret_cast<wxSpinCtrlDouble*>(m_nativeWidget)->Bind(wxEVT_SPINCTRLDOUBLE, [cb = std::move(onChange)](wxSpinDoubleEvent& evt) {
			cb(static_cast<T>(evt.GetValue()));
		});
}

template <SpinBoxValue T>
SpinBoxWrapper<T>::SpinBoxWrapper(ControlWrapper* parent, Range<T> range, T& value,
	const Position& pos, const Size& size, long style,
	std::function<void(T, void*)> onChange)
{
	createNativeObject(parent, range, value, pos, size, style);
	if constexpr (std::is_same_v<T, int>)
		reinterpret_cast<wxSpinCtrl*>(m_nativeWidget)->Bind(wxEVT_SPINCTRL, [&value, cb = std::move(onChange), nativeWgd = m_nativeWidget](wxSpinEvent& evt) {
			value = evt.GetInt();
			if (cb) cb(value, nativeWgd);
		});
	else
		reinterpret_cast<wxSpinCtrlDouble*>(m_nativeWidget)->Bind(wxEVT_SPINCTRLDOUBLE, [&value, cb = std::move(onChange), nativeWgd = m_nativeWidget](wxSpinDoubleEvent& evt) {
			value = static_cast<T>(evt.GetValue());
			if (cb) cb(value, nativeWgd);
		});
}

template <SpinBoxValue T>
SpinBoxWrapper<T>::SpinBoxWrapper(ControlWrapper* parent, Range<T> range, const T& initialValue,
	const Position& pos, const Size& size, long style,
	std::function<void(T, void*)> onChange)
{
	createNativeObject(parent, range, initialValue, pos, size, style);
	if (!onChange)
		return;
	if constexpr (std::is_same_v<T, int>)
		reinterpret_cast<wxSpinCtrl*>(m_nativeWidget)->Bind(wxEVT_SPINCTRL, [cb = std::move(onChange), nativeWgd = m_nativeWidget](wxSpinEvent& evt) { cb(evt.GetInt(), nativeWgd); });
	else
		reinterpret_cast<wxSpinCtrlDouble*>(m_nativeWidget)->Bind(wxEVT_SPINCTRLDOUBLE, [cb = std::move(onChange), nativeWgd = m_nativeWidget](wxSpinDoubleEvent& evt) {
			cb(static_cast<T>(evt.GetValue()), nativeWgd);
		});
}

template class SpinBoxWrapper<int>;
template class SpinBoxWrapper<float>;

// RadioButtonWrapper -----------------------------------------------------------

template <RadioButtonValue T>
int RadioButtonWrapper<T>::createNativeObject(ControlWrapper* parent, const std::string& label,
	const T& value, const Position& pos, const Size& size, long style, bool standalone)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "RadioButtonWrapper::RadioButtonWrapper()\t-> new wxRadioButton()\n");
#endif
	if constexpr (std::is_same_v<T, bool>)
	{
		auto* rb = new wxRadioButton(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, label,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
		rb->SetValue(value);
		m_nativeWidget = rb;
		return 0;
	}
	else
	{
		if (standalone)
		{
			// Standalone int RadioButton: each is its own group
			s_radioButtonId = 0;
			s_lastGroup = nullptr;
		}
		else if (const_cast<int*>(&value) != s_lastGroup)
		{
			s_radioButtonId = 0;
			s_lastGroup = const_cast<int*>(&value);
		}
		int index = s_radioButtonId++;
		long groupStyle = (index == 0) ? wxRB_GROUP : 0;
		auto* rb = new wxRadioButton(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, label,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style | groupStyle);
		rb->SetValue(static_cast<int>(value) == index);
		m_nativeWidget = rb;
		return index;
	}
}

template <RadioButtonValue T>
RadioButtonWrapper<T>::RadioButtonWrapper(ControlWrapper* parent, const std::string& label,
	T& value, const Position& pos, const Size& size, long style,
	std::function<void(T)> onChange)
{
	[[maybe_unused]] int index = createNativeObject(parent, label, value, pos, size, style, /*standalone=*/false);
	auto* rb = reinterpret_cast<wxRadioButton*>(m_nativeWidget);
	if constexpr (std::is_same_v<T, bool>)
		rb->Bind(wxEVT_RADIOBUTTON, [&value, cb = std::move(onChange)](wxCommandEvent&) {
			value = true;
			if (cb) cb(value);
		});
	else
		rb->Bind(wxEVT_RADIOBUTTON, [&value, index, cb = std::move(onChange)](wxCommandEvent&) {
			value = index;
			if (cb) cb(value);
		});
}

template <RadioButtonValue T>
RadioButtonWrapper<T>::RadioButtonWrapper(ControlWrapper* parent, const std::string& label,
	const T& initialValue, const Position& pos, const Size& size, long style,
	std::function<void(T)> onChange)
{
	[[maybe_unused]] int index = createNativeObject(parent, label, initialValue, pos, size, style, /*standalone=*/true);
	if (!onChange)
		return;
	auto* rb = reinterpret_cast<wxRadioButton*>(m_nativeWidget);
	if constexpr (std::is_same_v<T, bool>)
		rb->Bind(wxEVT_RADIOBUTTON, [cb = std::move(onChange)](wxCommandEvent&) { cb(true); });
	else
		rb->Bind(wxEVT_RADIOBUTTON, [index, cb = std::move(onChange)](wxCommandEvent&) { cb(static_cast<T>(index)); });
}

template <RadioButtonValue T>
RadioButtonWrapper<T>::RadioButtonWrapper(ControlWrapper* parent, const std::string& label,
	T& value, const Position& pos, const Size& size, long style,
	std::function<void(T, void*)> onChange)
{
	[[maybe_unused]] int index = createNativeObject(parent, label, value, pos, size, style, /*standalone=*/false);
	auto* rb = reinterpret_cast<wxRadioButton*>(m_nativeWidget);
	if constexpr (std::is_same_v<T, bool>)
		rb->Bind(wxEVT_RADIOBUTTON, [&value, cb = std::move(onChange), nativeWgd = m_nativeWidget](wxCommandEvent&) {
			value = true;
			if (cb) cb(value, nativeWgd);
		});
	else
		rb->Bind(wxEVT_RADIOBUTTON, [&value, index, cb = std::move(onChange), nativeWgd = m_nativeWidget](wxCommandEvent&) {
			value = index;
			if (cb) cb(value, nativeWgd);
		});
}

template <RadioButtonValue T>
RadioButtonWrapper<T>::RadioButtonWrapper(ControlWrapper* parent, const std::string& label,
	const T& initialValue, const Position& pos, const Size& size, long style,
	std::function<void(T, void*)> onChange)
{
	[[maybe_unused]] int index = createNativeObject(parent, label, initialValue, pos, size, style, /*standalone=*/true);
	if (!onChange)
		return;
	auto* rb = reinterpret_cast<wxRadioButton*>(m_nativeWidget);
	if constexpr (std::is_same_v<T, bool>)
		rb->Bind(wxEVT_RADIOBUTTON, [cb = std::move(onChange), nativeWgd = m_nativeWidget](wxCommandEvent&) { cb(true, nativeWgd); });
	else
		rb->Bind(wxEVT_RADIOBUTTON, [index, cb = std::move(onChange), nativeWgd = m_nativeWidget](wxCommandEvent&) { cb(static_cast<T>(index), nativeWgd); });
}

template class RadioButtonWrapper<bool>;
template class RadioButtonWrapper<int>;

// CheckBoxWrapper -----------------------------------------------------------

void CheckBoxWrapper::createNativeObject(ControlWrapper* parent, const std::string& label,
	const Position& pos, const Size& size, long style, const bool& checked)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "CheckBoxWrapper::CheckBoxWrapper()\t-> new wxCheckBox()\n");
#endif
	auto* cb = new wxCheckBox(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, label,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	cb->SetValue(checked);
	m_nativeWidget = cb;
}

CheckBoxWrapper::CheckBoxWrapper(ControlWrapper* parent, const std::string& label,
	const Position& pos, const Size& size, long style,
	bool& checked, std::function<void(bool)> onChange)
{
	createNativeObject(parent, label, pos, size, style, checked);
	reinterpret_cast<wxCheckBox*>(m_nativeWidget)->Bind(wxEVT_CHECKBOX, [&checked, cbk = std::move(onChange)](wxCommandEvent& evt) {
		checked = evt.IsChecked();
		if (cbk) cbk(checked);
	});
}

CheckBoxWrapper::CheckBoxWrapper(ControlWrapper* parent, const std::string& label,
	const Position& pos, const Size& size, long style,
	const bool& initialChecked, std::function<void(bool)> onChange)
{
	createNativeObject(parent, label, pos, size, style, initialChecked);
	if (onChange)
		reinterpret_cast<wxCheckBox*>(m_nativeWidget)->Bind(wxEVT_CHECKBOX, [cbk = std::move(onChange)](wxCommandEvent& evt) {
			cbk(evt.IsChecked());
		});
}

CheckBoxWrapper::CheckBoxWrapper(ControlWrapper* parent, const std::string& label,
	const Position& pos, const Size& size, long style,
	bool& checked, std::function<void(bool, void*)> onChange)
{
	createNativeObject(parent, label, pos, size, style, checked);
	reinterpret_cast<wxCheckBox*>(m_nativeWidget)->Bind(wxEVT_CHECKBOX, [&checked, cbk = std::move(onChange), nativeWgd = m_nativeWidget](wxCommandEvent& evt) {
		checked = evt.IsChecked();
		if (cbk) cbk(checked, nativeWgd);
	});
}

CheckBoxWrapper::CheckBoxWrapper(ControlWrapper* parent, const std::string& label,
	const Position& pos, const Size& size, long style,
	const bool& initialChecked, std::function<void(bool, void*)> onChange)
{
	createNativeObject(parent, label, pos, size, style, initialChecked);
	if (onChange)
		reinterpret_cast<wxCheckBox*>(m_nativeWidget)->Bind(wxEVT_CHECKBOX, [cbk = std::move(onChange), nativeWgd = m_nativeWidget](wxCommandEvent& evt) {
			cbk(evt.IsChecked(), nativeWgd);
		});
}

// ToggleButtonWrapper -----------------------------------------------------------

void ToggleButtonWrapper::createNativeObject(ControlWrapper* parent, const std::string& label,
	const bool& toggled, const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ToggleButtonWrapper::ToggleButtonWrapper()\t-> new wxToggleButton()\n");
#endif
	auto* btn = new wxToggleButton(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, label,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	btn->SetValue(toggled);
	m_nativeWidget = btn;
}

ToggleButtonWrapper::ToggleButtonWrapper(ControlWrapper* parent, const std::string& label,
	bool& toggled, const Position& pos, const Size& size, long style,
	std::function<void(bool)> onChange)
{
	createNativeObject(parent, label, toggled, pos, size, style);
	reinterpret_cast<wxToggleButton*>(m_nativeWidget)->Bind(wxEVT_TOGGLEBUTTON, [&toggled, cb = std::move(onChange)](wxCommandEvent& evt) {
		toggled = evt.IsChecked();
		if (cb) cb(toggled);
	});
}

ToggleButtonWrapper::ToggleButtonWrapper(ControlWrapper* parent, const std::string& label,
	const bool& initialToggled, const Position& pos, const Size& size, long style,
	std::function<void(bool)> onChange)
{
	createNativeObject(parent, label, initialToggled, pos, size, style);
	if (onChange)
		reinterpret_cast<wxToggleButton*>(m_nativeWidget)->Bind(wxEVT_TOGGLEBUTTON, [cb = std::move(onChange)](wxCommandEvent& evt) {
			cb(evt.IsChecked());
		});
}

ToggleButtonWrapper::ToggleButtonWrapper(ControlWrapper* parent, const std::string& label,
	bool& toggled, const Position& pos, const Size& size, long style,
	std::function<void(bool, void*)> onChange)
{
	createNativeObject(parent, label, toggled, pos, size, style);
	reinterpret_cast<wxToggleButton*>(m_nativeWidget)->Bind(wxEVT_TOGGLEBUTTON, [&toggled, cb = std::move(onChange), nativeWgd = m_nativeWidget](wxCommandEvent& evt) {
		toggled = evt.IsChecked();
		if (cb) cb(toggled, nativeWgd);
	});
}

ToggleButtonWrapper::ToggleButtonWrapper(ControlWrapper* parent, const std::string& label,
	const bool& initialToggled, const Position& pos, const Size& size, long style,
	std::function<void(bool, void*)> onChange)
{
	createNativeObject(parent, label, initialToggled, pos, size, style);
	if (onChange)
		reinterpret_cast<wxToggleButton*>(m_nativeWidget)->Bind(wxEVT_TOGGLEBUTTON, [cb = std::move(onChange), nativeWgd = m_nativeWidget](wxCommandEvent& evt) {
			cb(evt.IsChecked(), nativeWgd);
		});
}

// ImageWrapper -----------------------------------------------------------

void ImageWrapper::createNativeObject(ControlWrapper* parent, const std::string& filePath,
	const Position& pos, const Size& size, long style)
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
	m_nativeWidget = new wxStaticBitmap(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY, bmp,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
}

ImageWrapper::ImageWrapper(ControlWrapper* parent, const std::string& filePath,
	const Position& pos, const Size& size, long style,
	std::function<void()> onClick,
	std::function<void()> onHover)
{
	createNativeObject(parent, filePath, pos, size, style);
	auto* bmpCtrl = reinterpret_cast<wxStaticBitmap*>(m_nativeWidget);
	if (onClick)
		bmpCtrl->Bind(wxEVT_LEFT_DOWN, [cb = std::move(onClick)](wxMouseEvent&) { cb(); });
	if (onHover)
		bmpCtrl->Bind(wxEVT_ENTER_WINDOW, [cb = std::move(onHover)](wxMouseEvent&) { cb(); });
}

ImageWrapper::ImageWrapper(ControlWrapper* parent, const std::string& filePath,
	const Position& pos, const Size& size, long style,
	std::function<void(void*)> onClick,
	std::function<void(void*)> onHover)
{
	createNativeObject(parent, filePath, pos, size, style);
	auto* bmpCtrl = reinterpret_cast<wxStaticBitmap*>(m_nativeWidget);
	if (onClick)
		bmpCtrl->Bind(wxEVT_LEFT_DOWN, [cb = std::move(onClick), nativeWgd = m_nativeWidget](wxMouseEvent&) { cb(nativeWgd); });
	if (onHover)
		bmpCtrl->Bind(wxEVT_ENTER_WINDOW, [cb = std::move(onHover), nativeWgd = m_nativeWidget](wxMouseEvent&) { cb(nativeWgd); });
}

// ColorPickerWrapper -----------------------------------------------------------

void ColorPickerWrapper::createNativeObject(ControlWrapper* parent, const Color& value,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ColorPickerWrapper::ColorPickerWrapper()\t-> new wxColourPickerCtrl()\n");
#endif
	m_nativeWidget = new wxColourPickerCtrl(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY,
		wxColour(static_cast<unsigned char>(value.r * 255),
		         static_cast<unsigned char>(value.g * 255),
		         static_cast<unsigned char>(value.b * 255),
		         static_cast<unsigned char>(value.a * 255)),
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
}

ColorPickerWrapper::ColorPickerWrapper(ControlWrapper* parent, Color& value,
	const Position& pos, const Size& size, long style,
	std::function<void(const Color&)> onChange)
{
	createNativeObject(parent, value, pos, size, style);
	reinterpret_cast<wxColourPickerCtrl*>(m_nativeWidget)->Bind(wxEVT_COLOURPICKER_CHANGED, [&value, cb = std::move(onChange)](wxColourPickerEvent& evt) {
		const wxColour& c = evt.GetColour();
		value = Color{ c.Red() / 255.0f, c.Green() / 255.0f, c.Blue() / 255.0f, c.Alpha() / 255.0f };
		if (cb) cb(value);
	});
}

ColorPickerWrapper::ColorPickerWrapper(ControlWrapper* parent, const Color& value,
	const Position& pos, const Size& size, long style,
	std::function<void(const Color&)> onChange)
{
	createNativeObject(parent, value, pos, size, style);
	if (onChange)
		reinterpret_cast<wxColourPickerCtrl*>(m_nativeWidget)->Bind(wxEVT_COLOURPICKER_CHANGED, [cb = std::move(onChange)](wxColourPickerEvent& evt) {
			const wxColour& c = evt.GetColour();
			cb(Color{ c.Red() / 255.0f, c.Green() / 255.0f, c.Blue() / 255.0f, c.Alpha() / 255.0f });
		});
}

ColorPickerWrapper::ColorPickerWrapper(ControlWrapper* parent, Color& value,
	const Position& pos, const Size& size, long style,
	std::function<void(const Color&, void*)> onChange)
{
	createNativeObject(parent, value, pos, size, style);
	reinterpret_cast<wxColourPickerCtrl*>(m_nativeWidget)->Bind(wxEVT_COLOURPICKER_CHANGED, [&value, cb = std::move(onChange), nativeWgd = m_nativeWidget](wxColourPickerEvent& evt) {
		const wxColour& c = evt.GetColour();
		value = Color{ c.Red() / 255.0f, c.Green() / 255.0f, c.Blue() / 255.0f, c.Alpha() / 255.0f };
		if (cb) cb(value, nativeWgd);
	});
}

ColorPickerWrapper::ColorPickerWrapper(ControlWrapper* parent, const Color& value,
	const Position& pos, const Size& size, long style,
	std::function<void(const Color&, void*)> onChange)
{
	createNativeObject(parent, value, pos, size, style);
	if (onChange)
		reinterpret_cast<wxColourPickerCtrl*>(m_nativeWidget)->Bind(wxEVT_COLOURPICKER_CHANGED, [cb = std::move(onChange), nativeWgd = m_nativeWidget](wxColourPickerEvent& evt) {
			const wxColour& c = evt.GetColour();
			cb(Color{ c.Red() / 255.0f, c.Green() / 255.0f, c.Blue() / 255.0f, c.Alpha() / 255.0f }, nativeWgd);
		});
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
void ComboBoxWrapper<T>::createNativeObject(ControlWrapper* parent, const std::vector<std::string>& choices,
	const T& selected, const Position& pos, const Size& size, long style)
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
		combo->SetValue(selected);
	else
		combo->SetSelection(selected);
	m_nativeWidget = combo;
}

template <ComboBoxValue T>
ComboBoxWrapper<T>::ComboBoxWrapper(ControlWrapper* parent, std::vector<std::string> choices,
	T& selected, const Position& pos, const Size& size, long style,
	std::function<void(const T&)> onChange)
{
	createNativeObject(parent, choices, selected, pos, size, style);
	auto* combo = reinterpret_cast<wxComboBox*>(m_nativeWidget);
	if constexpr (std::is_same_v<T, std::string>)
		combo->Bind(wxEVT_COMBOBOX, [&selected, cb = std::move(onChange)](wxCommandEvent& evt) {
			selected = evt.GetString().ToStdString();
			if (cb) cb(selected);
		});
	else
		combo->Bind(wxEVT_COMBOBOX, [&selected, cb = std::move(onChange)](wxCommandEvent& evt) {
			selected = evt.GetSelection();
			if (cb) cb(selected);
		});
}

template <ComboBoxValue T>
ComboBoxWrapper<T>::ComboBoxWrapper(ControlWrapper* parent, std::vector<std::string> choices,
	const T& selected, const Position& pos, const Size& size, long style,
	std::function<void(const T&)> onChange)
{
	createNativeObject(parent, choices, selected, pos, size, style);
	if (!onChange)
		return;
	auto* combo = reinterpret_cast<wxComboBox*>(m_nativeWidget);
	if constexpr (std::is_same_v<T, std::string>)
		combo->Bind(wxEVT_COMBOBOX, [cb = std::move(onChange)](wxCommandEvent& evt) {
			cb(evt.GetString().ToStdString());
		});
	else
		combo->Bind(wxEVT_COMBOBOX, [cb = std::move(onChange)](wxCommandEvent& evt) {
			cb(static_cast<T>(evt.GetSelection()));
		});
}

template <ComboBoxValue T>
ComboBoxWrapper<T>::ComboBoxWrapper(ControlWrapper* parent, std::vector<std::string> choices,
	T& selected, const Position& pos, const Size& size, long style,
	std::function<void(const T&, void*)> onChange)
{
	createNativeObject(parent, choices, selected, pos, size, style);
	auto* combo = reinterpret_cast<wxComboBox*>(m_nativeWidget);
	if constexpr (std::is_same_v<T, std::string>)
		combo->Bind(wxEVT_COMBOBOX, [&selected, cb = std::move(onChange), nativeWgd = m_nativeWidget](wxCommandEvent& evt) {
			selected = evt.GetString().ToStdString();
			if (cb) cb(selected, nativeWgd);
		});
	else
		combo->Bind(wxEVT_COMBOBOX, [&selected, cb = std::move(onChange), nativeWgd = m_nativeWidget](wxCommandEvent& evt) {
			selected = evt.GetSelection();
			if (cb) cb(selected, nativeWgd);
		});
}

template <ComboBoxValue T>
ComboBoxWrapper<T>::ComboBoxWrapper(ControlWrapper* parent, std::vector<std::string> choices,
	const T& selected, const Position& pos, const Size& size, long style,
	std::function<void(const T&, void*)> onChange)
{
	createNativeObject(parent, choices, selected, pos, size, style);
	if (!onChange)
		return;
	auto* combo = reinterpret_cast<wxComboBox*>(m_nativeWidget);
	if constexpr (std::is_same_v<T, std::string>)
		combo->Bind(wxEVT_COMBOBOX, [cb = std::move(onChange), nativeWgd = m_nativeWidget](wxCommandEvent& evt) {
			cb(evt.GetString().ToStdString(), nativeWgd);
		});
	else
		combo->Bind(wxEVT_COMBOBOX, [cb = std::move(onChange), nativeWgd = m_nativeWidget](wxCommandEvent& evt) {
			cb(static_cast<T>(evt.GetSelection()), nativeWgd);
		});
}

template class ComboBoxWrapper<std::string>;
template class ComboBoxWrapper<int>;
