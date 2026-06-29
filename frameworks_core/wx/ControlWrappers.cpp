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

// Constructors only collect data and live inline in ControlWrappers.hpp.
// createAndAdd() creates the native wxWidget from the collected data (plus the
// position/size/style stored in the ControlWrapper base), binds its events, and
// then registers it with the layout via ControlWrapper::createAndAdd().

namespace
{
	inline wxWindow* parentWindow(ControlWrapper* parent)
	{
		return reinterpret_cast<wxWindow*>(parent->nativeHandle());
	}
}

// ButtonWrapper -----------------------------------------------------------

void ButtonWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ButtonWrapper::createAndAdd()\t-> new wxButton()\n");
#endif
	auto* btn = new wxButton(parentWindow(parent), wxID_ANY, m_label,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	m_nativeWidget = btn;

	if (m_onClick)
		btn->Bind(wxEVT_BUTTON, [cb = std::move(m_onClick)](wxCommandEvent&) { cb(); });
	else if (m_onClickWithWidget)
		btn->Bind(wxEVT_BUTTON, [cb = std::move(m_onClickWithWidget), nw = m_nativeWidget](wxCommandEvent&) { cb(nw); });

	ControlWrapper::createAndAdd(parent, layout, flags);
}

// TextCtrlWrapper -----------------------------------------------------------

void TextCtrlWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "TextCtrlWrapper::createAndAdd()\t-> new wxTextCtrl()\n");
#endif
	const std::string& initial = m_externalRef ? m_externalRef->get() : m_ownedValue;
	auto* txt = new wxTextCtrl(parentWindow(parent), wxID_ANY, initial,
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
	}
	else if (m_onChange)
		txt->Bind(wxEVT_TEXT, [cb = std::move(m_onChange)](wxCommandEvent& evt) { cb(evt.GetString().ToStdString()); });
	else if (m_onChangeWithWidget)
		txt->Bind(wxEVT_TEXT, [cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent& evt) { cbw(evt.GetString().ToStdString(), nw); });

	ControlWrapper::createAndAdd(parent, layout, flags);
}

// PasswordInputWrapper -----------------------------------------------------------

void PasswordInputWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "PasswordInputWrapper::createAndAdd()\t-> new wxTextCtrl(wxTE_PASSWORD)\n");
#endif
	const std::string& initial = m_externalRef ? m_externalRef->get() : m_ownedValue;
	auto* txt = new wxTextCtrl(parentWindow(parent), wxID_ANY, initial,
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
	}
	else if (m_onChange)
		txt->Bind(wxEVT_TEXT, [cb = std::move(m_onChange)](wxCommandEvent& evt) { cb(evt.GetString().ToStdString()); });
	else if (m_onChangeWithWidget)
		txt->Bind(wxEVT_TEXT, [cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent& evt) { cbw(evt.GetString().ToStdString(), nw); });

	ControlWrapper::createAndAdd(parent, layout, flags);
}

// MultiLineTextCtrlWrapper -----------------------------------------------------------

void MultiLineTextCtrlWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "MultiLineTextCtrlWrapper::createAndAdd()\t-> new wxTextCtrl(wxTE_MULTILINE)\n");
#endif
	const std::string& initial = m_externalRef ? m_externalRef->get() : m_ownedValue;
	auto* txt = new wxTextCtrl(parentWindow(parent), wxID_ANY, initial,
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
	}
	else if (m_onChange)
		txt->Bind(wxEVT_TEXT, [cb = std::move(m_onChange)](wxCommandEvent& evt) { cb(evt.GetString().ToStdString()); });
	else if (m_onChangeWithWidget)
		txt->Bind(wxEVT_TEXT, [cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent& evt) { cbw(evt.GetString().ToStdString(), nw); });

	ControlWrapper::createAndAdd(parent, layout, flags);
}

// ReadonlyTextCtrlWrapper -----------------------------------------------------------

void ReadonlyTextCtrlWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ReadonlyTextCtrlWrapper::createAndAdd()\t-> new wxTextCtrl(wxTE_READONLY)\n");
#endif
	m_nativeWidget = new wxTextCtrl(parentWindow(parent), wxID_ANY, m_value,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style | wxTE_READONLY);

	ControlWrapper::createAndAdd(parent, layout, flags);
}

// ClickableTextWrapper -----------------------------------------------------------

void ClickableTextWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ClickableTextWrapper::createAndAdd()\t-> new wxStaticText()\n");
#endif
	auto* st = new wxStaticText(parentWindow(parent), wxID_ANY, m_text,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	m_nativeWidget = st;

	if (m_onClick)
		st->Bind(wxEVT_LEFT_DOWN, [cb = std::move(m_onClick)](wxMouseEvent&) { cb(); });
	else if (m_onClickWithWidget)
		st->Bind(wxEVT_LEFT_DOWN, [cb = std::move(m_onClickWithWidget), nw = m_nativeWidget](wxMouseEvent&) { cb(nw); });

	ControlWrapper::createAndAdd(parent, layout, flags);
}

// LinkTextWrapper -----------------------------------------------------------

void LinkTextWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "LinkTextWrapper::createAndAdd()\t-> new wxHyperlinkCtrl()\n");
#endif
	auto* link = new wxHyperlinkCtrl(parentWindow(parent), wxID_ANY, m_text, wxEmptyString,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	m_nativeWidget = link;

	if (m_onClick)
		link->Bind(wxEVT_HYPERLINK, [cb = std::move(m_onClick)](wxHyperlinkEvent&) { cb(); });
	else if (m_onClickWithWidget)
		link->Bind(wxEVT_HYPERLINK, [cb = std::move(m_onClickWithWidget), nw = m_nativeWidget](wxHyperlinkEvent&) { cb(nw); });

	ControlWrapper::createAndAdd(parent, layout, flags);
}

// StaticTextWrapper -----------------------------------------------------------

void StaticTextWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "StaticTextWrapper::createAndAdd()\t-> new wxStaticText()\n");
#endif
	m_nativeWidget = new wxStaticText(parentWindow(parent), wxID_ANY, m_text,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);

	ControlWrapper::createAndAdd(parent, layout, flags);
}

// DatePickerWrapper -----------------------------------------------------------

void DatePickerWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "DatePickerWrapper::createAndAdd()\t-> new wxDatePickerCtrl()\n");
#endif
	const Date& dval = m_externalRef ? m_externalRef->get() : m_ownedValue;
	wxDateTime dt;
	dt.Set(static_cast<wxDateTime::wxDateTime_t>(dval.day),
		static_cast<wxDateTime::Month>(dval.month - 1),
		dval.year);
	auto* dp = new wxDatePickerCtrl(parentWindow(parent), wxID_ANY, dt,
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

	ControlWrapper::createAndAdd(parent, layout, flags);
}

// TimePickerWrapper -----------------------------------------------------------

void TimePickerWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "TimePickerWrapper::createAndAdd()\t-> new wxTimePickerCtrl()\n");
#endif
	const Time& tval = m_externalRef ? m_externalRef->get() : m_ownedValue;
	wxDateTime dt = wxDateTime::Now();
	dt.SetHour(tval.hour);
	dt.SetMinute(tval.minute);
	dt.SetSecond(tval.second);
	auto* tp = new wxTimePickerCtrl(parentWindow(parent), wxID_ANY, dt,
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

	ControlWrapper::createAndAdd(parent, layout, flags);
}

// SliderWrapper -----------------------------------------------------------

template <SliderValue T>
void SliderWrapper<T>::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "SliderWrapper::createAndAdd()\t-> new wxSlider()\n");
#endif
	const T& val = m_externalRef ? m_externalRef->get() : m_ownedValue;
	wxSlider* sl = nullptr;
	if constexpr (std::is_floating_point_v<T>)
	{
		int iMin = static_cast<int>(m_range.min / m_range.step);
		int iMax = static_cast<int>(m_range.max / m_range.step);
		int iVal = static_cast<int>(val / m_range.step);
		sl = new wxSlider(parentWindow(parent), wxID_ANY, iVal, iMin, iMax,
			wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	}
	else
	{
		sl = new wxSlider(parentWindow(parent), wxID_ANY, static_cast<int>(val), m_range.min, m_range.max,
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

	ControlWrapper::createAndAdd(parent, layout, flags);
}

template class SliderWrapper<int>;
template class SliderWrapper<float>;

// SpinBoxWrapper -----------------------------------------------------------

template <SpinBoxValue T>
void SpinBoxWrapper<T>::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "SpinBoxWrapper::createAndAdd()\t-> new wxSpinCtrl[Double]()\n");
#endif
	const T& val = m_externalRef ? m_externalRef->get() : m_ownedValue;
	if constexpr (std::is_same_v<T, int>)
	{
		auto* spin = new wxSpinCtrl(parentWindow(parent), wxID_ANY, wxEmptyString,
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
		}
		else if (m_onChange)
			spin->Bind(wxEVT_SPINCTRL, [cb = std::move(m_onChange)](wxSpinEvent& evt) { cb(evt.GetInt()); });
		else if (m_onChangeWithWidget)
			spin->Bind(wxEVT_SPINCTRL, [cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxSpinEvent& evt) { cbw(evt.GetInt(), nw); });
	}
	else
	{
		auto* spin = new wxSpinCtrlDouble(parentWindow(parent), wxID_ANY, wxEmptyString,
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
		}
		else if (m_onChange)
			spin->Bind(wxEVT_SPINCTRLDOUBLE, [cb = std::move(m_onChange)](wxSpinDoubleEvent& evt) { cb(static_cast<T>(evt.GetValue())); });
		else if (m_onChangeWithWidget)
			spin->Bind(wxEVT_SPINCTRLDOUBLE, [cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxSpinDoubleEvent& evt) { cbw(static_cast<T>(evt.GetValue()), nw); });
	}

	ControlWrapper::createAndAdd(parent, layout, flags);
}

template class SpinBoxWrapper<int>;
template class SpinBoxWrapper<float>;

// RadioButtonWrapper -----------------------------------------------------------

template <RadioButtonValue T>
void RadioButtonWrapper<T>::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "RadioButtonWrapper::createAndAdd()\t-> new wxRadioButton()\n");
#endif
	const T& val = m_externalRef ? m_externalRef->get() : m_ownedValue;
	wxRadioButton* rb = nullptr;
	if constexpr (std::is_same_v<T, bool>)
	{
		rb = new wxRadioButton(parentWindow(parent), wxID_ANY, m_label,
			wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
		rb->SetValue(val);
	}
	else
	{
		// m_index (assigned at construction) is the radio's position within its
		// group; index 0 starts a new wx radio group.
		long groupStyle = (m_index == 0) ? wxRB_GROUP : 0;
		rb = new wxRadioButton(parentWindow(parent), wxID_ANY, m_label,
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

	ControlWrapper::createAndAdd(parent, layout, flags);
}

template class RadioButtonWrapper<bool>;
template class RadioButtonWrapper<int>;

// CheckBoxWrapper -----------------------------------------------------------

void CheckBoxWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "CheckBoxWrapper::createAndAdd()\t-> new wxCheckBox()\n");
#endif
	const bool checked = m_externalRef ? m_externalRef->get() : m_ownedValue;
	auto* chk = new wxCheckBox(parentWindow(parent), wxID_ANY, m_label,
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
	}
	else if (m_onChange)
		chk->Bind(wxEVT_CHECKBOX, [cb = std::move(m_onChange)](wxCommandEvent& evt) { cb(evt.IsChecked()); });
	else if (m_onChangeWithWidget)
		chk->Bind(wxEVT_CHECKBOX, [cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent& evt) { cbw(evt.IsChecked(), nw); });

	ControlWrapper::createAndAdd(parent, layout, flags);
}

// ToggleButtonWrapper -----------------------------------------------------------

void ToggleButtonWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ToggleButtonWrapper::createAndAdd()\t-> new wxToggleButton()\n");
#endif
	const bool toggled = m_externalRef ? m_externalRef->get() : m_ownedValue;
	auto* btn = new wxToggleButton(parentWindow(parent), wxID_ANY, m_label,
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
	}
	else if (m_onChange)
		btn->Bind(wxEVT_TOGGLEBUTTON, [cb = std::move(m_onChange)](wxCommandEvent& evt) { cb(evt.IsChecked()); });
	else if (m_onChangeWithWidget)
		btn->Bind(wxEVT_TOGGLEBUTTON, [cbw = std::move(m_onChangeWithWidget), nw = m_nativeWidget](wxCommandEvent& evt) { cbw(evt.IsChecked(), nw); });

	ControlWrapper::createAndAdd(parent, layout, flags);
}

// ImageWrapper -----------------------------------------------------------

void ImageWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ImageWrapper::createAndAdd()\t-> new wxStaticBitmap()\n");
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
	auto* bmpCtrl = new wxStaticBitmap(parentWindow(parent), wxID_ANY, bmp,
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

	ControlWrapper::createAndAdd(parent, layout, flags);
}

// ColorPickerWrapper -----------------------------------------------------------

void ColorPickerWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ColorPickerWrapper::createAndAdd()\t-> new wxColourPickerCtrl()\n");
#endif
	const Color& cval = m_externalRef ? m_externalRef->get() : m_ownedValue;
	auto* picker = new wxColourPickerCtrl(parentWindow(parent), wxID_ANY,
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

	ControlWrapper::createAndAdd(parent, layout, flags);
}

// SeparatorWrapper -----------------------------------------------------------

void SeparatorWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "SeparatorWrapper::createAndAdd()\t-> new wxStaticLine()\n");
#endif
	m_nativeWidget = new wxStaticLine(parentWindow(parent), wxID_ANY,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style | wxLI_HORIZONTAL);

	ControlWrapper::createAndAdd(parent, layout, flags);
}

// ProgressBarWrapper -----------------------------------------------------------

void ProgressBarWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ProgressBarWrapper::createAndAdd()\t-> new wxGauge()\n");
#endif
	const float value = m_externalRef ? m_externalRef->get() : m_ownedValue;
	auto* gauge = new wxGauge(parentWindow(parent), wxID_ANY, 100,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style | wxGA_HORIZONTAL | wxGA_SMOOTH);
	gauge->SetValue(static_cast<int>(std::clamp(value, 0.0f, 1.0f) * 100));
	m_nativeWidget = gauge;

	ControlWrapper::createAndAdd(parent, layout, flags);
}

// ComboBoxWrapper -----------------------------------------------------------

template <ComboBoxValue T>
void ComboBoxWrapper<T>::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ComboBoxWrapper::createAndAdd()\t-> new wxComboBox()\n");
#endif
	wxArrayString items;
	for (const auto& c : m_choices)
		items.Add(c);
	const T& selected = m_externalRef ? m_externalRef->get() : m_ownedSelected;
	auto* combo = new wxComboBox(parentWindow(parent), wxID_ANY, "",
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

	ControlWrapper::createAndAdd(parent, layout, flags);
}

template class ComboBoxWrapper<std::string>;
template class ComboBoxWrapper<int>;
