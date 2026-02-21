#include "frameworks_core/ControlWrappers.hpp"
#include "frameworks_core/LayoutWrapper.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#ifdef USE_WX
#include <wx/wx.h>
#include <wx/hyperlink.h>
#include <wx/spinctrl.h>
#include <wx/datectrl.h>
#include <wx/timectrl.h>
#include <wx/dateevt.h>

// ButtonWrapper -----------------------------------------------------------

ButtonWrapper::ButtonWrapper(ControlWrapper* parent, const std::string& label,
	const Position& pos, const Size& size, long style,
	std::function<void()> onClick)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ButtonWrapper::ButtonWrapper()\t-> new wxButton()\n");
#endif
	auto* btn = new wxButton(parent->nativeHandle(), wxID_ANY, label,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	if (onClick)
		btn->Bind(wxEVT_BUTTON, [cb = std::move(onClick)](wxCommandEvent&) { cb(); });
	m_nativeWidget = btn;
}

// TextCtrlWrapper -----------------------------------------------------------

TextCtrlWrapper::TextCtrlWrapper(ControlWrapper* parent, std::string& value,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "TextCtrlWrapper::TextCtrlWrapper()\t-> new wxTextCtrl()\n");
#endif
	auto* tc = new wxTextCtrl(parent->nativeHandle(), wxID_ANY, value,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	tc->Bind(wxEVT_TEXT, [&value](wxCommandEvent& evt) { value = evt.GetString().ToStdString(); });
	m_nativeWidget = tc;
}

// PasswordInputWrapper -----------------------------------------------------------

PasswordInputWrapper::PasswordInputWrapper(ControlWrapper* parent, std::string& value,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "PasswordInputWrapper::PasswordInputWrapper()\t-> new wxTextCtrl(wxTE_PASSWORD)\n");
#endif
	auto* tc = new wxTextCtrl(parent->nativeHandle(), wxID_ANY, value,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style | wxTE_PASSWORD);
	tc->Bind(wxEVT_TEXT, [&value](wxCommandEvent& evt) { value = evt.GetString().ToStdString(); });
	m_nativeWidget = tc;
}

// MultiLineTextCtrlWrapper -----------------------------------------------------------

MultiLineTextCtrlWrapper::MultiLineTextCtrlWrapper(ControlWrapper* parent, std::string& value,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "MultiLineTextCtrlWrapper::MultiLineTextCtrlWrapper()\t-> new wxTextCtrl(wxTE_MULTILINE)\n");
#endif
	auto* tc = new wxTextCtrl(parent->nativeHandle(), wxID_ANY, value,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style | wxTE_MULTILINE);
	tc->Bind(wxEVT_TEXT, [&value](wxCommandEvent& evt) { value = evt.GetString().ToStdString(); });
	m_nativeWidget = tc;
}

// ReadonlyTextCtrlWrapper -----------------------------------------------------------

ReadonlyTextCtrlWrapper::ReadonlyTextCtrlWrapper(ControlWrapper* parent, const std::string& value,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ReadonlyTextCtrlWrapper::ReadonlyTextCtrlWrapper()\t-> new wxTextCtrl(wxTE_READONLY)\n");
#endif
	m_nativeWidget = new wxTextCtrl(parent->nativeHandle(), wxID_ANY, value,
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
	auto* st = new wxStaticText(parent->nativeHandle(), wxID_ANY, text,
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
	auto* hl = new wxHyperlinkCtrl(parent->nativeHandle(), wxID_ANY, text, wxEmptyString,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	if (onClick)
		hl->Bind(wxEVT_HYPERLINK, [cb = std::move(onClick)](wxHyperlinkEvent& evt) { cb(); });
	m_nativeWidget = hl;
}

// DatePickerWrapper -----------------------------------------------------------

DatePickerWrapper::DatePickerWrapper(ControlWrapper* parent, Date& value,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "DatePickerWrapper::DatePickerWrapper()\t-> new wxDatePickerCtrl()\n");
#endif
	wxDateTime dt;
	dt.Set(static_cast<wxDateTime::wxDateTime_t>(value.day),
		static_cast<wxDateTime::Month>(value.month - 1),
		value.year);
	auto* dp = new wxDatePickerCtrl(parent->nativeHandle(), wxID_ANY, dt,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	dp->Bind(wxEVT_DATE_CHANGED, [&value](wxDateEvent& evt) {
		const wxDateTime& d = evt.GetDate();
		value.year  = d.GetYear();
		value.month = static_cast<int>(d.GetMonth()) + 1;
		value.day   = d.GetDay();
	});
	m_nativeWidget = dp;
}

// TimePickerWrapper -----------------------------------------------------------

TimePickerWrapper::TimePickerWrapper(ControlWrapper* parent, Time& value,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "TimePickerWrapper::TimePickerWrapper()\t-> new wxTimePickerCtrl()\n");
#endif
	wxDateTime dt = wxDateTime::Now();
	dt.SetHour(value.hour);
	dt.SetMinute(value.minute);
	dt.SetSecond(value.second);
	auto* tp = new wxTimePickerCtrl(parent->nativeHandle(), wxID_ANY, dt,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	tp->Bind(wxEVT_TIME_CHANGED, [&value](wxDateEvent& evt) {
		const wxDateTime& d = evt.GetDate();
		value.hour   = d.GetHour();
		value.minute = d.GetMinute();
		value.second = d.GetSecond();
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
	m_nativeWidget = new wxStaticText(parent->nativeHandle(), wxID_ANY, text,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
}

// SliderWrapper -----------------------------------------------------------

template <SliderValue T>
SliderWrapper<T>::SliderWrapper(ControlWrapper* parent, Range<T> range, T& value,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGERß
	Logger::instance().log(LayoutWrapper::indent() + "SliderWrapper::SliderWrapper()\t-> new wxSlider()\n");
#endif
	if constexpr (std::is_floating_point_v<T>)
	{
		int iMin = static_cast<int>(range.min / range.step);
		int iMax = static_cast<int>(range.max / range.step);
		int iVal = static_cast<int>(value / range.step);
		auto* sl = new wxSlider(parent->nativeHandle(), wxID_ANY,
			iVal, iMin, iMax,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
		sl->Bind(wxEVT_SLIDER, [&value, step = range.step](wxCommandEvent& evt) {
			value = static_cast<T>(evt.GetInt()) * step;
		});
		m_nativeWidget = sl;
	}
	else
	{
		auto* sl = new wxSlider(parent->nativeHandle(), wxID_ANY,
			value, range.min, range.max,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
		sl->Bind(wxEVT_SLIDER, [&value](wxCommandEvent& evt) { value = evt.GetInt(); });
		m_nativeWidget = sl;
	}
}

template class SliderWrapper<int>;
template class SliderWrapper<float>;

// SpinBoxWrapper -----------------------------------------------------------

template <SpinBoxValue T>
SpinBoxWrapper<T>::SpinBoxWrapper(ControlWrapper* parent, Range<T> range, T& value,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "SpinBoxWrapper::SpinBoxWrapper()\t-> new wxSpinCtrl[Double]()\n");
#endif
	if constexpr (std::is_same_v<T, int>)
	{
		auto* sc = new wxSpinCtrl(parent->nativeHandle(), wxID_ANY, wxEmptyString,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style,
			range.min, range.max, value);
		sc->Bind(wxEVT_SPINCTRL, [&value](wxSpinEvent& evt) { value = evt.GetInt(); });
		m_nativeWidget = sc;
	}
	else
	{
		auto* sc = new wxSpinCtrlDouble(parent->nativeHandle(), wxID_ANY, wxEmptyString,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style,
			range.min, range.max, value, range.step);
		sc->Bind(wxEVT_SPINCTRLDOUBLE, [&value](wxSpinDoubleEvent& evt) { value = static_cast<T>(evt.GetValue()); });
		m_nativeWidget = sc;
	}
}

template class SpinBoxWrapper<int>;
template class SpinBoxWrapper<float>;

// RadioButtonWrapper -----------------------------------------------------------

template <RadioButtonValue T>
RadioButtonWrapper<T>::RadioButtonWrapper(ControlWrapper* parent, const std::string& label,
	T& value, const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "RadioButtonWrapper::RadioButtonWrapper()\t-> new wxRadioButton()\n");
#endif
	if constexpr (std::is_same_v<T, bool>)
	{
		auto* rb = new wxRadioButton(parent->nativeHandle(), wxID_ANY, label,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
		rb->SetValue(value);
		rb->Bind(wxEVT_RADIOBUTTON, [&value](wxCommandEvent&) { value = true; });
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
		auto* rb = new wxRadioButton(parent->nativeHandle(), wxID_ANY, label,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style | groupStyle);
		rb->SetValue(value == index);
		rb->Bind(wxEVT_RADIOBUTTON, [&value, index](wxCommandEvent&) { value = index; });
		m_nativeWidget = rb;
	}
}

template class RadioButtonWrapper<bool>;
template class RadioButtonWrapper<int>;

// CheckBoxWrapper -----------------------------------------------------------

CheckBoxWrapper::CheckBoxWrapper(ControlWrapper* parent, const std::string& label,
	const Position& pos, const Size& size, long style,
	bool& checked)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "CheckBoxWrapper::CheckBoxWrapper()\t-> new wxCheckBox()\n");
#endif
	auto* cb = new wxCheckBox(parent->nativeHandle(), wxID_ANY, label,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	cb->SetValue(checked);
	cb->Bind(wxEVT_CHECKBOX, [&checked](wxCommandEvent& evt) { checked = evt.IsChecked(); });
	m_nativeWidget = cb;
}

// ComboBoxWrapper -----------------------------------------------------------

template <ComboBoxValue T>
ComboBoxWrapper<T>::ComboBoxWrapper(ControlWrapper* parent, std::vector<std::string> choices,
	T& selected, const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ComboBoxWrapper::ComboBoxWrapper()\t-> new wxComboBox()\n");
#endif
	wxArrayString items;
	for (const auto& c : choices)
		items.Add(c);
	auto* combo = new wxComboBox(parent->nativeHandle(), wxID_ANY, "",
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), items, style);

	if constexpr (std::is_same_v<T, std::string>)
	{
		combo->SetValue(selected);
		combo->Bind(wxEVT_COMBOBOX, [&selected](wxCommandEvent& evt) { selected = evt.GetString().ToStdString(); });
	}
	else
	{
		combo->SetSelection(selected);
		combo->Bind(wxEVT_COMBOBOX, [&selected](wxCommandEvent& evt) { selected = evt.GetSelection(); });
	}
	m_nativeWidget = combo;
}

template class ComboBoxWrapper<std::string>;
template class ComboBoxWrapper<int>;

#elif defined(USE_QT) // ------------------QT IMPLEMENTATIONS----------------
// Implementations for Qt would go here, following a similar pattern to the wxWidgets implementations but using Qt's widget classes and signal/slot mechanism.
#elif defined(USE_IMGUI) // ----------------IMGUI IMPLEMENTATIONS----------------
#include "imgui.h"
#include <algorithm>

// pos, size, style: not directly applicable in ImGui immediate mode

// ButtonWrapper -----------------------------------------------------------

ButtonWrapper::ButtonWrapper(ControlWrapper* parent, const std::string& label,
	const Position& pos, const Size& size, long style,
	std::function<void()> onClick)
	: m_label(label.empty() ? "##button" : label)
	, m_onClick(std::move(onClick))
{
}

void ButtonWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ButtonWrapper::createAndAdd()\t-> ImGui::Button()\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	if (ImGui::Button(m_label.c_str()))
	{
		if (m_onClick)
			m_onClick();
	}
}

// TextCtrlWrapper -----------------------------------------------------------

TextCtrlWrapper::TextCtrlWrapper(ControlWrapper* parent, std::string& value,
	const Position& pos, const Size& size, long style)
	: m_value(value)
{
}

void TextCtrlWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "TextCtrlWrapper::createAndAdd()\t-> ImGui::InputText()\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	char buf[256] = {};
	std::snprintf(buf, sizeof(buf), "%s", m_value.c_str());
	if (ImGui::InputText("##textctrl", buf, sizeof(buf)))
		m_value = buf;
}

// PasswordInputWrapper -----------------------------------------------------------

PasswordInputWrapper::PasswordInputWrapper(ControlWrapper* parent, std::string& value,
	const Position& pos, const Size& size, long style)
	: m_value(value)
{
}

void PasswordInputWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "PasswordInputWrapper::createAndAdd()\t-> ImGui::InputText(Password)\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	char buf[256] = {};
	std::snprintf(buf, sizeof(buf), "%s", m_value.c_str());
	if (ImGui::InputText("##passwordinput", buf, sizeof(buf), ImGuiInputTextFlags_Password))
		m_value = buf;
}

// MultiLineTextCtrlWrapper -----------------------------------------------------------

MultiLineTextCtrlWrapper::MultiLineTextCtrlWrapper(ControlWrapper* parent, std::string& value,
	const Position& pos, const Size& size, long style)
	: m_value(value)
{
}

void MultiLineTextCtrlWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "MultiLineTextCtrlWrapper::createAndAdd()\t-> ImGui::InputTextMultiline()\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	char buf[4096] = {};
	std::snprintf(buf, sizeof(buf), "%s", m_value.c_str());
	if (ImGui::InputTextMultiline("##multilinetextctrl", buf, sizeof(buf)))
		m_value = buf;
}

// ReadonlyTextCtrlWrapper -----------------------------------------------------------

ReadonlyTextCtrlWrapper::ReadonlyTextCtrlWrapper(ControlWrapper* parent, const std::string& value,
	const Position& pos, const Size& size, long style)
	: m_value(value)
{
}

void ReadonlyTextCtrlWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ReadonlyTextCtrlWrapper::createAndAdd()\t-> ImGui::InputText(ReadOnly)\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	char buf[256] = {};
	std::snprintf(buf, sizeof(buf), "%s", m_value.c_str());
	ImGui::InputText("##readonly_textctrl", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
}

// ClickableTextWrapper -----------------------------------------------------------

ClickableTextWrapper::ClickableTextWrapper(ControlWrapper* parent, const std::string& text,
	const Position& pos, const Size& size, long style,
	std::function<void()> onClick)
	: m_text(text.empty() ? "##clickable" : text)
	, m_onClick(std::move(onClick))
{
}

void ClickableTextWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ClickableTextWrapper::createAndAdd()\t-> ImGui::Selectable()\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	if (ImGui::Selectable(m_text.c_str()))
	{
		if (m_onClick)
			m_onClick();
	}
}

// LinkTextWrapper -----------------------------------------------------------

LinkTextWrapper::LinkTextWrapper(ControlWrapper* parent, const std::string& text,
	const Position& pos, const Size& size, long style,
	std::function<void()> onClick)
	: m_text(text.empty() ? "##link" : text)
	, m_onClick(std::move(onClick))
{
}

void LinkTextWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "LinkTextWrapper::createAndAdd()\t-> ImGui::Selectable() [link style]\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.26f, 0.59f, 0.98f, 1.00f));
	bool clicked = ImGui::Selectable(m_text.c_str());
	ImGui::PopStyleColor();
	if (clicked && m_onClick)
		m_onClick();
}

// StaticTextWrapper -----------------------------------------------------------

StaticTextWrapper::StaticTextWrapper(ControlWrapper* parent, const std::string& text,
	const Position& pos, const Size& size, long style)
	: m_text(text)
{
}

void StaticTextWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "StaticTextWrapper::createAndAdd()\t-> ImGui::TextUnformatted()\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	ImGui::TextUnformatted(m_text.c_str());
}

// DatePickerWrapper -----------------------------------------------------------

DatePickerWrapper::DatePickerWrapper(ControlWrapper* parent, Date& value,
	const Position& pos, const Size& size, long style)
	: m_value(value)
{
}

void DatePickerWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "DatePickerWrapper::createAndAdd()\t-> ImGui::InputInt x3 [date]\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	bool changed = false;
	ImGui::PushItemWidth(70);
	changed |= ImGui::InputInt("##dp_year",  &m_value.year,  1, 10);
	ImGui::SameLine(0, 4);
	changed |= ImGui::InputInt("##dp_month", &m_value.month, 1, 0);
	ImGui::SameLine(0, 4);
	changed |= ImGui::InputInt("##dp_day",   &m_value.day,   1, 0);
	ImGui::PopItemWidth();
	if (changed)
	{
		m_value.month = std::clamp(m_value.month, 1, 12);
		m_value.day   = std::clamp(m_value.day,   1, 31);
	}
}

// TimePickerWrapper -----------------------------------------------------------

TimePickerWrapper::TimePickerWrapper(ControlWrapper* parent, Time& value,
	const Position& pos, const Size& size, long style)
	: m_value(value)
{
}

void TimePickerWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "TimePickerWrapper::createAndAdd()\t-> ImGui::InputInt x3 [time]\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	bool changed = false;
	ImGui::PushItemWidth(60);
	changed |= ImGui::InputInt("##tp_hour",   &m_value.hour,   1, 0);
	ImGui::SameLine(0, 4);
	changed |= ImGui::InputInt("##tp_minute", &m_value.minute, 1, 0);
	ImGui::SameLine(0, 4);
	changed |= ImGui::InputInt("##tp_second", &m_value.second, 1, 0);
	ImGui::PopItemWidth();
	if (changed)
	{
		m_value.hour   = std::clamp(m_value.hour,   0, 23);
		m_value.minute = std::clamp(m_value.minute, 0, 59);
		m_value.second = std::clamp(m_value.second, 0, 59);
	}
}

// SliderWrapper -----------------------------------------------------------

template <SliderValue T>
SliderWrapper<T>::SliderWrapper(ControlWrapper* parent, Range<T> range, T& value,
	const Position& pos, const Size& size, long style)
	: m_range(range)
	, m_value(value)
{
}

template <SliderValue T>
void SliderWrapper<T>::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
	ControlWrapper::createAndAdd(parent, layout, flags);
	if constexpr (std::is_same_v<T, int>)
	{
#ifdef USE_LOGGER
		Logger::instance().log(LayoutWrapper::indent() + "SliderWrapper::createAndAdd()\t-> ImGui::SliderInt()\n");
#endif
		ImGui::SliderInt("##slider", &m_value, m_range.min, m_range.max);
	}
	else
	{
#ifdef USE_LOGGER
		Logger::instance().log(LayoutWrapper::indent() + "SliderWrapper::createAndAdd()\t-> ImGui::SliderFloat()\n");
#endif
		ImGui::SliderFloat("##slider", &m_value, m_range.min, m_range.max);
	}
}

template class SliderWrapper<int>;
template class SliderWrapper<float>;

// SpinBoxWrapper -----------------------------------------------------------

template <SpinBoxValue T>
SpinBoxWrapper<T>::SpinBoxWrapper(ControlWrapper* parent, Range<T> range, T& value,
	const Position& pos, const Size& size, long style)
	: m_range(range)
	, m_value(value)
{
}

template <SpinBoxValue T>
void SpinBoxWrapper<T>::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
	ControlWrapper::createAndAdd(parent, layout, flags);
	if constexpr (std::is_same_v<T, int>)
	{
#ifdef USE_LOGGER
		Logger::instance().log(LayoutWrapper::indent() + "SpinBoxWrapper::createAndAdd()\t-> ImGui::InputInt()\n");
#endif
		if (ImGui::InputInt("##spinbox", &m_value, static_cast<int>(m_range.step)))
			m_value = std::clamp(m_value, m_range.min, m_range.max);
	}
	else
	{
#ifdef USE_LOGGER
		Logger::instance().log(LayoutWrapper::indent() + "SpinBoxWrapper::createAndAdd()\t-> ImGui::InputFloat()\n");
#endif
		if (ImGui::InputFloat("##spinbox", &m_value, m_range.step))
			m_value = std::clamp(m_value, m_range.min, m_range.max);
	}
}

template class SpinBoxWrapper<int>;
template class SpinBoxWrapper<float>;

// RadioButtonWrapper -----------------------------------------------------------

template <RadioButtonValue T>
RadioButtonWrapper<T>::RadioButtonWrapper(ControlWrapper* parent, const std::string& label,
	T& value, const Position& pos, const Size& size, long style)
	: m_label(label.empty() ? "##radio" : label)
	, m_value(value)
{
	if constexpr (std::is_same_v<T, int>)
	{
		if (&value != s_lastGroup)
		{
			s_radioButtonId = 0;
			s_lastGroup = &value;
		}
		m_index = s_radioButtonId++;
	}
}

template <RadioButtonValue T>
void RadioButtonWrapper<T>::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "RadioButtonWrapper::createAndAdd()\t-> ImGui::RadioButton()\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	if constexpr (std::is_same_v<T, bool>)
	{
		if (ImGui::RadioButton(m_label.c_str(), m_value))
			m_value = !m_value;
	}
	else
	{
		ImGui::RadioButton(m_label.c_str(), &m_value, m_index);
	}
}

template class RadioButtonWrapper<bool>;
template class RadioButtonWrapper<int>;

// CheckBoxWrapper -----------------------------------------------------------

CheckBoxWrapper::CheckBoxWrapper(ControlWrapper* parent, const std::string& label,
	const Position& pos, const Size& size, long style,
	bool& checked)
	: m_label(label.empty() ? "##checkbox" : label)
	, m_checked(checked)
{
}

void CheckBoxWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "CheckBoxWrapper::createAndAdd()\t-> ImGui::Checkbox()\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	ImGui::Checkbox(m_label.c_str(), &m_checked);
}

// ComboBoxWrapper -----------------------------------------------------------

template <ComboBoxValue T>
ComboBoxWrapper<T>::ComboBoxWrapper(ControlWrapper* parent, std::vector<std::string> choices,
	T& selected, const Position& pos, const Size& size, long style)
	: m_choices(std::move(choices))
	, m_selected(selected)
{
	for (const auto& c : m_choices)
	{
		m_items += c;
		m_items += '\0';
	}

	if constexpr (std::is_same_v<T, int>)
	{
		m_currentItem = selected;
	}
	else
	{
		for (int i = 0; i < static_cast<int>(m_choices.size()); ++i)
		{
			if (m_choices[i] == selected)
			{
				m_currentItem = i;
				break;
			}
		}
	}
}

template <ComboBoxValue T>
void ComboBoxWrapper<T>::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ComboBoxWrapper::createAndAdd()\t-> ImGui::Combo()\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	if constexpr (std::is_same_v<T, int>)
		m_currentItem = m_selected;

	if (ImGui::Combo("##combo", &m_currentItem, m_items.c_str()))
	{
		if constexpr (std::is_same_v<T, int>)
			m_selected = m_currentItem;
		else if (m_currentItem >= 0 && m_currentItem < static_cast<int>(m_choices.size()))
			m_selected = m_choices[m_currentItem];
	}
}

template class ComboBoxWrapper<std::string>;
template class ComboBoxWrapper<int>;
#endif
