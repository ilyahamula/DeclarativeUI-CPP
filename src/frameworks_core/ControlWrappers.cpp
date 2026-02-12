#include "frameworks_core/ControlWrappers.hpp"

#ifdef USE_WX
#include <wx/wx.h>

// ButtonWrapper -----------------------------------------------------------

ButtonWrapper::ButtonWrapper(ControlWrapper* parent, const std::string& label,
	const Position& pos, const Size& size, long style,
	std::function<void()> onClick)
{
	auto* btn = new wxButton(parent->nativeHandle(), wxID_ANY, label,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	if (onClick)
		btn->Bind(wxEVT_BUTTON, [cb = std::move(onClick)](wxCommandEvent&) { cb(); });
	m_nativeWidget = btn;
}

// TextCtrlWrapper -----------------------------------------------------------

TextCtrlWrapper::TextCtrlWrapper(ControlWrapper* parent, const std::string& value,
	const Position& pos, const Size& size, long style)
{
	m_nativeWidget = new wxTextCtrl(parent->nativeHandle(), wxID_ANY, value,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
}

// StaticTextWrapper -----------------------------------------------------------

StaticTextWrapper::StaticTextWrapper(ControlWrapper* parent, const std::string& text,
	const Position& pos, const Size& size, long style)
{
	m_nativeWidget = new wxStaticText(parent->nativeHandle(), wxID_ANY, text,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
}

// SliderWrapper -----------------------------------------------------------

SliderWrapper::SliderWrapper(ControlWrapper* parent, Range range,
	const Position& pos, const Size& size, long style)
{
	m_nativeWidget = new wxSlider(parent->nativeHandle(), wxID_ANY,
		range.value.value_or(range.min), range.min, range.max,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
}

// RadioButtonWrapper -----------------------------------------------------------

RadioButtonWrapper::RadioButtonWrapper(ControlWrapper* parent, const std::string& label,
	const Position& pos, const Size& size, long style)
{
	m_nativeWidget = new wxRadioButton(parent->nativeHandle(), wxID_ANY, label,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
}

// CheckBoxWrapper -----------------------------------------------------------

CheckBoxWrapper::CheckBoxWrapper(ControlWrapper* parent, const std::string& label,
	const Position& pos, const Size& size, long style,
	bool checked)
{
	auto* cb = new wxCheckBox(parent->nativeHandle(), wxID_ANY, label,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	cb->SetValue(checked);
	m_nativeWidget = cb;
}

// ComboBoxWrapper -----------------------------------------------------------

ComboBoxWrapper::ComboBoxWrapper(ControlWrapper* parent, std::vector<std::string> choices,
	const std::string& selected, const Position& pos, const Size& size, long style)
{
	wxArrayString items;
	for (const auto& c : choices)
		items.Add(c);
	m_nativeWidget = new wxComboBox(parent->nativeHandle(), wxID_ANY, selected,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), items, style);
}
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif
