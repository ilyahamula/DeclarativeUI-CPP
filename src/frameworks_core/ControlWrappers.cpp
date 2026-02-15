#include "frameworks_core/ControlWrappers.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#ifdef USE_WX
#include <wx/wx.h>

// ButtonWrapper -----------------------------------------------------------

ButtonWrapper::ButtonWrapper(ControlWrapper* parent, const std::string& label,
	const Position& pos, const Size& size, long style,
	std::function<void()> onClick)
{
#ifdef USE_LOGGER
	Logger::instance().log("ButtonWrapper::ButtonWrapper() -> new wxButton()\n");
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
	Logger::instance().log("TextCtrlWrapper::TextCtrlWrapper() -> new wxTextCtrl()\n");
#endif
	m_nativeWidget = new wxTextCtrl(parent->nativeHandle(), wxID_ANY, value,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
}

// StaticTextWrapper -----------------------------------------------------------

StaticTextWrapper::StaticTextWrapper(ControlWrapper* parent, const std::string& text,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log("StaticTextWrapper::StaticTextWrapper() -> new wxStaticText()\n");
#endif
	m_nativeWidget = new wxStaticText(parent->nativeHandle(), wxID_ANY, text,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
}

// SliderWrapper -----------------------------------------------------------

template <SliderValue T>
SliderWrapper<T>::SliderWrapper(ControlWrapper* parent, Range<T> range, T* ptrValue,
	const Position& pos, const Size& size, long style)
	: m_range(range)
	, m_ptrValue(ptrValue)
{
	assert(ptrValue);
#ifdef USE_LOGGER
	Logger::instance().log("SliderWrapper::SliderWrapper() -> new wxSlider()\n");
#endif
	if constexpr (std::is_floating_point_v<T>)
	{
		int iMin = static_cast<int>(range.min / range.step);
		int iMax = static_cast<int>(range.max / range.step);
		int iVal = static_cast<int>(*ptrValue / range.step);
		m_nativeWidget = new wxSlider(parent->nativeHandle(), wxID_ANY,
			iVal, iMin, iMax,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	}
	else
	{
		m_nativeWidget = new wxSlider(parent->nativeHandle(), wxID_ANY,
			*ptrValue, range.min, range.max,
			wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	}
}

template class SliderWrapper<int>;
template class SliderWrapper<float>;

// RadioButtonWrapper -----------------------------------------------------------

RadioButtonWrapper::RadioButtonWrapper(ControlWrapper* parent, const std::string& label,
	const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log("RadioButtonWrapper::RadioButtonWrapper() -> new wxRadioButton()\n");
#endif
	m_nativeWidget = new wxRadioButton(parent->nativeHandle(), wxID_ANY, label,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
}

// CheckBoxWrapper -----------------------------------------------------------

CheckBoxWrapper::CheckBoxWrapper(ControlWrapper* parent, const std::string& label,
	const Position& pos, const Size& size, long style,
	bool checked)
{
#ifdef USE_LOGGER
	Logger::instance().log("CheckBoxWrapper::CheckBoxWrapper() -> new wxCheckBox()\n");
#endif
	auto* cb = new wxCheckBox(parent->nativeHandle(), wxID_ANY, label,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), style);
	cb->SetValue(checked);
	m_nativeWidget = cb;
}

// ComboBoxWrapper -----------------------------------------------------------

ComboBoxWrapper::ComboBoxWrapper(ControlWrapper* parent, std::vector<std::string> choices,
	const std::string& selected, const Position& pos, const Size& size, long style)
{
#ifdef USE_LOGGER
	Logger::instance().log("ComboBoxWrapper::ComboBoxWrapper() -> new wxComboBox()\n");
#endif
	wxArrayString items;
	for (const auto& c : choices)
		items.Add(c);
	m_nativeWidget = new wxComboBox(parent->nativeHandle(), wxID_ANY, selected,
		wxPoint(pos.x, pos.y), wxSize(size.width, size.height), items, style);
}
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#include "imgui.h"

// pos, size, style: not directly applicable in ImGui immediate mode

// ButtonWrapper -----------------------------------------------------------

ButtonWrapper::ButtonWrapper(ControlWrapper* parent, const std::string& label,
	const Position& pos, const Size& size, long style,
	std::function<void()> onClick)
	: m_label(label)
	, m_onClick(std::move(onClick))
{
}

void ButtonWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log("ButtonWrapper::createAndAdd() -> ImGui::Button()\n");
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
	Logger::instance().log("TextCtrlWrapper::createAndAdd() -> ImGui::InputText()\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);

	char buf[256] = {};
	std::snprintf(buf, sizeof(buf), "%s", m_value.c_str());
	if (ImGui::InputText("##textctrl", buf, sizeof(buf)))
		m_value = buf;
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
	Logger::instance().log("StaticTextWrapper::createAndAdd() -> ImGui::TextUnformatted()\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	ImGui::TextUnformatted(m_text.c_str());
}

// SliderWrapper -----------------------------------------------------------

template <SliderValue T>
SliderWrapper<T>::SliderWrapper(ControlWrapper* parent, Range<T> range, T* ptrValue,
	const Position& pos, const Size& size, long style)
	: m_range(range)
	, m_ptrValue(ptrValue)
{
}

template <SliderValue T>
void SliderWrapper<T>::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
	ControlWrapper::createAndAdd(parent, layout, flags);
	if constexpr (std::is_same_v<T, int>)
	{
#ifdef USE_LOGGER
		Logger::instance().log("SliderWrapper::createAndAdd() -> ImGui::SliderInt()\n");
#endif
		ImGui::SliderInt("##slider", m_ptrValue, m_range.min, m_range.max);
	}
	else
	{
#ifdef USE_LOGGER
		Logger::instance().log("SliderWrapper::createAndAdd() -> ImGui::SliderFloat()\n");
#endif
		ImGui::SliderFloat("##slider", m_ptrValue, m_range.min, m_range.max);
	}
}

template class SliderWrapper<int>;
template class SliderWrapper<float>;

// RadioButtonWrapper -----------------------------------------------------------

RadioButtonWrapper::RadioButtonWrapper(ControlWrapper* parent, const std::string& label,
	const Position& pos, const Size& size, long style)
	: m_label(label)
{
}

void RadioButtonWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log("RadioButtonWrapper::createAndAdd() -> ImGui::RadioButton()\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	if (ImGui::RadioButton(m_label.c_str(), m_active))
		m_active = !m_active;
}

// CheckBoxWrapper -----------------------------------------------------------

CheckBoxWrapper::CheckBoxWrapper(ControlWrapper* parent, const std::string& label,
	const Position& pos, const Size& size, long style,
	bool checked)
	: m_label(label)
{
	m_checked = checked;
}

void CheckBoxWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log("CheckBoxWrapper::createAndAdd() -> ImGui::Checkbox()\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	ImGui::Checkbox(m_label.c_str(), &m_checked);
}

// ComboBoxWrapper -----------------------------------------------------------

ComboBoxWrapper::ComboBoxWrapper(ControlWrapper* parent, std::vector<std::string> choices,
	const std::string& selected, const Position& pos, const Size& size, long style)
{
	for (int i = 0; i < static_cast<int>(choices.size()); ++i)
	{
		if (choices[i] == selected)
		{
			m_currentItem = i;
			break;
		}
	}
	// Build null-separated items string for ImGui::Combo
	for (const auto& c : choices)
	{
		m_items += c;
		m_items += '\0';
	}
}

void ComboBoxWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log("ComboBoxWrapper::createAndAdd() -> ImGui::Combo()\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	ImGui::Combo("##combo", &m_currentItem, m_items.c_str());
}
#endif
