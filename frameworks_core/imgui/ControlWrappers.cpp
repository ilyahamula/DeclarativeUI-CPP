#include "frameworks_core/ControlWrappers.hpp"
#include "frameworks_core/LayoutWrapper.hpp"
#include <algorithm>

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#include "imgui.h"
#include "frameworks_core/imgui/ImGuiWidgetIdManager.hpp"

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#elif defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Constructors only collect data and live inline in ControlWrappers.hpp.
// pos, size, style: not directly applicable in ImGui immediate mode.
// Empty labels are mapped to a hidden ("##...") id at render time so the
// stored label keeps the raw value the caller passed.

// ButtonWrapper -----------------------------------------------------------

void ButtonWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ButtonWrapper::createAndAdd()\t-> ImGui::Button()\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	const char* label = m_label.empty() ? "##button" : m_label.c_str();
	ImGui::PushID(WidgetIdManager::nextWidgetId());
	if (ImGui::Button(label))
	{
		if (m_onClick)
			m_onClick();
		else if (m_onClickWithWidget)
			m_onClickWithWidget(m_nativeWidget);
	}
	ImGui::PopID();
}

// TextCtrlWrapper -----------------------------------------------------------

void TextCtrlWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "TextCtrlWrapper::createAndAdd()\t-> ImGui::InputText()\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	if (m_externalRef)
		m_ownedValue = m_externalRef->get();
	char buf[256] = {};
	std::snprintf(buf, sizeof(buf), "%s", m_ownedValue.c_str());
	ImGui::PushID(WidgetIdManager::nextWidgetId());
	if (ImGui::InputText("##textctrl", buf, sizeof(buf)))
	{
		m_ownedValue = buf;
		if (m_externalRef)
			m_externalRef->get() = m_ownedValue;
		if (m_onChange)
			m_onChange(m_ownedValue);
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(m_ownedValue, m_nativeWidget);
	}
	ImGui::PopID();
}

// PasswordInputWrapper -----------------------------------------------------------

void PasswordInputWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "PasswordInputWrapper::createAndAdd()\t-> ImGui::InputText(Password)\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	if (m_externalRef)
		m_ownedValue = m_externalRef->get();
	char buf[256] = {};
	std::snprintf(buf, sizeof(buf), "%s", m_ownedValue.c_str());
	ImGui::PushID(WidgetIdManager::nextWidgetId());
	if (ImGui::InputText("##passwordinput", buf, sizeof(buf), ImGuiInputTextFlags_Password))
	{
		m_ownedValue = buf;
		if (m_externalRef)
			m_externalRef->get() = m_ownedValue;
		if (m_onChange)
			m_onChange(m_ownedValue);
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(m_ownedValue, m_nativeWidget);
	}
	ImGui::PopID();
}

// MultiLineTextCtrlWrapper -----------------------------------------------------------

void MultiLineTextCtrlWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "MultiLineTextCtrlWrapper::createAndAdd()\t-> ImGui::InputTextMultiline()\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	if (m_externalRef)
		m_ownedValue = m_externalRef->get();
	char buf[4096] = {};
	std::snprintf(buf, sizeof(buf), "%s", m_ownedValue.c_str());
	ImGui::PushID(WidgetIdManager::nextWidgetId());
	if (ImGui::InputTextMultiline("##multilinetextctrl", buf, sizeof(buf)))
	{
		m_ownedValue = buf;
		if (m_externalRef)
			m_externalRef->get() = m_ownedValue;
		if (m_onChange)
			m_onChange(m_ownedValue);
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(m_ownedValue, m_nativeWidget);
	}
	ImGui::PopID();
}

// ReadonlyTextCtrlWrapper -----------------------------------------------------------

void ReadonlyTextCtrlWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ReadonlyTextCtrlWrapper::createAndAdd()\t-> ImGui::InputText(ReadOnly)\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	char buf[256] = {};
	std::snprintf(buf, sizeof(buf), "%s", m_value.c_str());
	ImGui::PushID(WidgetIdManager::nextWidgetId());
	ImGui::InputText("##readonly_textctrl", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
	ImGui::PopID();
}

// ClickableTextWrapper -----------------------------------------------------------

void ClickableTextWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ClickableTextWrapper::createAndAdd()\t-> ImGui::Selectable()\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	const char* text = m_text.empty() ? "##clickable" : m_text.c_str();
	ImGui::PushID(WidgetIdManager::nextWidgetId());
	if (ImGui::Selectable(text))
	{
		if (m_onClick)
			m_onClick();
		else if (m_onClickWithWidget)
			m_onClickWithWidget(m_nativeWidget);
	}
	ImGui::PopID();
}

// LinkTextWrapper -----------------------------------------------------------

void LinkTextWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "LinkTextWrapper::createAndAdd()\t-> ImGui::Selectable() [link style]\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	const char* text = m_text.empty() ? "##link" : m_text.c_str();
	ImGui::PushID(WidgetIdManager::nextWidgetId());
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.26f, 0.59f, 0.98f, 1.00f));
	bool clicked = ImGui::Selectable(text);
	ImGui::PopStyleColor();
	ImGui::PopID();
	if (clicked)
	{
		if (m_onClick)
			m_onClick();
		else if (m_onClickWithWidget)
			m_onClickWithWidget(m_nativeWidget);
	}
}

// StaticTextWrapper -----------------------------------------------------------

void StaticTextWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "StaticTextWrapper::createAndAdd()\t-> ImGui::TextUnformatted()\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	ImGui::TextUnformatted(m_text.c_str());
}

// DatePickerWrapper -----------------------------------------------------------

void DatePickerWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "DatePickerWrapper::createAndAdd()\t-> ImGui::InputInt x3 [date]\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	if (m_externalRef)
		m_ownedValue = m_externalRef->get();

	bool changed = false;
	ImGui::PushID(WidgetIdManager::nextWidgetId());
	{
		const ImGuiStyle& style = ImGui::GetStyle();
		const float btnW = (ImGui::GetFrameHeight() + style.ItemInnerSpacing.x) * 2.0f;
		const float padW = style.FramePadding.x * 2.0f;
		ImGui::SetNextItemWidth(ImGui::CalcTextSize("9999").x + padW + btnW);
		changed |= ImGui::InputInt("##dp_year",  &m_ownedValue.year,  1, 10);
		ImGui::SameLine(0, 4);
		ImGui::SetNextItemWidth(ImGui::CalcTextSize("12").x + padW + btnW);
		changed |= ImGui::InputInt("##dp_month", &m_ownedValue.month, 1, 0);
		ImGui::SameLine(0, 4);
		ImGui::SetNextItemWidth(ImGui::CalcTextSize("31").x + padW + btnW);
		changed |= ImGui::InputInt("##dp_day",   &m_ownedValue.day,   1, 0);
	}
	ImGui::PopID();
	if (changed)
	{
		m_ownedValue.month = std::clamp(m_ownedValue.month, 1, 12);
		m_ownedValue.day   = std::clamp(m_ownedValue.day,   1, 31);
		if (m_externalRef)
			m_externalRef->get() = m_ownedValue;
		if (m_onChange)
			m_onChange(m_ownedValue);
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(m_ownedValue, m_nativeWidget);
	}
}

// TimePickerWrapper -----------------------------------------------------------

void TimePickerWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "TimePickerWrapper::createAndAdd()\t-> ImGui::InputInt x3 [time]\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	if (m_externalRef)
		m_ownedValue = m_externalRef->get();

	bool changed = false;
	ImGui::PushID(WidgetIdManager::nextWidgetId());
	{
		const ImGuiStyle& style = ImGui::GetStyle();
		const float btnW  = (ImGui::GetFrameHeight() + style.ItemInnerSpacing.x) * 2.0f;
		const float padW  = style.FramePadding.x * 2.0f;
		const float twoDigitW = ImGui::CalcTextSize("59").x + padW + btnW;
		ImGui::SetNextItemWidth(ImGui::CalcTextSize("23").x + padW + btnW);
		changed |= ImGui::InputInt("##tp_hour",   &m_ownedValue.hour,   1, 0);
		ImGui::SameLine(0, 4);
		ImGui::SetNextItemWidth(twoDigitW);
		changed |= ImGui::InputInt("##tp_minute", &m_ownedValue.minute, 1, 0);
		ImGui::SameLine(0, 4);
		ImGui::SetNextItemWidth(twoDigitW);
		changed |= ImGui::InputInt("##tp_second", &m_ownedValue.second, 1, 0);
	}
	ImGui::PopID();
	if (changed)
	{
		m_ownedValue.hour   = std::clamp(m_ownedValue.hour,   0, 23);
		m_ownedValue.minute = std::clamp(m_ownedValue.minute, 0, 59);
		m_ownedValue.second = std::clamp(m_ownedValue.second, 0, 59);
		if (m_externalRef)
			m_externalRef->get() = m_ownedValue;
		if (m_onChange)
			m_onChange(m_ownedValue);
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(m_ownedValue, m_nativeWidget);
	}
}

// SliderWrapper -----------------------------------------------------------

template <SliderValue T>
void SliderWrapper<T>::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
	ControlWrapper::createAndAdd(parent, layout, flags);
	if (m_externalRef)
		m_ownedValue = m_externalRef->get();

	ImGui::PushID(WidgetIdManager::nextWidgetId());
	bool changed = false;
	if constexpr (std::is_same_v<T, int>)
	{
#ifdef USE_LOGGER
		Logger::instance().log(LayoutWrapper::indent() + "SliderWrapper::createAndAdd()\t-> ImGui::SliderInt()\n");
#endif
		changed = ImGui::SliderInt("##slider", &m_ownedValue, m_range.min, m_range.max);
	}
	else
	{
#ifdef USE_LOGGER
		Logger::instance().log(LayoutWrapper::indent() + "SliderWrapper::createAndAdd()\t-> ImGui::SliderFloat()\n");
#endif
		changed = ImGui::SliderFloat("##slider", &m_ownedValue, m_range.min, m_range.max);
	}
	ImGui::PopID();

	if (changed)
	{
		if (m_externalRef)
			m_externalRef->get() = m_ownedValue;
		if (m_onChange)
			m_onChange(m_ownedValue);
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(m_ownedValue, m_nativeWidget);
	}
}

template class SliderWrapper<int>;
template class SliderWrapper<float>;

// SpinBoxWrapper -----------------------------------------------------------

template <SpinBoxValue T>
void SpinBoxWrapper<T>::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
	ControlWrapper::createAndAdd(parent, layout, flags);
	if (m_externalRef)
		m_ownedValue = m_externalRef->get();

	if (!flags.expand() && flags.proportion() == 0)
	{
		char bufMin[32], bufMax[32];
		if constexpr (std::is_same_v<T, int>)
		{
			snprintf(bufMin, sizeof(bufMin), "%d", m_range.min);
			snprintf(bufMax, sizeof(bufMax), "%d", m_range.max);
		}
		else
		{
			snprintf(bufMin, sizeof(bufMin), "%.3f", m_range.min);
			snprintf(bufMax, sizeof(bufMax), "%.3f", m_range.max);
		}
		float textW = std::max(ImGui::CalcTextSize(bufMin).x, ImGui::CalcTextSize(bufMax).x);
		const ImGuiStyle& style = ImGui::GetStyle();
		float stepBtnW = (m_range.step != 0) ? (ImGui::GetFrameHeight() + style.ItemInnerSpacing.x) * 2.0f : 0.0f;
		float minWidth = textW + style.FramePadding.x * 2.0f + stepBtnW;
		ImGui::SetNextItemWidth(std::max(minWidth, 60.0f));
	}

	ImGui::PushID(WidgetIdManager::nextWidgetId());
	bool changed = false;
	if constexpr (std::is_same_v<T, int>)
	{
#ifdef USE_LOGGER
		Logger::instance().log(LayoutWrapper::indent() + "SpinBoxWrapper::createAndAdd()\t-> ImGui::InputInt()\n");
#endif
		changed = ImGui::InputInt("##spinbox", &m_ownedValue, static_cast<int>(m_range.step));
		if (changed)
			m_ownedValue = std::clamp(m_ownedValue, m_range.min, m_range.max);
	}
	else
	{
#ifdef USE_LOGGER
		Logger::instance().log(LayoutWrapper::indent() + "SpinBoxWrapper::createAndAdd()\t-> ImGui::InputFloat()\n");
#endif
		changed = ImGui::InputFloat("##spinbox", &m_ownedValue, m_range.step);
		if (changed)
			m_ownedValue = std::clamp(m_ownedValue, m_range.min, m_range.max);
	}
	ImGui::PopID();

	if (changed)
	{
		if (m_externalRef)
			m_externalRef->get() = m_ownedValue;
		if (m_onChange)
			m_onChange(m_ownedValue);
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(m_ownedValue, m_nativeWidget);
	}
}

template class SpinBoxWrapper<int>;
template class SpinBoxWrapper<float>;

// RadioButtonWrapper -----------------------------------------------------------

template <RadioButtonValue T>
void RadioButtonWrapper<T>::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "RadioButtonWrapper::createAndAdd()\t-> ImGui::RadioButton()\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	if (m_externalRef)
		m_ownedValue = m_externalRef->get();
	const char* label = m_label.empty() ? "##radio" : m_label.c_str();
	ImGui::PushID(WidgetIdManager::nextWidgetId());
	if constexpr (std::is_same_v<T, bool>)
	{
		if (ImGui::RadioButton(label, m_ownedValue))
		{
			m_ownedValue = !m_ownedValue;
			if (m_externalRef)
				m_externalRef->get() = m_ownedValue;
			if (m_onChange)
				m_onChange(m_ownedValue);
			else if (m_onChangeWithWidget)
				m_onChangeWithWidget(m_ownedValue, m_nativeWidget);
		}
	}
	else
	{
		if (ImGui::RadioButton(label, &m_ownedValue, m_index))
		{
			if (m_externalRef)
				m_externalRef->get() = m_ownedValue;
			if (m_onChange)
				m_onChange(m_ownedValue);
			else if (m_onChangeWithWidget)
				m_onChangeWithWidget(m_ownedValue, m_nativeWidget);
		}
	}
	ImGui::PopID();
}

template class RadioButtonWrapper<bool>;
template class RadioButtonWrapper<int>;

// CheckBoxWrapper -----------------------------------------------------------

void CheckBoxWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "CheckBoxWrapper::createAndAdd()\t-> ImGui::Checkbox()\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	if (m_externalRef)
		m_ownedValue = m_externalRef->get();
	const char* label = m_label.empty() ? "##checkbox" : m_label.c_str();
	ImGui::PushID(WidgetIdManager::nextWidgetId());
	if (ImGui::Checkbox(label, &m_ownedValue))
	{
		if (m_externalRef)
			m_externalRef->get() = m_ownedValue;
		if (m_onChange)
			m_onChange(m_ownedValue);
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(m_ownedValue, m_nativeWidget);
	}
	ImGui::PopID();
}

// ToggleButtonWrapper -----------------------------------------------------------

void ToggleButtonWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ToggleButtonWrapper::createAndAdd()\t-> ImGui::Button() [toggle]\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	if (m_externalRef)
		m_ownedValue = m_externalRef->get();
	const char* label = m_label.empty() ? "##toggle" : m_label.c_str();
	ImGui::PushID(WidgetIdManager::nextWidgetId());
	const bool wasToggled = m_ownedValue;
	if (wasToggled)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
	}
	if (ImGui::Button(label))
	{
		m_ownedValue = !m_ownedValue;
		if (m_externalRef)
			m_externalRef->get() = m_ownedValue;
		if (m_onChange)
			m_onChange(m_ownedValue);
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(m_ownedValue, m_nativeWidget);
	}
	if (wasToggled)
		ImGui::PopStyleColor(2);
	ImGui::PopID();
}

// ImageWrapper -----------------------------------------------------------

void ImageWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ImageWrapper::createAndAdd()\t-> ImGui::Image()\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);

	if (m_textureId == nullptr && !m_filePath.empty())
	{
		stbi_set_flip_vertically_on_load(0);
		unsigned char* data = stbi_load(m_filePath.c_str(), &m_imgWidth, &m_imgHeight, nullptr, 4);
		if (data)
		{
			GLuint texId = 0;
			glGenTextures(1, &texId);
			glBindTexture(GL_TEXTURE_2D, texId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_imgWidth, m_imgHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
			m_textureId = reinterpret_cast<void*>(static_cast<uintptr_t>(texId));
		}
	}

	ImGui::PushID(WidgetIdManager::nextWidgetId());
	if (m_textureId != nullptr)
	{
		const float w = (m_displayWidth  > 0) ? static_cast<float>(m_displayWidth)  : static_cast<float>(m_imgWidth);
		const float h = (m_displayHeight > 0) ? static_cast<float>(m_displayHeight) : static_cast<float>(m_imgHeight);
		ImGui::Image(static_cast<ImTextureID>(reinterpret_cast<uintptr_t>(m_textureId)), ImVec2(w, h));
		if (ImGui::IsItemHovered())
		{
			if (m_onHover)
				m_onHover();
			else if (m_onHoverWithWidget)
				m_onHoverWithWidget(m_nativeWidget);
		}
		if (ImGui::IsItemClicked())
		{
			if (m_onClick)
				m_onClick();
			else if (m_onClickWithWidget)
				m_onClickWithWidget(m_nativeWidget);
		}
	}
	else
	{
		ImGui::TextUnformatted("[Image: failed to load]");
	}
	ImGui::PopID();
}

// ComboBoxWrapper -----------------------------------------------------------

template <ComboBoxValue T>
void ComboBoxWrapper<T>::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ComboBoxWrapper::createAndAdd()\t-> ImGui::Combo()\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	if (m_externalRef)
	{
		if constexpr (std::is_same_v<T, int>)
		{
			m_currentItem = m_externalRef->get();
		}
		else
		{
			for (int i = 0; i < static_cast<int>(m_choices.size()); ++i)
			{
				if (m_choices[i] == m_externalRef->get())
				{
					m_currentItem = i;
					break;
				}
			}
		}
	}

	ImGui::PushID(WidgetIdManager::nextWidgetId());
	if (ImGui::Combo("##combo", &m_currentItem, m_items.c_str()))
	{
		if constexpr (std::is_same_v<T, int>)
			m_ownedSelected = m_currentItem;
		else if (m_currentItem >= 0 && m_currentItem < static_cast<int>(m_choices.size()))
			m_ownedSelected = m_choices[m_currentItem];

		if (m_externalRef)
			m_externalRef->get() = m_ownedSelected;
		if (m_onChange)
			m_onChange(m_ownedSelected);
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(m_ownedSelected, m_nativeWidget);
	}
	ImGui::PopID();
}

template class ComboBoxWrapper<std::string>;
template class ComboBoxWrapper<int>;

// ColorPickerWrapper -----------------------------------------------------------

void ColorPickerWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ColorPickerWrapper::createAndAdd()\t-> ImGui::ColorEdit4()\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	if (m_externalRef)
		m_ownedValue = m_externalRef->get();

	float col[4] = { m_ownedValue.r, m_ownedValue.g, m_ownedValue.b, m_ownedValue.a };
	ImGui::PushID(WidgetIdManager::nextWidgetId());
	if (ImGui::ColorEdit4("##colorpicker", col))
	{
		m_ownedValue = Color{ col[0], col[1], col[2], col[3] };
		if (m_externalRef)
			m_externalRef->get() = m_ownedValue;
		if (m_onChange)
			m_onChange(m_ownedValue);
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(m_ownedValue, m_nativeWidget);
	}
	ImGui::PopID();
}

// SeparatorWrapper -----------------------------------------------------------

void SeparatorWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "SeparatorWrapper::createAndAdd()\t-> ImGui::Separator()\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	ImGui::Separator();
}

// ProgressBarWrapper -----------------------------------------------------------

void ProgressBarWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "ProgressBarWrapper::createAndAdd()\t-> ImGui::ProgressBar()\n");
#endif
	ControlWrapper::createAndAdd(parent, layout, flags);
	float value = m_externalRef ? m_externalRef->get() : m_ownedValue;
	float width = flags.expand() ? -FLT_MIN : 0.0f;
	ImGui::PushID(WidgetIdManager::nextWidgetId());
	ImGui::ProgressBar(std::clamp(value, 0.0f, 1.0f), ImVec2(width, 0.0f));
	ImGui::PopID();
}
