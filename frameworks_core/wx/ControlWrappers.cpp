#include "frameworks_core/ControlWrappers.hpp"
#include "frameworks_core/wx/RefSync.hpp"
#include <algorithm>
#include <cmath>
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
	const std::string& initial = m_externalRef ? m_externalRef->get() : m_ownedValue;
	auto* txt = new wxTextCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY, initial,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	m_nativeWidget = txt;

	if (m_externalRef)
	{
		auto& value = m_externalRef->get();
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
	const std::string& initial = m_externalRef ? m_externalRef->get() : m_ownedValue;
	auto* txt = new wxTextCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY, initial,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style | wxTE_PASSWORD);
	m_nativeWidget = txt;

	if (m_externalRef)
	{
		auto& value = m_externalRef->get();
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
	const std::string& initial = m_externalRef ? m_externalRef->get() : m_ownedValue;
	auto* txt = new wxTextCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY, initial,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style | wxTE_MULTILINE);
	m_nativeWidget = txt;

	if (m_externalRef)
	{
		auto& value = m_externalRef->get();
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
	const Date& dval = m_externalRef ? m_externalRef->get() : m_ownedValue;
	wxDateTime dt;
	dt.Set(static_cast<wxDateTime::wxDateTime_t>(dval.day),
		static_cast<wxDateTime::Month>(dval.month - 1),
		dval.year);
	auto* dp = new wxDatePickerCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY, dt,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	m_nativeWidget = dp;

	if (m_externalRef)
	{
		auto& value = m_externalRef->get();
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
	const Time& tval = m_externalRef ? m_externalRef->get() : m_ownedValue;
	wxDateTime dt = wxDateTime::Now();
	dt.SetHour(tval.hour);
	dt.SetMinute(tval.minute);
	dt.SetSecond(tval.second);
	auto* tp = new wxTimePickerCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY, dt,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	m_nativeWidget = tp;

	if (m_externalRef)
	{
		auto& value = m_externalRef->get();
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
	const T& val = m_externalRef ? m_externalRef->get() : m_ownedValue;
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

	if (m_externalRef)
	{
		auto& value = m_externalRef->get();
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
	const T& val = m_externalRef ? m_externalRef->get() : m_ownedValue;
	if constexpr (std::is_same_v<T, int>)
	{
		auto* spin = new wxSpinCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY, wxEmptyString,
			wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style,
			m_range.min, m_range.max, static_cast<int>(val));
		m_nativeWidget = spin;

		if (m_externalRef)
		{
			auto& value = m_externalRef->get();
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

		if (m_externalRef)
		{
			auto& value = m_externalRef->get();
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
	const T& val = m_externalRef ? m_externalRef->get() : m_ownedValue;
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

	if (m_externalRef)
	{
		auto& value = m_externalRef->get();
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
	const bool checked = m_externalRef ? m_externalRef->get() : m_ownedValue;
	auto* chk = new wxCheckBox(static_cast<wxWindow*>(parentWindow), wxID_ANY, m_label,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	chk->SetValue(checked);
	m_nativeWidget = chk;

	if (m_externalRef)
	{
		auto& value = m_externalRef->get();
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
	const bool toggled = m_externalRef ? m_externalRef->get() : m_ownedValue;
	auto* btn = new wxToggleButton(static_cast<wxWindow*>(parentWindow), wxID_ANY, m_label,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	btn->SetValue(toggled);
	m_nativeWidget = btn;

	if (m_externalRef)
	{
		auto& value = m_externalRef->get();
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

// ColorPickerWrapper -----------------------------------------------------------

void ColorPickerWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("ColorPickerWrapper::realize()\t-> new wxColourPickerCtrl()\n");
#endif
	const Color& cval = m_externalRef ? m_externalRef->get() : m_ownedValue;
	auto* picker = new wxColourPickerCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY,
		wxColour(static_cast<unsigned char>(cval.r * 255),
		         static_cast<unsigned char>(cval.g * 255),
		         static_cast<unsigned char>(cval.b * 255),
		         static_cast<unsigned char>(cval.a * 255)),
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	m_nativeWidget = picker;

	if (m_externalRef)
	{
		auto& value = m_externalRef->get();
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
	m_nativeWidget = new wxStaticLine(static_cast<wxWindow*>(parentWindow), wxID_ANY,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style | wxLI_HORIZONTAL);

}

// ProgressBarWrapper -----------------------------------------------------------

void ProgressBarWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("ProgressBarWrapper::realize()\t-> new wxGauge()\n");
#endif
	// The bound float is a 0..100 percentage, matching the gauge's own integer range.
	const auto toGauge = [](float v) { return static_cast<int>(std::clamp(v, 0.0f, 100.0f)); };

	const float initial = m_externalRef ? m_externalRef->get() : m_ownedValue;
	auto* gauge = new wxGauge(static_cast<wxWindow*>(parentWindow), wxID_ANY, 100,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style | wxGA_HORIZONTAL | wxGA_SMOOTH);
	gauge->SetValue(toGauge(initial));
	m_nativeWidget = gauge;

	// A progress bar has no input events of its own -- the bound float is only ever
	// written from outside -- so the idle sync is the whole story here.
	if (m_externalRef)
	{
		bindExternalRefSync(gauge,
			[gauge] { return gauge->GetValue(); },
			[&value = m_externalRef->get(), toGauge] { return toGauge(value); },
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
	const T& selected = m_externalRef ? m_externalRef->get() : m_ownedSelected;
	auto* combo = new wxComboBox(static_cast<wxWindow*>(parentWindow), wxID_ANY, "",
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), items, m_style);
	if constexpr (std::is_same_v<T, std::string>)
		combo->SetValue(selected);
	else
		combo->SetSelection(selected);
	m_nativeWidget = combo;

	if (m_externalRef)
	{
		auto& value = m_externalRef->get();
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
