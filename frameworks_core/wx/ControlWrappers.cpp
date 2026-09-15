#include "frameworks_core/ControlWrappers.hpp"
#include "frameworks_core/wx/RefSync.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>
#include <tuple>
#include <utility>

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
#include <wx/treectrl.h>
#include <wx/toolbar.h>
#include <wx/dataview.h>
#include <wx/settings.h>
#include <wx/collheaderctrl.h>

// Constructors only collect data and live inline in ControlWrappers.hpp.
// realize() creates the native wxWidget from the collected data (plus the
// position/size/style stored in the ControlWrapper base) under the given
// parent window and binds its events; the layout engine measures and places.

// ButtonWrapper -----------------------------------------------------------

void ButtonWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("ButtonWrapper::realize()\t-> new wxButton()\n");
#endif
	auto* btn = new wxButton(static_cast<wxWindow*>(parentWindow), wxID_ANY, m_label,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	m_nativeWidget = btn;

	if (m_onClick)
		btn->Bind(wxEVT_BUTTON, [cb = std::move(m_onClick)](wxCommandEvent&) { cb(); });
	else if (m_onClickWithWidget)
		btn->Bind(wxEVT_BUTTON, [cb = std::move(m_onClickWithWidget), nw = m_nativeWidget](wxCommandEvent&) { cb(nw); });

}

// TextCtrlWrapper -----------------------------------------------------------

void TextCtrlWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("TextCtrlWrapper::realize()\t-> new wxTextCtrl()\n");
#endif
	const std::string& initial = m_value.get();
	auto* txt = new wxTextCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY, initial,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	m_nativeWidget = txt;

	if (m_value.isBound())
	{
		auto& value = m_value.get();
		txt->Bind(wxEVT_TEXT, [&value, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent& evt) {
			value = evt.GetString().ToStdString();
			if (cb) cb(value);
			else if (cbw) cbw(value, nw);
		});
		bindExternalRefSync(txt,
			[txt] { return txt->GetValue().ToStdString(); },
			[&value] { return value; },
			[txt](const std::string& v) { txt->ChangeValue(v); });
	}
	else if (m_onChange)
		txt->Bind(wxEVT_TEXT, [cb = std::move(m_onChange)](wxCommandEvent& evt) { cb(evt.GetString().ToStdString()); });
	else if (m_onChangeWithWidget)
		txt->Bind(wxEVT_TEXT, [cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent& evt) { cbw(evt.GetString().ToStdString(), nw); });

}

// PasswordInputWrapper -----------------------------------------------------------

void PasswordInputWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("PasswordInputWrapper::realize()\t-> new wxTextCtrl(wxTE_PASSWORD)\n");
#endif
	const std::string& initial = m_value.get();
	auto* txt = new wxTextCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY, initial,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style | wxTE_PASSWORD);
	m_nativeWidget = txt;

	if (m_value.isBound())
	{
		auto& value = m_value.get();
		txt->Bind(wxEVT_TEXT, [&value, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent& evt) {
			value = evt.GetString().ToStdString();
			if (cb) cb(value);
			else if (cbw) cbw(value, nw);
		});
		bindExternalRefSync(txt,
			[txt] { return txt->GetValue().ToStdString(); },
			[&value] { return value; },
			[txt](const std::string& v) { txt->ChangeValue(v); });
	}
	else if (m_onChange)
		txt->Bind(wxEVT_TEXT, [cb = std::move(m_onChange)](wxCommandEvent& evt) { cb(evt.GetString().ToStdString()); });
	else if (m_onChangeWithWidget)
		txt->Bind(wxEVT_TEXT, [cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent& evt) { cbw(evt.GetString().ToStdString(), nw); });

}

// MultiLineTextCtrlWrapper -----------------------------------------------------------

void MultiLineTextCtrlWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("MultiLineTextCtrlWrapper::realize()\t-> new wxTextCtrl(wxTE_MULTILINE)\n");
#endif
	const std::string& initial = m_value.get();
	auto* txt = new wxTextCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY, initial,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style | wxTE_MULTILINE);
	m_nativeWidget = txt;

	if (m_value.isBound())
	{
		auto& value = m_value.get();
		txt->Bind(wxEVT_TEXT, [&value, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent& evt) {
			value = evt.GetString().ToStdString();
			if (cb) cb(value);
			else if (cbw) cbw(value, nw);
		});
		bindExternalRefSync(txt,
			[txt] { return txt->GetValue().ToStdString(); },
			[&value] { return value; },
			[txt](const std::string& v) { txt->ChangeValue(v); });
	}
	else if (m_onChange)
		txt->Bind(wxEVT_TEXT, [cb = std::move(m_onChange)](wxCommandEvent& evt) { cb(evt.GetString().ToStdString()); });
	else if (m_onChangeWithWidget)
		txt->Bind(wxEVT_TEXT, [cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent& evt) { cbw(evt.GetString().ToStdString(), nw); });

}

// ReadonlyTextCtrlWrapper -----------------------------------------------------------

void ReadonlyTextCtrlWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("ReadonlyTextCtrlWrapper::realize()\t-> new wxTextCtrl(wxTE_READONLY)\n");
#endif
	m_nativeWidget = new wxTextCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY, m_value,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style | wxTE_READONLY);

}

// ClickableTextWrapper -----------------------------------------------------------

void ClickableTextWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("ClickableTextWrapper::realize()\t-> new wxStaticText()\n");
#endif
	auto* st = new wxStaticText(static_cast<wxWindow*>(parentWindow), wxID_ANY, m_text,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	m_nativeWidget = st;

	if (m_onClick)
		st->Bind(wxEVT_LEFT_DOWN, [cb = std::move(m_onClick)](wxMouseEvent&) { cb(); });
	else if (m_onClickWithWidget)
		st->Bind(wxEVT_LEFT_DOWN, [cb = std::move(m_onClickWithWidget), nw = m_nativeWidget](wxMouseEvent&) { cb(nw); });

}

// LinkTextWrapper -----------------------------------------------------------

void LinkTextWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("LinkTextWrapper::realize()\t-> new wxHyperlinkCtrl()\n");
#endif
	auto* link = new wxHyperlinkCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY, m_text, wxEmptyString,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	m_nativeWidget = link;

	if (m_onClick)
		link->Bind(wxEVT_HYPERLINK, [cb = std::move(m_onClick)](wxHyperlinkEvent&) { cb(); });
	else if (m_onClickWithWidget)
		link->Bind(wxEVT_HYPERLINK, [cb = std::move(m_onClickWithWidget), nw = m_nativeWidget](wxHyperlinkEvent&) { cb(nw); });

}

// StaticTextWrapper -----------------------------------------------------------

void StaticTextWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("StaticTextWrapper::realize()\t-> new wxStaticText()\n");
#endif
	m_nativeWidget = new wxStaticText(static_cast<wxWindow*>(parentWindow), wxID_ANY, m_text,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);

}

// DatePickerWrapper -----------------------------------------------------------

void DatePickerWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("DatePickerWrapper::realize()\t-> new wxDatePickerCtrl()\n");
#endif
	const Date& dval = m_value.get();
	wxDateTime dt;
	dt.Set(static_cast<wxDateTime::wxDateTime_t>(dval.day),
		static_cast<wxDateTime::Month>(dval.month - 1),
		dval.year);
	auto* dp = new wxDatePickerCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY, dt,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	m_nativeWidget = dp;

	if (m_value.isBound())
	{
		auto& value = m_value.get();
		dp->Bind(wxEVT_DATE_CHANGED, [&value, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxDateEvent& evt) {
			const wxDateTime& d = evt.GetDate();
			value.year  = d.GetYear();
			value.month = static_cast<int>(d.GetMonth()) + 1;
			value.day   = d.GetDay();
			if (cb) cb(value);
			else if (cbw) cbw(value, nw);
		});
		// Compare the Y/M/D triple, not the wxDateTime: the picker keeps a time-of-day
		// component that Date has no opinion about.
		bindExternalRefSync(dp,
			[dp] {
				const wxDateTime d = dp->GetValue();
				return std::tuple{ d.GetYear(), static_cast<int>(d.GetMonth()) + 1, static_cast<int>(d.GetDay()) };
			},
			[&value] { return std::tuple{ value.year, value.month, value.day }; },
			[dp](const std::tuple<int, int, int>& ymd) {
				wxDateTime d;
				d.Set(static_cast<wxDateTime::wxDateTime_t>(std::get<2>(ymd)),
					static_cast<wxDateTime::Month>(std::get<1>(ymd) - 1),
					std::get<0>(ymd));
				dp->SetValue(d);
			});
	}
	else if (m_onChange)
		dp->Bind(wxEVT_DATE_CHANGED, [cb = std::move(m_onChange)](wxDateEvent& evt) {
			const wxDateTime& d = evt.GetDate();
			Date date{ d.GetYear(), static_cast<int>(d.GetMonth()) + 1, d.GetDay() };
			cb(date);
		});
	else if (m_onChangeWithWidget)
		dp->Bind(wxEVT_DATE_CHANGED, [cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxDateEvent& evt) {
			const wxDateTime& d = evt.GetDate();
			Date date{ d.GetYear(), static_cast<int>(d.GetMonth()) + 1, d.GetDay() };
			cbw(date, nw);
		});

}

// TimePickerWrapper -----------------------------------------------------------

void TimePickerWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("TimePickerWrapper::realize()\t-> new wxTimePickerCtrl()\n");
#endif
	const Time& tval = m_value.get();
	wxDateTime dt = wxDateTime::Now();
	dt.SetHour(tval.hour);
	dt.SetMinute(tval.minute);
	dt.SetSecond(tval.second);
	auto* tp = new wxTimePickerCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY, dt,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	m_nativeWidget = tp;

	if (m_value.isBound())
	{
		auto& value = m_value.get();
		tp->Bind(wxEVT_TIME_CHANGED, [&value, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxDateEvent& evt) {
			const wxDateTime& d = evt.GetDate();
			value.hour   = d.GetHour();
			value.minute = d.GetMinute();
			value.second = d.GetSecond();
			if (cb) cb(value);
			else if (cbw) cbw(value, nw);
		});
		bindExternalRefSync(tp,
			[tp] {
				int h = 0, m = 0, s = 0;
				tp->GetTime(&h, &m, &s);
				return std::tuple{ h, m, s };
			},
			[&value] { return std::tuple{ value.hour, value.minute, value.second }; },
			[tp](const std::tuple<int, int, int>& hms) { tp->SetTime(std::get<0>(hms), std::get<1>(hms), std::get<2>(hms)); });
	}
	else if (m_onChange)
		tp->Bind(wxEVT_TIME_CHANGED, [cb = std::move(m_onChange)](wxDateEvent& evt) {
			const wxDateTime& d = evt.GetDate();
			Time time{ d.GetHour(), d.GetMinute(), d.GetSecond() };
			cb(time);
		});
	else if (m_onChangeWithWidget)
		tp->Bind(wxEVT_TIME_CHANGED, [cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxDateEvent& evt) {
			const wxDateTime& d = evt.GetDate();
			Time time{ d.GetHour(), d.GetMinute(), d.GetSecond() };
			cbw(time, nw);
		});

}

// SliderWrapper -----------------------------------------------------------

template <SliderValue T>
void SliderWrapper<T>::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("SliderWrapper::realize()\t-> new wxSlider()\n");
#endif
	const T& val = m_value.get();
	wxSlider* sl = nullptr;
	if constexpr (std::is_floating_point_v<T>)
	{
		int iMin = static_cast<int>(m_range.min / m_range.step);
		int iMax = static_cast<int>(m_range.max / m_range.step);
		int iVal = static_cast<int>(val / m_range.step);
		sl = new wxSlider(static_cast<wxWindow*>(parentWindow), wxID_ANY, iVal, iMin, iMax,
			wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	}
	else
	{
		sl = new wxSlider(static_cast<wxWindow*>(parentWindow), wxID_ANY, static_cast<int>(val), m_range.min, m_range.max,
			wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	}
	m_nativeWidget = sl;

	if (m_value.isBound())
	{
		auto& value = m_value.get();
		if constexpr (std::is_floating_point_v<T>)
			sl->Bind(wxEVT_SLIDER, [&value, step = m_range.step, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent& evt) {
				value = static_cast<T>(evt.GetInt()) * step;
				if (cb) cb(value);
				else if (cbw) cbw(value, nw);
			});
		else
			sl->Bind(wxEVT_SLIDER, [&value, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent& evt) {
				value = evt.GetInt();
				if (cb) cb(value);
				else if (cbw) cbw(value, nw);
			});
		// wxSlider is integral; a float slider lives in step units, so compare there.
		bindExternalRefSync(sl,
			[sl] { return sl->GetValue(); },
			[&value, step = m_range.step] {
				if constexpr (std::is_floating_point_v<T>)
					return static_cast<int>(value / step);
				else
					return static_cast<int>(value);
			},
			[sl](int v) { sl->SetValue(v); });
	}
	else if (m_onChange)
	{
		if constexpr (std::is_floating_point_v<T>)
			sl->Bind(wxEVT_SLIDER, [step = m_range.step, cb = std::move(m_onChange)](wxCommandEvent& evt) { cb(static_cast<T>(evt.GetInt()) * step); });
		else
			sl->Bind(wxEVT_SLIDER, [cb = std::move(m_onChange)](wxCommandEvent& evt) { cb(static_cast<T>(evt.GetInt())); });
	}
	else if (m_onChangeWithWidget)
	{
		if constexpr (std::is_floating_point_v<T>)
			sl->Bind(wxEVT_SLIDER, [step = m_range.step, cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent& evt) { cbw(static_cast<T>(evt.GetInt()) * step, nw); });
		else
			sl->Bind(wxEVT_SLIDER, [cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent& evt) { cbw(static_cast<T>(evt.GetInt()), nw); });
	}

}

template class SliderWrapper<int>;
template class SliderWrapper<float>;

// SpinBoxWrapper -----------------------------------------------------------

template <SpinBoxValue T>
void SpinBoxWrapper<T>::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("SpinBoxWrapper::realize()\t-> new wxSpinCtrl[Double]()\n");
#endif
	const T& val = m_value.get();
	if constexpr (std::is_same_v<T, int>)
	{
		auto* spin = new wxSpinCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY, wxEmptyString,
			wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style,
			m_range.min, m_range.max, static_cast<int>(val));
		m_nativeWidget = spin;

		if (m_value.isBound())
		{
			auto& value = m_value.get();
			spin->Bind(wxEVT_SPINCTRL, [&value, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxSpinEvent& evt) {
				value = evt.GetInt();
				if (cb) cb(value);
				else if (cbw) cbw(value, nw);
			});
			bindExternalRefSync(spin,
				[spin] { return spin->GetValue(); },
				[&value] { return static_cast<int>(value); },
				[spin](int v) { spin->SetValue(v); });
		}
		else if (m_onChange)
			spin->Bind(wxEVT_SPINCTRL, [cb = std::move(m_onChange)](wxSpinEvent& evt) { cb(evt.GetInt()); });
		else if (m_onChangeWithWidget)
			spin->Bind(wxEVT_SPINCTRL, [cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxSpinEvent& evt) { cbw(evt.GetInt(), nw); });
	}
	else
	{
		auto* spin = new wxSpinCtrlDouble(static_cast<wxWindow*>(parentWindow), wxID_ANY, wxEmptyString,
			wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style,
			m_range.min, m_range.max, static_cast<double>(val), m_range.step);
		m_nativeWidget = spin;

		if (m_value.isBound())
		{
			auto& value = m_value.get();
			spin->Bind(wxEVT_SPINCTRLDOUBLE, [&value, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxSpinDoubleEvent& evt) {
				value = static_cast<T>(evt.GetValue());
				if (cb) cb(value);
				else if (cbw) cbw(value, nw);
			});
			// Quantise both sides to step units: wxSpinCtrlDouble rounds what it stores to
			// its display precision, so a raw double compare would push-and-round forever.
			bindExternalRefSync(spin,
				[spin, step = m_range.step] { return std::lround(spin->GetValue() / step); },
				[&value, step = m_range.step] { return std::lround(value / step); },
				[spin, step = m_range.step](long units) { spin->SetValue(static_cast<double>(units) * step); });
		}
		else if (m_onChange)
			spin->Bind(wxEVT_SPINCTRLDOUBLE, [cb = std::move(m_onChange)](wxSpinDoubleEvent& evt) { cb(static_cast<T>(evt.GetValue())); });
		else if (m_onChangeWithWidget)
			spin->Bind(wxEVT_SPINCTRLDOUBLE, [cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxSpinDoubleEvent& evt) { cbw(static_cast<T>(evt.GetValue()), nw); });
	}

}

template class SpinBoxWrapper<int>;
template class SpinBoxWrapper<float>;

// RadioButtonWrapper -----------------------------------------------------------

template <RadioButtonValue T>
void RadioButtonWrapper<T>::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("RadioButtonWrapper::realize()\t-> new wxRadioButton()\n");
#endif
	const T& val = m_value.get();
	wxRadioButton* rb = nullptr;
	if constexpr (std::is_same_v<T, bool>)
	{
		rb = new wxRadioButton(static_cast<wxWindow*>(parentWindow), wxID_ANY, m_label,
			wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
		rb->SetValue(val);
	}
	else
	{
		// m_index (assigned at construction) is the radio's position within its
		// group; index 0 starts a new wx radio group.
		long groupStyle = (m_index == 0) ? wxRB_GROUP : 0;
		rb = new wxRadioButton(static_cast<wxWindow*>(parentWindow), wxID_ANY, m_label,
			wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style | groupStyle);
		rb->SetValue(static_cast<int>(val) == m_index);
	}
	m_nativeWidget = rb;

	if (m_value.isBound())
	{
		auto& value = m_value.get();
		if constexpr (std::is_same_v<T, bool>)
			rb->Bind(wxEVT_RADIOBUTTON, [&value, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent&) {
				value = true;
				if (cb) cb(value);
				else if (cbw) cbw(value, nw);
			});
		else
			rb->Bind(wxEVT_RADIOBUTTON, [&value, index = m_index, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent&) {
				value = index;
				if (cb) cb(value);
				else if (cbw) cbw(value, nw);
			});
		// Every radio in the group syncs itself; wx clears the siblings when one is set.
		bindExternalRefSync(rb,
			[rb] { return rb->GetValue(); },
			[&value, index = m_index] {
				if constexpr (std::is_same_v<T, bool>)
					return static_cast<bool>(value);
				else
					return static_cast<int>(value) == index;
			},
			[rb](bool on) { rb->SetValue(on); });
	}
	else if (m_onChange)
	{
		if constexpr (std::is_same_v<T, bool>)
			rb->Bind(wxEVT_RADIOBUTTON, [cb = std::move(m_onChange)](wxCommandEvent&) { cb(true); });
		else
			rb->Bind(wxEVT_RADIOBUTTON, [index = m_index, cb = std::move(m_onChange)](wxCommandEvent&) { cb(static_cast<T>(index)); });
	}
	else if (m_onChangeWithWidget)
	{
		if constexpr (std::is_same_v<T, bool>)
			rb->Bind(wxEVT_RADIOBUTTON, [cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent&) { cbw(true, nw); });
		else
			rb->Bind(wxEVT_RADIOBUTTON, [index = m_index, cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent&) { cbw(static_cast<T>(index), nw); });
	}

}

template class RadioButtonWrapper<bool>;
template class RadioButtonWrapper<int>;

// CheckBoxWrapper -----------------------------------------------------------

void CheckBoxWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("CheckBoxWrapper::realize()\t-> new wxCheckBox()\n");
#endif
	const bool checked = m_value.get();
	auto* chk = new wxCheckBox(static_cast<wxWindow*>(parentWindow), wxID_ANY, m_label,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	chk->SetValue(checked);
	m_nativeWidget = chk;

	if (m_value.isBound())
	{
		auto& value = m_value.get();
		chk->Bind(wxEVT_CHECKBOX, [&value, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent& evt) {
			value = evt.IsChecked();
			if (cb) cb(value);
			else if (cbw) cbw(value, nw);
		});
		bindExternalRefSync(chk,
			[chk] { return chk->GetValue(); },
			[&value] { return value; },
			[chk](bool on) { chk->SetValue(on); });
	}
	else if (m_onChange)
		chk->Bind(wxEVT_CHECKBOX, [cb = std::move(m_onChange)](wxCommandEvent& evt) { cb(evt.IsChecked()); });
	else if (m_onChangeWithWidget)
		chk->Bind(wxEVT_CHECKBOX, [cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent& evt) { cbw(evt.IsChecked(), nw); });

}

// ToggleButtonWrapper -----------------------------------------------------------

void ToggleButtonWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("ToggleButtonWrapper::realize()\t-> new wxToggleButton()\n");
#endif
	const bool toggled = m_value.get();
	auto* btn = new wxToggleButton(static_cast<wxWindow*>(parentWindow), wxID_ANY, m_label,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	btn->SetValue(toggled);
	m_nativeWidget = btn;

	if (m_value.isBound())
	{
		auto& value = m_value.get();
		btn->Bind(wxEVT_TOGGLEBUTTON, [&value, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent& evt) {
			value = evt.IsChecked();
			if (cb) cb(value);
			else if (cbw) cbw(value, nw);
		});
		bindExternalRefSync(btn,
			[btn] { return btn->GetValue(); },
			[&value] { return value; },
			[btn](bool on) { btn->SetValue(on); });
	}
	else if (m_onChange)
		btn->Bind(wxEVT_TOGGLEBUTTON, [cb = std::move(m_onChange)](wxCommandEvent& evt) { cb(evt.IsChecked()); });
	else if (m_onChangeWithWidget)
		btn->Bind(wxEVT_TOGGLEBUTTON, [cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent& evt) { cbw(evt.IsChecked(), nw); });

}

// ImageWrapper -----------------------------------------------------------

void ImageWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("ImageWrapper::realize()\t-> new wxStaticBitmap()\n");
#endif
	static bool s_handlersInit = false;
	if (!s_handlersInit)
	{
		wxInitAllImageHandlers();
		s_handlersInit = true;
	}
	wxImage wxImg(m_filePath, wxBITMAP_TYPE_ANY);
	if (wxImg.IsOk() && m_size.width > 0 && m_size.height > 0)
		wxImg = wxImg.Scale(m_size.width, m_size.height, wxIMAGE_QUALITY_HIGH);
	wxBitmap bmp(wxImg.IsOk() ? wxImg : wxImage(16, 16));
	auto* bmpCtrl = new wxStaticBitmap(static_cast<wxWindow*>(parentWindow), wxID_ANY, bmp,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	m_nativeWidget = bmpCtrl;

	if (m_onClick)
		bmpCtrl->Bind(wxEVT_LEFT_DOWN, [cb = std::move(m_onClick)](wxMouseEvent&) { cb(); });
	else if (m_onClickWithWidget)
		bmpCtrl->Bind(wxEVT_LEFT_DOWN, [cb = std::move(m_onClickWithWidget), nw = m_nativeWidget](wxMouseEvent&) { cb(nw); });

	if (m_onHover)
		bmpCtrl->Bind(wxEVT_ENTER_WINDOW, [cb = std::move(m_onHover)](wxMouseEvent&) { cb(); });
	else if (m_onHoverWithWidget)
		bmpCtrl->Bind(wxEVT_ENTER_WINDOW, [cb = std::move(m_onHoverWithWidget), nw = m_nativeWidget](wxMouseEvent&) { cb(nw); });

}

// ToolBarWrapper -----------------------------------------------------------

void ToolBarWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("ToolBarWrapper::realize()\t-> new wxToolBar()\n");
#endif
	static bool s_handlersInit = false;
	if (!s_handlersInit)
	{
		wxInitAllImageHandlers();
		s_handlersInit = true;
	}

	// A CHILD wxToolBar, deliberately not wxFrame::CreateToolBar(): that one
	// docks itself to a frame and would be invisible to the engine, and it
	// would make a toolbar impossible inside a Dialog or anywhere down a stack.
	long style = wxTB_HORIZONTAL | wxTB_FLAT | wxTB_NODIVIDER;
	const bool anyLabels = m_labelsForced
		|| std::any_of(m_tools.begin(), m_tools.end(), [](const ToolItem& tool) {
			return !tool.isSeparator && tool.iconPath.empty();
		});
	if (anyLabels)
		style |= wxTB_TEXT;

	auto* bar = new wxToolBar(static_cast<wxWindow*>(parentWindow), wxID_ANY,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), style | m_style);
	bar->SetToolBitmapSize(wxSize(m_iconSize.width, m_iconSize.height));
	m_nativeWidget = bar;

	for (ToolItem& tool : m_tools)
	{
		if (tool.isSeparator)
		{
			bar->AddSeparator();
			continue;
		}

		// A tool with no usable icon shows its label instead, so the row is
		// never blank -- and a bitmap is still required by AddTool, so an empty
		// one of the right size stands in.
		wxBitmap bitmap;
		if (!tool.iconPath.empty())
		{
			wxImage image(tool.iconPath, wxBITMAP_TYPE_ANY);
			if (image.IsOk())
			{
				if (m_iconSize.width > 0 && m_iconSize.height > 0)
					image = image.Scale(m_iconSize.width, m_iconSize.height, wxIMAGE_QUALITY_HIGH);
				bitmap = wxBitmap(image);
			}
#ifdef USE_LOGGER
			else
			{
				Logger::instance().log("ToolBarWrapper::realize()\t-> icon \""
					+ tool.iconPath + "\" failed to load; falling back to the label\n");
			}
#endif
		}
		if (!bitmap.IsOk())
		{
			bitmap = wxBitmap(std::max(1, m_iconSize.width), std::max(1, m_iconSize.height));
			// Fully transparent, so only the label reads.
			wxImage blank = bitmap.ConvertToImage();
			blank.InitAlpha();
			std::memset(blank.GetAlpha(), 0,
				(std::size_t)blank.GetWidth() * (std::size_t)blank.GetHeight());
			bitmap = wxBitmap(blank);
		}

		const int id = wxWindow::NewControlId();
		const wxString label = wxString::FromUTF8(tool.label);
		const wxString help = wxString::FromUTF8(tool.tooltip);
		ToolItem* model = &tool;
		if (model->toggledFlag)
		{
			bar->AddCheckTool(id, label, bitmap, wxNullBitmap, help);
			bar->ToggleTool(id, model->toggledFlag->get());
		}
		else
		{
			bar->AddTool(id, label, bitmap, help);
		}
		bar->EnableTool(id, !model->disabledFlag.get());

		bar->Bind(wxEVT_TOOL, [model, bar, id](wxCommandEvent& event) {
			// Value first, then the callback, so a handler reading the bound
			// bool sees the state the user just produced.
			if (model->toggledFlag)
				model->toggledFlag->set(bar->GetToolState(id));
			if (model->clickHandler)
				model->clickHandler();
			event.Skip(false);
		}, id);

		// Bound flags are polled, never pushed on disagreement -- the shape
		// every externally-written value in the framework uses.
		if (model->toggledFlag && model->toggledFlag->isBound())
		{
			const bool* flag = model->toggledFlag->boundValue();
			bindExternalRefSync(bar,
				[bar, id] { return bar->GetToolState(id); },
				[flag] { return *flag; },
				[bar, id](bool value) { bar->ToggleTool(id, value); });
		}
		if (model->disabledFlag.isBound())
		{
			const bool* flag = model->disabledFlag.boundValue();
			bindExternalRefSync(bar,
				[bar, id] { return bar->GetToolEnabled(id); },
				[flag] { return !*flag; },
				[bar, id](bool enabled) { bar->EnableTool(id, enabled); });
		}
	}

	// Lays the tools out and fixes the bar's best size -- nothing appears
	// without it.
	bar->Realize();
}

// ColorPickerWrapper -----------------------------------------------------------

void ColorPickerWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("ColorPickerWrapper::realize()\t-> new wxColourPickerCtrl()\n");
#endif
	const Color& cval = m_value.get();
	auto* picker = new wxColourPickerCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY,
		wxColour(static_cast<unsigned char>(cval.r * 255),
		         static_cast<unsigned char>(cval.g * 255),
		         static_cast<unsigned char>(cval.b * 255),
		         static_cast<unsigned char>(cval.a * 255)),
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	m_nativeWidget = picker;

	if (m_value.isBound())
	{
		auto& value = m_value.get();
		picker->Bind(wxEVT_COLOURPICKER_CHANGED, [&value, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxColourPickerEvent& evt) {
			const wxColour& c = evt.GetColour();
			value = Color{ c.Red() / 255.0f, c.Green() / 255.0f, c.Blue() / 255.0f, c.Alpha() / 255.0f };
			if (cb) cb(value);
			else if (cbw) cbw(value, nw);
		});
		// Compare as wxColour: Color is float 0..1 but the control quantises to 0..255,
		// so the byte domain is the only one where round-tripping is stable.
		bindExternalRefSync(picker,
			[picker] { return picker->GetColour(); },
			[&value] {
				return wxColour(static_cast<unsigned char>(value.r * 255),
					static_cast<unsigned char>(value.g * 255),
					static_cast<unsigned char>(value.b * 255),
					static_cast<unsigned char>(value.a * 255));
			},
			[picker](const wxColour& c) { picker->SetColour(c); });
	}
	else if (m_onChange)
		picker->Bind(wxEVT_COLOURPICKER_CHANGED, [cb = std::move(m_onChange)](wxColourPickerEvent& evt) {
			const wxColour& c = evt.GetColour();
			cb(Color{ c.Red() / 255.0f, c.Green() / 255.0f, c.Blue() / 255.0f, c.Alpha() / 255.0f });
		});
	else if (m_onChangeWithWidget)
		picker->Bind(wxEVT_COLOURPICKER_CHANGED, [cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxColourPickerEvent& evt) {
			const wxColour& c = evt.GetColour();
			cbw(Color{ c.Red() / 255.0f, c.Green() / 255.0f, c.Blue() / 255.0f, c.Alpha() / 255.0f }, nw);
		});

}

// SeparatorWrapper -----------------------------------------------------------

void SeparatorWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("SeparatorWrapper::realize()\t-> new wxStaticLine()\n");
#endif
	const long orientStyle = m_orient == Orientation::Vertical ? wxLI_VERTICAL : wxLI_HORIZONTAL;
	m_nativeWidget = new wxStaticLine(static_cast<wxWindow*>(parentWindow), wxID_ANY,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style | orientStyle);

}

// SplitterSashWrapper -----------------------------------------------------------

void SplitterSashWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("SplitterSashWrapper::realize()\t-> new wxPanel()\n");
#endif
	const bool horizontal = m_state->orientation == Orientation::Horizontal;

	auto* sash = new wxPanel(static_cast<wxWindow*>(parentWindow), wxID_ANY,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	m_nativeWidget = sash;
	sash->SetCursor(wxCursor(horizontal ? wxCURSOR_SIZEWE : wxCURSOR_SIZENS));
	sash->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW));
	// An empty wxPanel's best size is arbitrary; the sash is exactly this thick
	// on its own axis and asks for nothing on the other, where the engine
	// stretches it across both panes.
	sash->SetMinSize(wxSize(horizontal ? SplitterState::kSashThickness : 0,
		horizontal ? 0 : SplitterState::kSashThickness));

	// Per-drag state, shared by the handlers below through a shared_ptr rather
	// than through the wrapper: every handler dies with the panel, and nothing
	// here may capture the wrapper (wx/RefSync.hpp -- wrapper and window
	// teardown order is not fixed). The SplitterState may be: it lives in the
	// node tree, which the engine session owns.
	struct SashDrag
	{
		int anchorScreen = 0;
		int anchorPos = 0;
		bool active = false;
	};
	auto drag = std::make_shared<SashDrag>();
	SplitterState* state = m_state;

	// The drag anchors on the position arrange RESOLVED when the gesture began,
	// so a long drag cannot accumulate rounding the way a per-event delta would,
	// and screen coordinates are used because the panel itself moves underneath
	// the pointer as the layout follows it.
	sash->Bind(wxEVT_LEFT_DOWN, [sash, state, drag, horizontal](wxMouseEvent& event) {
		const wxPoint screen = sash->ClientToScreen(event.GetPosition());
		drag->anchorScreen = horizontal ? screen.x : screen.y;
		drag->anchorPos = state->resolved;
		drag->active = true;
		if (!sash->HasCapture())
			sash->CaptureMouse();
		event.Skip();
	});
	sash->Bind(wxEVT_MOTION, [sash, state, drag, horizontal](wxMouseEvent& event) {
		if (drag->active && event.Dragging())
		{
			const wxPoint screen = sash->ClientToScreen(event.GetPosition());
			const int moved = (horizontal ? screen.x : screen.y) - drag->anchorScreen;
			state->position.set(std::clamp(drag->anchorPos + moved,
				state->lowerBound, state->upperBound));
		}
		event.Skip();
	});
	sash->Bind(wxEVT_LEFT_UP, [sash, drag](wxMouseEvent& event) {
		drag->active = false;
		if (sash->HasCapture())
			sash->ReleaseMouse();
		event.Skip();
	});
	// The capture is already gone here; releasing it again would assert.
	sash->Bind(wxEVT_MOUSE_CAPTURE_LOST, [drag](wxMouseCaptureLostEvent&) {
		drag->active = false;
	});

}

// ExpanderHeaderWrapper -----------------------------------------------------------

void ExpanderHeaderWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("ExpanderHeaderWrapper::realize()\t-> new wxCollapsibleHeaderCtrl()\n");
#endif
	// wxCollapsibleHeaderCtrl is the header half of wxCollapsiblePane without
	// the pane -- which is precisely what is wanted here, since the content is
	// an engine-arranged subtree rather than something wx may size for us.
	auto* header = new wxCollapsibleHeaderCtrl(static_cast<wxWindow*>(parentWindow),
		wxID_ANY, m_label, wxPoint(m_pos.x, m_pos.y),
		wxSize(m_size.width, m_size.height), m_style);
	header->SetCollapsed(!m_state->expanded.get());
	m_nativeWidget = header;

	// The ExpanderState lives in the node tree, which the engine session owns
	// and which outlives every window here; the wrapper must never be captured
	// (wx/RefSync.hpp -- wrapper and window teardown order is not fixed).
	ExpanderState* state = m_state;
	header->Bind(wxEVT_COLLAPSIBLEHEADER_CHANGED, [header, state](wxCommandEvent& event) {
		state->expanded.set(!header->IsCollapsed());
		event.Skip();
	});

	// A bound flag can be written from anywhere, and the header control applies
	// its own state only when clicked -- so the arrow is mirrored like any other
	// external ref. The relayout that follows is armed separately, by the
	// session's bindInvalidation: this only keeps the header itself honest.
	if (m_state->expanded.isBound())
	{
		bindExternalRefSync(header,
			[header] { return !header->IsCollapsed(); },
			[state] { return state->expanded.get(); },
			[header](bool open) { header->SetCollapsed(!open); });
	}

}

// ProgressBarWrapper -----------------------------------------------------------

void ProgressBarWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("ProgressBarWrapper::realize()\t-> new wxGauge()\n");
#endif
	// The bound float is a 0..100 percentage, matching the gauge's own integer range.
	const auto toGauge = [](float v) { return static_cast<int>(std::clamp(v, 0.0f, 100.0f)); };

	const float initial = m_value.get();
	auto* gauge = new wxGauge(static_cast<wxWindow*>(parentWindow), wxID_ANY, 100,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style | wxGA_HORIZONTAL | wxGA_SMOOTH);
	gauge->SetValue(toGauge(initial));
	m_nativeWidget = gauge;

	// A progress bar has no input events of its own -- the bound float is only ever
	// written from outside -- so the idle sync is the whole story here.
	if (m_value.isBound())
	{
		bindExternalRefSync(gauge,
			[gauge] { return gauge->GetValue(); },
			[&value = m_value.get(), toGauge] { return toGauge(value); },
			[gauge](int v) { gauge->SetValue(v); });
	}
}

// ComboBoxWrapper -----------------------------------------------------------

template <ComboBoxValue T>
void ComboBoxWrapper<T>::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("ComboBoxWrapper::realize()\t-> new wxComboBox()\n");
#endif
	wxArrayString items;
	for (const auto& c : m_choices)
		items.Add(c);
	const T& selected = m_value.get();
	auto* combo = new wxComboBox(static_cast<wxWindow*>(parentWindow), wxID_ANY, "",
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), items, m_style);
	if constexpr (std::is_same_v<T, std::string>)
		combo->SetValue(selected);
	else
		combo->SetSelection(selected);
	m_nativeWidget = combo;

	if (m_value.isBound())
	{
		auto& value = m_value.get();
		if constexpr (std::is_same_v<T, std::string>)
			combo->Bind(wxEVT_COMBOBOX, [&value, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent& evt) {
				value = evt.GetString().ToStdString();
				if (cb) cb(value);
				else if (cbw) cbw(value, nw);
			});
		else
			combo->Bind(wxEVT_COMBOBOX, [&value, cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent& evt) {
				value = evt.GetSelection();
				if (cb) cb(value);
				else if (cbw) cbw(value, nw);
			});
		if constexpr (std::is_same_v<T, std::string>)
			bindExternalRefSync(combo,
				[combo] { return combo->GetValue().ToStdString(); },
				[&value] { return value; },
				[combo](const std::string& v) { combo->ChangeValue(v); });
		else
			bindExternalRefSync(combo,
				[combo] { return combo->GetSelection(); },
				[&value] { return static_cast<int>(value); },
				[combo](int i) { combo->SetSelection(i); });
	}
	else if (m_onChange)
	{
		if constexpr (std::is_same_v<T, std::string>)
			combo->Bind(wxEVT_COMBOBOX, [cb = std::move(m_onChange)](wxCommandEvent& evt) { cb(evt.GetString().ToStdString()); });
		else
			combo->Bind(wxEVT_COMBOBOX, [cb = std::move(m_onChange)](wxCommandEvent& evt) { cb(static_cast<T>(evt.GetSelection())); });
	}
	else if (m_onChangeWithWidget)
	{
		if constexpr (std::is_same_v<T, std::string>)
			combo->Bind(wxEVT_COMBOBOX, [cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent& evt) { cbw(evt.GetString().ToStdString(), nw); });
		else
			combo->Bind(wxEVT_COMBOBOX, [cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent& evt) { cbw(static_cast<T>(evt.GetSelection()), nw); });
	}

}

template class ComboBoxWrapper<std::string>;
template class ComboBoxWrapper<int>;

// ListBoxWrapper -----------------------------------------------------------

namespace
{

// wx spells "no selection" as wxNOT_FOUND for single-select boxes and as an
// empty selection array for multi-select ones; both arrive here as an empty
// index list.
std::vector<int> listBoxSelection(const wxListBox* list, bool multiSelect)
{
	std::vector<int> indices;
	if (multiSelect)
	{
		wxArrayInt selections;
		list->GetSelections(selections);
		indices.reserve(selections.GetCount());
		for (int index : selections)
			indices.push_back(index);
	}
	else if (const int selection = list->GetSelection(); selection != wxNOT_FOUND)
	{
		indices.push_back(selection);
	}
	return indices;
}

// Programmatic selection: wx setters do not fire wxEVT_LISTBOX, so this never
// re-enters the user's onChange -- which is what the ref sync needs of a push.
void setListBoxSelection(wxListBox* list, const std::vector<int>& indices, bool multiSelect)
{
	if (!multiSelect)
	{
		list->SetSelection(indices.empty() ? wxNOT_FOUND : indices.front());
		return;
	}

	for (unsigned int i = 0; i < list->GetCount(); ++i)
	{
		if (std::find(indices.begin(), indices.end(), static_cast<int>(i)) != indices.end())
			list->SetSelection(i);
		else
			list->Deselect(i);
	}
}

} // unnamed namespace

template <ListBoxValue T>
void ListBoxWrapper<T>::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("ListBoxWrapper::realize()\t-> new wxListBox()\n");
#endif
	wxArrayString items;
	for (const auto& item : m_items)
		items.Add(item);

	// wxLB_EXTENDED gives ctrl/shift-click range selection; wxLB_MULTIPLE would
	// toggle on a plain click, which is not what a desktop list does.
	const long selectionStyle = kMultiSelect ? wxLB_EXTENDED : wxLB_SINGLE;
	auto* list = new wxListBox(static_cast<wxWindow*>(parentWindow), wxID_ANY,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), items,
		m_style | selectionStyle | wxLB_NEEDED_SB);
	setListBoxSelection(list, indicesFor(m_items, boundValue()), kMultiSelect);
	m_nativeWidget = list;

	// wxListBox's own best height grows with the item count, so a long list would
	// ask the engine for a window taller than the screen. Pin it to visibleRows --
	// the same height the Qt and ImGui wrappers compute -- and keep wx's
	// content-derived best width.
	constexpr int kListBoxFrame = 6; // border the native box draws around its rows
	const wxSize best = list->GetBestSize();
	const int rowHeight = list->GetCharHeight() + 2;
	list->CacheBestSize(wxSize(best.x, rowHeight * m_visibleRows + kListBoxFrame));

	if (m_value.isBound())
	{
		auto& value = m_value.get();
		list->Bind(wxEVT_LISTBOX, [&value, list, items = m_items, cb = std::move(m_onChange),
			cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent&) {
			value = valueFor(items, listBoxSelection(list, kMultiSelect));
			if (cb) cb(value);
			else if (cbw) cbw(value, nw);
		});
		bindExternalRefSync(list,
			[list] { return listBoxSelection(list, kMultiSelect); },
			[&value, items = m_items] { return indicesFor(items, value); },
			[list](const std::vector<int>& indices) { setListBoxSelection(list, indices, kMultiSelect); });
	}
	else if (m_onChange)
	{
		list->Bind(wxEVT_LISTBOX, [list, items = m_items, cb = std::move(m_onChange)](wxCommandEvent&) {
			cb(valueFor(items, listBoxSelection(list, kMultiSelect)));
		});
	}
	else if (m_onChangeWithWidget)
	{
		list->Bind(wxEVT_LISTBOX, [list, items = m_items, cbw = std::move(m_onChangeWithWidget),
			nw = m_nativeWidget](wxCommandEvent&) {
			cbw(valueFor(items, listBoxSelection(list, kMultiSelect)), nw);
		});
	}
}

template class ListBoxWrapper<int>;
template class ListBoxWrapper<std::string>;
template class ListBoxWrapper<std::vector<int>>;
template class ListBoxWrapper<std::vector<std::string>>;

// TreeViewWrapper -----------------------------------------------------------

namespace
{

// Carries an item's full path on the native item, so a selection event decodes
// to a path with a single lookup instead of walking back up through labels --
// which would also be ambiguous once two siblings share a name.
class TreePathData : public wxTreeItemData
{
public:
	explicit TreePathData(std::string itemPath)
		: path(std::move(itemPath))
	{
	}

	std::string path;
};

std::string treeItemPath(const wxTreeCtrl* tree, const wxTreeItemId& id)
{
	if (!id.IsOk())
		return {};
	const auto* data = static_cast<TreePathData*>(tree->GetItemData(id));
	return data ? data->path : std::string {};
}

// Depth-first over every item below `parent`. The hidden root is never visited
// with itself as an argument, so it never appears in a selection.
void forEachTreeItem(wxTreeCtrl* tree, const wxTreeItemId& parent,
	const std::function<void(const wxTreeItemId&)>& fn)
{
	wxTreeItemIdValue cookie;
	for (wxTreeItemId id = tree->GetFirstChild(parent, cookie); id.IsOk();
		id = tree->GetNextChild(parent, cookie))
	{
		fn(id);
		forEachTreeItem(tree, id, fn);
	}
}

std::vector<std::string> treeSelection(wxTreeCtrl* tree, bool multiSelect)
{
	std::vector<std::string> paths;
	if (multiSelect)
	{
		wxArrayTreeItemIds ids;
		tree->GetSelections(ids);
		paths.reserve(ids.GetCount());
		for (const auto& id : ids)
			paths.push_back(treeItemPath(tree, id));
	}
	else if (const wxTreeItemId id = tree->GetSelection(); id.IsOk())
	{
		paths.push_back(treeItemPath(tree, id));
	}
	// The hidden root carries no TreePathData and wx can report it selected;
	// an empty path is that non-item, not a selection.
	paths.erase(std::remove(paths.begin(), paths.end(), std::string {}), paths.end());
	return paths;
}

// Programmatic selection. Unlike the list-box setters, wxTreeCtrl::SelectItem
// DOES fire wxEVT_TREE_SEL_CHANGED, so every caller here has to hold the
// re-entry guard the ref sync installs -- see realize().
void setTreeSelection(wxTreeCtrl* tree, const std::vector<std::string>& paths, bool multiSelect)
{
	if (multiSelect)
		tree->UnselectAll();
	else
		tree->Unselect();

	if (paths.empty())
		return;

	forEachTreeItem(tree, tree->GetRootItem(), [&](const wxTreeItemId& id) {
		const std::string path = treeItemPath(tree, id);
		if (multiSelect
			? std::find(paths.begin(), paths.end(), path) != paths.end()
			: path == paths.front())
		{
			tree->SelectItem(id);
		}
	});
}

void addTreeItems(wxTreeCtrl* tree, const wxTreeItemId& parent,
	const std::vector<TreeItem>& items, const std::string& parentPath, char separator)
{
	for (const TreeItem& item : items)
	{
		const std::string path = parentPath.empty()
			? item.label
			: parentPath + separator + item.label;
		const wxTreeItemId id = tree->AppendItem(parent, item.label, -1, -1, new TreePathData(path));
		addTreeItems(tree, id, item.children, path, separator);
		// Expanding is done after the children exist: wx has nothing to expand
		// on a childless item and silently ignores the call.
		if (item.expanded && !item.children.empty())
			tree->Expand(id);
	}
}

} // unnamed namespace

template <TreeViewValue T>
void TreeViewWrapper<T>::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("TreeViewWrapper::realize()\t-> new wxTreeCtrl()\n");
#endif
	// wxTreeCtrl has exactly one root, but a TreeItem list has many top-level
	// items, so the root is a hidden placeholder every top-level item hangs off.
	const long selectionStyle = m_multiSelect ? wxTR_MULTIPLE : wxTR_SINGLE;
	auto* tree = new wxTreeCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height),
		m_style | selectionStyle | wxTR_HIDE_ROOT | wxTR_HAS_BUTTONS | wxTR_NO_LINES);
	tree->AddRoot("");
	addTreeItems(tree, tree->GetRootItem(), m_items, std::string {}, kPathSeparator);
	m_nativeWidget = tree;

	// wxTreeCtrl's best size is its client area, not its content, so it would
	// ask the engine for whatever it happens to have been given. Compute the
	// same shape the other two backends do: widest indented label, visibleRows
	// of height.
	constexpr int kTreeFrame = 6;      // border the native control draws
	constexpr int kScrollbarSlack = 20; // room for the vertical scrollbar
	int widest = 0;
	forEachItem(m_items, [&](const TreeItem& item, const std::string&, int depth) {
		widest = std::max(widest,
			static_cast<int>(tree->GetIndent()) * (depth + 1)
				+ tree->GetTextExtent(item.label).GetWidth());
	});
	const int rowHeight = tree->GetCharHeight() + 4;
	tree->CacheBestSize(wxSize(widest + kTreeFrame + kScrollbarSlack,
		rowHeight * m_visibleRows + kTreeFrame));

	setTreeSelection(tree, pathsFor(boundValue()), m_multiSelect);

	if (m_value.isBound())
	{
		auto& value = m_value.get();
		// SelectItem notifies, so mirroring the ref into the control would
		// re-enter this handler and echo the value straight back out. The guard
		// is shared by both lambdas and lives as long as they do.
		auto syncing = std::make_shared<bool>(false);
		tree->Bind(wxEVT_TREE_SEL_CHANGED, [&value, tree, syncing, multi = m_multiSelect,
			cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget),
			nw = m_nativeWidget](wxTreeEvent& evt) {
			evt.Skip();
			if (*syncing)
				return;
			value = valueFor(treeSelection(tree, multi));
			if (cb) cb(value);
			else if (cbw) cbw(value, nw);
		});
		bindExternalRefSync(tree,
			[tree, multi = m_multiSelect] { return treeSelection(tree, multi); },
			[&value] { return pathsFor(value); },
			[tree, syncing, multi = m_multiSelect](const std::vector<std::string>& paths) {
				*syncing = true;
				setTreeSelection(tree, paths, multi);
				*syncing = false;
			});
	}
	else if (m_onChange)
	{
		tree->Bind(wxEVT_TREE_SEL_CHANGED, [tree, multi = m_multiSelect,
			cb = std::move(m_onChange)](wxTreeEvent& evt) {
			evt.Skip();
			cb(valueFor(treeSelection(tree, multi)));
		});
	}
	else if (m_onChangeWithWidget)
	{
		tree->Bind(wxEVT_TREE_SEL_CHANGED, [tree, multi = m_multiSelect,
			cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxTreeEvent& evt) {
			evt.Skip();
			cbw(valueFor(treeSelection(tree, multi)), nw);
		});
	}
}

template class TreeViewWrapper<std::string>;
template class TreeViewWrapper<std::vector<std::string>>;

// TableWrapper -----------------------------------------------------------

namespace
{

// wxDataViewListCtrl rather than wxListCtrl or wxGrid. wxListCtrl in report
// mode can only edit its first column (wxLC_EDIT_LABELS), which rules it out
// the moment a Table has an editable column further right; wxGrid is a
// spreadsheet, with row headers and a cell-range selection model that looks
// nothing like QTableWidget or an ImGui table. wxDataViewListCtrl is the one
// wx control with per-column editability, per-column sorting and row
// selection -- the same three axes the other two backends offer.

// An item's ORIGINAL row index, which wx carries for us in the item data. It
// stays with the row when a column sort reorders the view, so it survives
// exactly what a view position does not.
int dataViewRowIndex(const wxDataViewListCtrl* view, const wxDataViewItem& item)
{
	return item.IsOk() ? static_cast<int>(view->GetItemData(item)) : -1;
}

std::vector<int> dataViewSelection(wxDataViewListCtrl* view, bool multiSelect)
{
	std::vector<int> indices;
	if (multiSelect)
	{
		wxDataViewItemArray items;
		view->GetSelections(items);
		indices.reserve(items.GetCount());
		for (const auto& item : items)
		{
			if (const int row = dataViewRowIndex(view, item); row >= 0)
				indices.push_back(row);
		}
	}
	else if (const int row = dataViewRowIndex(view, view->GetSelection()); row >= 0)
	{
		indices.push_back(row);
	}
	// Reported in original-index order, not the order wx happens to hold the
	// selection in, so a multi-select binding reads the same on all three
	// backends however the view is currently sorted.
	std::sort(indices.begin(), indices.end());
	return indices;
}

// Programmatic selection. wx sends wxEVT_DATAVIEW_SELECTION_CHANGED for these
// on the platforms whose native control notifies on selection (as wxTreeCtrl
// does, and unlike the list-box setters), so every caller here has to hold the
// re-entry guard the ref sync installs -- see realize().
void setDataViewSelection(wxDataViewListCtrl* view, const std::vector<int>& indices, bool multiSelect)
{
	view->UnselectAll();
	if (indices.empty())
		return;

	wxDataViewItemArray items;
	for (int viewRow = 0; viewRow < static_cast<int>(view->GetItemCount()); ++viewRow)
	{
		const wxDataViewItem item = view->RowToItem(viewRow);
		const int row = dataViewRowIndex(view, item);
		if (row < 0)
			continue;
		if (multiSelect
			? std::find(indices.begin(), indices.end(), row) != indices.end()
			: row == indices.front())
		{
			items.Add(item);
		}
	}

	if (multiSelect)
		view->SetSelections(items);
	else if (!items.IsEmpty())
		view->Select(items[0]);
}

} // unnamed namespace

template <TableValue T>
void TableWrapper<T>::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("TableWrapper::realize()\t-> new wxDataViewListCtrl()\n");
#endif
	// wxDV_MULTIPLE is wx's ctrl/shift-click mode; wxDV_SINGLE is the default
	// and is spelled out here so the two read as a pair.
	const long selectionStyle = kMultiSelect ? wxDV_MULTIPLE : wxDV_SINGLE;
	auto* view = new wxDataViewListCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height),
		m_style | selectionStyle | wxDV_ROW_LINES);
	m_nativeWidget = view;

	const TableRows& rows = m_rows.get();
	const auto measureText = [view](const std::string& text) {
		return view->GetTextExtent(text).GetWidth();
	};

	// Columns are given the width the shared policy computes rather than wx's
	// wxCOL_WIDTH_DEFAULT, so a measured column comes out the same width here as
	// it does on Qt and ImGui.
	constexpr int kCellPadding = 12; // wx draws the cell's text inset on both sides
	int contentWidth = 0;
	for (int column = 0; column < static_cast<int>(m_columns.size()); ++column)
	{
		const TableColumn& spec = m_columns[column];
		const int width = columnWidth(m_columns, rows, column, measureText) + kCellPadding;
		contentWidth += width;
		view->AppendTextColumn(spec.label,
			spec.editable ? wxDATAVIEW_CELL_EDITABLE : wxDATAVIEW_CELL_INERT,
			width, wxALIGN_LEFT,
			wxDATAVIEW_COL_RESIZABLE | (spec.sortable ? wxDATAVIEW_COL_SORTABLE : 0));
	}

	for (int row = 0; row < static_cast<int>(rows.size()); ++row)
	{
		wxVector<wxVariant> values;
		values.reserve(m_columns.size());
		for (int column = 0; column < static_cast<int>(m_columns.size()); ++column)
			values.push_back(wxVariant(wxString(cellText(rows, row, column))));
		// The original index rides along as item data -- see dataViewRowIndex().
		view->AppendItem(values, static_cast<wxUIntPtr>(row));
	}

	// wxDataViewListCtrl's best size is its client area rather than its content,
	// so it would ask the engine for whatever it happens to have been given.
	// Compute the same shape the other two backends do: summed column widths,
	// a header plus visibleRows of body.
	constexpr int kViewFrame = 6;       // border the native control draws
	constexpr int kScrollbarSlack = 20; // room for the vertical scrollbar
	const int rowHeight = view->GetCharHeight() + 6;
	view->CacheBestSize(wxSize(contentWidth + kViewFrame + kScrollbarSlack,
		rowHeight * (m_visibleRows + 1) + kViewFrame));

	setDataViewSelection(view, rowIndicesFor(rows, boundValue()), kMultiSelect);

	// Rows the handlers read: the caller's own while bound, so an edit made
	// anywhere is visible here, and a snapshot otherwise. Never the wrapper's --
	// wrapper and window teardown order is not fixed (see wx/RefSync.hpp), and
	// the snapshot is shared rather than copied into each lambda.
	TableRows* editTarget = boundRows();
	auto snapshot = std::make_shared<const TableRows>(rows);
	const auto liveRows = [editTarget, snapshot]() -> const TableRows& {
		return editTarget ? *editTarget : *snapshot;
	};

	if (std::any_of(m_columns.begin(), m_columns.end(),
		[](const TableColumn& column) { return column.editable; }))
	{
		view->Bind(wxEVT_DATAVIEW_ITEM_EDITING_DONE, [view, editTarget,
			cb = std::move(m_onCellChange)](wxDataViewEvent& evt) {
			evt.Skip();
			// Escape leaves the cell untouched, and wx says so here rather than
			// by withholding the event.
			if (evt.IsEditCancelled())
				return;
			const int row = dataViewRowIndex(view, evt.GetItem());
			if (row < 0)
				return;
			const std::string text = evt.GetValue().GetString().ToStdString();
			applyCellEdit(editTarget, row, evt.GetColumn(), text);
			if (cb)
				cb(row, evt.GetColumn(), text);
		});
	}

	if (m_value.isBound())
	{
		auto& value = m_value.get();
		// The selection setters notify, so mirroring the ref into the control
		// would re-enter this handler and echo the value straight back out. The
		// guard is shared by both lambdas and lives as long as they do.
		auto syncing = std::make_shared<bool>(false);
		view->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, [&value, view, syncing, liveRows,
			cb = std::move(m_onChange), cbw = std::move(m_onChangeWithWidget),
			nw = m_nativeWidget](wxDataViewEvent& evt) {
			evt.Skip();
			if (*syncing)
				return;
			value = valueFor(liveRows(), dataViewSelection(view, kMultiSelect));
			if (cb) cb(value);
			else if (cbw) cbw(value, nw);
		});
		bindExternalRefSync(view,
			[view] { return dataViewSelection(view, kMultiSelect); },
			[&value, liveRows] { return rowIndicesFor(liveRows(), value); },
			[view, syncing](const std::vector<int>& indices) {
				*syncing = true;
				setDataViewSelection(view, indices, kMultiSelect);
				*syncing = false;
			});
	}
	else if (m_onChange)
	{
		view->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, [view, liveRows,
			cb = std::move(m_onChange)](wxDataViewEvent& evt) {
			evt.Skip();
			cb(valueFor(liveRows(), dataViewSelection(view, kMultiSelect)));
		});
	}
	else if (m_onChangeWithWidget)
	{
		view->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, [view, liveRows,
			cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxDataViewEvent& evt) {
			evt.Skip();
			cbw(valueFor(liveRows(), dataViewSelection(view, kMultiSelect)), nw);
		});
	}
}

template class TableWrapper<int>;
template class TableWrapper<std::string>;
template class TableWrapper<std::vector<int>>;
template class TableWrapper<std::vector<std::string>>;
