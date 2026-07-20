#include "frameworks_core/ControlWrappers.hpp"
#include <algorithm>
#include <cmath>

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
//
// Each wrapper implements the engine path (measureIntrinsic + render) per the
// measurement contract table in docs/specs/custom_layout_system/architecture.md.
// NOTE on editable fields: the declarative tree is rebuilt every frame on
// ImGui, so "measure initial content" would re-measure the live value each
// frame and grow the field while typing. ImGui editable fields therefore
// measure content-independent defaults (the N-char floor); content-based
// initial sampling applies to the retained backends (Phase 3).

namespace
{

// "natural size" frame for the legacy path: render() applies no explicit size
const Rect kNaturalFrame { -1, -1, -1, -1 };

bool sized(const Rect& frame)
{
	return frame.width > 0;
}

int ceilInt(float v)
{
	return (int)std::ceil(v);
}

int frameHeight()
{
	return ceilInt(ImGui::GetFrameHeight());
}

// text-only items (StaticText, Selectable)
Size textItemSize(const std::string& text)
{
	const ImVec2 t = ImGui::CalcTextSize(text.c_str());
	return Size { ceilInt(t.x), ceilInt(t.y) };
}

// Selectable-based items (ClickableText, LinkText): the item rect is always
// size_arg + ItemSpacing (the interactive area extends into the spacing), so
// measure reports text + spacing and render passes frame - spacing back.
Size selectableSize(const std::string& text)
{
	const ImGuiStyle& style = ImGui::GetStyle();
	const ImVec2 t = ImGui::CalcTextSize(text.c_str());
	return Size { ceilInt(t.x + style.ItemSpacing.x), ceilInt(t.y + style.ItemSpacing.y) };
}

ImVec2 selectableSizeArg(const Rect& frame)
{
	const ImGuiStyle& style = ImGui::GetStyle();
	return ImVec2(std::max(1.0f, (float)frame.width - style.ItemSpacing.x),
		std::max(1.0f, (float)frame.height - style.ItemSpacing.y));
}

// framed items (Button): label + FramePadding
Size framedTextSize(const std::string& text)
{
	const ImGuiStyle& style = ImGui::GetStyle();
	const ImVec2 t = ImGui::CalcTextSize(text.c_str());
	return Size { ceilInt(t.x + style.FramePadding.x * 2.0f), frameHeight() };
}

// N-average-character floor for editable fields (architecture.md, N = 10)
int editableFloorWidth()
{
	const ImGuiStyle& style = ImGui::GetStyle();
	return ceilInt(ImGui::CalcTextSize("0").x * 10.0f + style.FramePadding.x * 2.0f);
}

// glyph + label (CheckBox, RadioButton)
Size glyphLabelSize(const std::string& label)
{
	const ImGuiStyle& style = ImGui::GetStyle();
	const float glyph = ImGui::GetFrameHeight();
	const float text = label.empty() ? 0.0f
		: style.ItemInnerSpacing.x + ImGui::CalcTextSize(label.c_str()).x;
	return Size { ceilInt(glyph + text), frameHeight() };
}

// default width for value controls with no content-derived width
// (Slider, ProgressBar, ColorPicker) — an engine-path stand-in for
// ImGui's window-relative CalcItemWidth
constexpr int kDefaultControlWidth = 200;

} // unnamed namespace

// ButtonWrapper -----------------------------------------------------------

Size ButtonWrapper::measureIntrinsic(const Constraints&)
{
	return framedTextSize(m_label);
}

void ButtonWrapper::render(const Rect& frame)
{
	const char* label = m_label.empty() ? "##button" : m_label.c_str();
	ImGui::PushID(WidgetIdManager::nextWidgetId());
	const bool clicked = sized(frame)
		? ImGui::Button(label, ImVec2((float)frame.width, (float)frame.height))
		: ImGui::Button(label);
	ImGui::PopID();
	if (clicked)
	{
		if (m_onClick)
			m_onClick();
		else if (m_onClickWithWidget)
			m_onClickWithWidget(m_nativeWidget);
	}
}

// TextCtrlWrapper -----------------------------------------------------------

Size TextCtrlWrapper::measureIntrinsic(const Constraints&)
{
	return Size { editableFloorWidth(), frameHeight() };
}

void TextCtrlWrapper::render(const Rect& frame)
{
	if (m_externalRef)
		m_ownedValue = m_externalRef->get();
	char buf[256] = {};
	std::snprintf(buf, sizeof(buf), "%s", m_ownedValue.c_str());
	if (sized(frame))
		ImGui::SetNextItemWidth((float)frame.width);
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

Size PasswordInputWrapper::measureIntrinsic(const Constraints&)
{
	return Size { editableFloorWidth(), frameHeight() };
}

void PasswordInputWrapper::render(const Rect& frame)
{
	if (m_externalRef)
		m_ownedValue = m_externalRef->get();
	char buf[256] = {};
	std::snprintf(buf, sizeof(buf), "%s", m_ownedValue.c_str());
	if (sized(frame))
		ImGui::SetNextItemWidth((float)frame.width);
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

Size MultiLineTextCtrlWrapper::measureIntrinsic(const Constraints&)
{
	// ImGui's default multiline height (8 text lines + frame padding)
	const ImGuiStyle& style = ImGui::GetStyle();
	const int height = ceilInt(ImGui::GetTextLineHeight() * 8.0f + style.FramePadding.y * 2.0f);
	return Size { editableFloorWidth(), height };
}

void MultiLineTextCtrlWrapper::render(const Rect& frame)
{
	if (m_externalRef)
		m_ownedValue = m_externalRef->get();
	char buf[4096] = {};
	std::snprintf(buf, sizeof(buf), "%s", m_ownedValue.c_str());
	const ImVec2 size = sized(frame)
		? ImVec2((float)frame.width, (float)frame.height)
		: ImVec2(0, 0);
	ImGui::PushID(WidgetIdManager::nextWidgetId());
	if (ImGui::InputTextMultiline("##multilinetextctrl", buf, sizeof(buf), size))
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

Size ReadonlyTextCtrlWrapper::measureIntrinsic(const Constraints&)
{
	// readonly content is static, so it may size to its text (with floor)
	const ImGuiStyle& style = ImGui::GetStyle();
	const int contentW = ceilInt(ImGui::CalcTextSize(m_value.c_str()).x + style.FramePadding.x * 2.0f);
	return Size { std::max(contentW, editableFloorWidth()), frameHeight() };
}

void ReadonlyTextCtrlWrapper::render(const Rect& frame)
{
	char buf[256] = {};
	std::snprintf(buf, sizeof(buf), "%s", m_value.c_str());
	if (sized(frame))
		ImGui::SetNextItemWidth((float)frame.width);
	ImGui::PushID(WidgetIdManager::nextWidgetId());
	ImGui::InputText("##readonly_textctrl", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
	ImGui::PopID();
}

// ClickableTextWrapper -----------------------------------------------------------

Size ClickableTextWrapper::measureIntrinsic(const Constraints&)
{
	return selectableSize(m_text);
}

void ClickableTextWrapper::render(const Rect& frame)
{
	const char* text = m_text.empty() ? "##clickable" : m_text.c_str();
	ImGui::PushID(WidgetIdManager::nextWidgetId());
	const bool clicked = sized(frame)
		? ImGui::Selectable(text, false, 0, selectableSizeArg(frame))
		: ImGui::Selectable(text);
	ImGui::PopID();
	if (clicked)
	{
		if (m_onClick)
			m_onClick();
		else if (m_onClickWithWidget)
			m_onClickWithWidget(m_nativeWidget);
	}
}

// LinkTextWrapper -----------------------------------------------------------

Size LinkTextWrapper::measureIntrinsic(const Constraints&)
{
	return selectableSize(m_text);
}

void LinkTextWrapper::render(const Rect& frame)
{
	const char* text = m_text.empty() ? "##link" : m_text.c_str();
	ImGui::PushID(WidgetIdManager::nextWidgetId());
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.26f, 0.59f, 0.98f, 1.00f));
	const bool clicked = sized(frame)
		? ImGui::Selectable(text, false, 0, selectableSizeArg(frame))
		: ImGui::Selectable(text);
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

Size StaticTextWrapper::measureIntrinsic(const Constraints& c)
{
	// wraps at the offered width (auto-fit cap / fixed dialog width)
	const ImVec2 t = ImGui::CalcTextSize(m_text.c_str(), nullptr, false, (float)c.maxWidth);
	return Size { ceilInt(t.x), ceilInt(t.y) };
}

void StaticTextWrapper::render(const Rect& frame)
{
	const bool wrap = sized(frame)
		&& ImGui::CalcTextSize(m_text.c_str()).x > (float)frame.width;
	if (wrap)
		ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + (float)frame.width);
	ImGui::TextUnformatted(m_text.c_str());
	if (wrap)
		ImGui::PopTextWrapPos();
}

// DatePickerWrapper -----------------------------------------------------------

Size DatePickerWrapper::measureIntrinsic(const Constraints&)
{
	const ImGuiStyle& style = ImGui::GetStyle();
	const float btnW = (ImGui::GetFrameHeight() + style.ItemInnerSpacing.x) * 2.0f;
	const float padW = style.FramePadding.x * 2.0f;
	const float w = (ImGui::CalcTextSize("9999").x + padW + btnW)
		+ (ImGui::CalcTextSize("12").x + padW + btnW)
		+ (ImGui::CalcTextSize("31").x + padW + btnW)
		+ 2.0f * 4.0f; // SameLine(0, 4) gaps
	return Size { ceilInt(w), frameHeight() };
}

void DatePickerWrapper::render(const Rect&)
{
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

Size TimePickerWrapper::measureIntrinsic(const Constraints&)
{
	const ImGuiStyle& style = ImGui::GetStyle();
	const float btnW = (ImGui::GetFrameHeight() + style.ItemInnerSpacing.x) * 2.0f;
	const float padW = style.FramePadding.x * 2.0f;
	const float twoDigitW = ImGui::CalcTextSize("59").x + padW + btnW;
	const float w = (ImGui::CalcTextSize("23").x + padW + btnW)
		+ twoDigitW * 2.0f
		+ 2.0f * 4.0f; // SameLine(0, 4) gaps
	return Size { ceilInt(w), frameHeight() };
}

void TimePickerWrapper::render(const Rect&)
{
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
Size SliderWrapper<T>::measureIntrinsic(const Constraints&)
{
	return Size { kDefaultControlWidth, frameHeight() };
}

template <SliderValue T>
void SliderWrapper<T>::render(const Rect& frame)
{
	if (m_externalRef)
		m_ownedValue = m_externalRef->get();

	if (sized(frame))
		ImGui::SetNextItemWidth((float)frame.width);
	ImGui::PushID(WidgetIdManager::nextWidgetId());
	bool changed = false;
	if constexpr (std::is_same_v<T, int>)
		changed = ImGui::SliderInt("##slider", &m_ownedValue, m_range.min, m_range.max);
	else
		changed = ImGui::SliderFloat("##slider", &m_ownedValue, m_range.min, m_range.max);
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
Size SpinBoxWrapper<T>::measureIntrinsic(const Constraints&)
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
	const float textW = std::max(ImGui::CalcTextSize(bufMin).x, ImGui::CalcTextSize(bufMax).x);
	const ImGuiStyle& style = ImGui::GetStyle();
	const float stepBtnW = (m_range.step != 0) ? (ImGui::GetFrameHeight() + style.ItemInnerSpacing.x) * 2.0f : 0.0f;
	const float minWidth = textW + style.FramePadding.x * 2.0f + stepBtnW;
	return Size { ceilInt(std::max(minWidth, 60.0f)), frameHeight() };
}

template <SpinBoxValue T>
void SpinBoxWrapper<T>::render(const Rect& frame)
{
	if (m_externalRef)
		m_ownedValue = m_externalRef->get();

	if (sized(frame))
		ImGui::SetNextItemWidth((float)frame.width);
	ImGui::PushID(WidgetIdManager::nextWidgetId());
	bool changed = false;
	if constexpr (std::is_same_v<T, int>)
	{
		changed = ImGui::InputInt("##spinbox", &m_ownedValue, static_cast<int>(m_range.step));
		if (changed)
			m_ownedValue = std::clamp(m_ownedValue, m_range.min, m_range.max);
	}
	else
	{
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
Size RadioButtonWrapper<T>::measureIntrinsic(const Constraints&)
{
	return glyphLabelSize(m_label);
}

template <RadioButtonValue T>
void RadioButtonWrapper<T>::render(const Rect&)
{
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

Size CheckBoxWrapper::measureIntrinsic(const Constraints&)
{
	return glyphLabelSize(m_label);
}

void CheckBoxWrapper::render(const Rect&)
{
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

Size ToggleButtonWrapper::measureIntrinsic(const Constraints&)
{
	return framedTextSize(m_label);
}

void ToggleButtonWrapper::render(const Rect& frame)
{
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
	const bool clicked = sized(frame)
		? ImGui::Button(label, ImVec2((float)frame.width, (float)frame.height))
		: ImGui::Button(label);
	if (clicked)
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

Size ImageWrapper::measureIntrinsic(const Constraints&)
{
	// natural image size; header-only probe, no decode (explicit withSize
	// dimensions override per axis in measureContent)
	if (m_imgWidth == 0 && !m_filePath.empty())
		stbi_info(m_filePath.c_str(), &m_imgWidth, &m_imgHeight, nullptr);
	const int w = m_displayWidth  > 0 ? m_displayWidth  : m_imgWidth;
	const int h = m_displayHeight > 0 ? m_displayHeight : m_imgHeight;
	return Size { w, h };
}

void ImageWrapper::render(const Rect& frame)
{
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
		float w, h;
		if (sized(frame))
		{
			w = (float)frame.width;
			h = (float)frame.height;
		}
		else
		{
			w = (m_displayWidth  > 0) ? static_cast<float>(m_displayWidth)  : static_cast<float>(m_imgWidth);
			h = (m_displayHeight > 0) ? static_cast<float>(m_displayHeight) : static_cast<float>(m_imgHeight);
		}
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
Size ComboBoxWrapper<T>::measureIntrinsic(const Constraints&)
{
	// widest choice + frame padding + arrow square
	const ImGuiStyle& style = ImGui::GetStyle();
	float widest = 0.0f;
	for (const auto& choice : m_choices)
		widest = std::max(widest, ImGui::CalcTextSize(choice.c_str()).x);
	const float w = widest + style.FramePadding.x * 2.0f + ImGui::GetFrameHeight();
	return Size { ceilInt(w), frameHeight() };
}

template <ComboBoxValue T>
void ComboBoxWrapper<T>::render(const Rect& frame)
{
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

	if (sized(frame))
		ImGui::SetNextItemWidth((float)frame.width);
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

Size ColorPickerWrapper::measureIntrinsic(const Constraints&)
{
	return Size { kDefaultControlWidth, frameHeight() };
}

void ColorPickerWrapper::render(const Rect& frame)
{
	if (m_externalRef)
		m_ownedValue = m_externalRef->get();

	float col[4] = { m_ownedValue.r, m_ownedValue.g, m_ownedValue.b, m_ownedValue.a };
	if (sized(frame))
		ImGui::SetNextItemWidth((float)frame.width);
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

Size SeparatorWrapper::measureIntrinsic(const Constraints&)
{
	// a hairline; spans when stretched/expanded
	return Size { 0, 1 };
}

void SeparatorWrapper::render(const Rect&)
{
	ImGui::Separator();
}

// ProgressBarWrapper -----------------------------------------------------------

Size ProgressBarWrapper::measureIntrinsic(const Constraints&)
{
	return Size { kDefaultControlWidth, frameHeight() };
}

void ProgressBarWrapper::render(const Rect& frame)
{
	const float value = m_externalRef ? m_externalRef->get() : m_ownedValue;
	const ImVec2 size = sized(frame)
		? ImVec2((float)frame.width, (float)frame.height)
		: ImVec2(0.0f, 0.0f);
	ImGui::PushID(WidgetIdManager::nextWidgetId());
	auto clampedValue = std::clamp(value / 100.0f, 0.0f, 1.0f);
	ImGui::ProgressBar(clampedValue, size);
	ImGui::PopID();
}
