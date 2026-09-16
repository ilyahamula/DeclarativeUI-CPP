#include "frameworks_core/ControlWrappers.hpp"
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <optional>

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#include "imgui.h"
#include "frameworks_core/imgui/ImGuiWidgetIdManager.hpp"
#include "frameworks_core/imgui/SnapshotStore.hpp"

#include "frameworks_core/imgui/TextureCache.hpp"
#include "frameworks_core/imgui/FileBrowserPopup.hpp"

// Constructors only collect data and live inline in ControlWrappers.hpp.
// pos, size, style: not directly applicable in ImGui immediate mode.
// Empty labels are mapped to a hidden ("##...") id at render time so the
// stored label keeps the raw value the caller passed.
//
// Each wrapper implements the engine path (measureIntrinsic + render) per the
// measurement contract table in docs/specs/custom_layout_system/architecture.md.
// NOTE on unbound values: the tree is rebuilt every frame, so a wrapper's own
// snapshot would be overwritten by the caller's literal before every draw and
// no control declared with one could ever be changed. Every wrapper whose value
// the user can edit therefore opens a WidgetSnapshot (SnapshotStore.hpp) as the
// first thing render() does, which parks the snapshot outside the frame; bound
// values pass straight through, being the caller's variable already.
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
	WidgetSnapshot<std::string> snapshot(m_value);
	char buf[256] = {};
	std::snprintf(buf, sizeof(buf), "%s", m_value.get().c_str());
	if (sized(frame))
		ImGui::SetNextItemWidth((float)frame.width);
	ImGui::PushID(snapshot.id());
	if (ImGui::InputText("##textctrl", buf, sizeof(buf)))
	{
		m_value.set(buf);
		if (m_onChange)
			m_onChange(m_value.get());
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(m_value.get(), m_nativeWidget);
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
	WidgetSnapshot<std::string> snapshot(m_value);
	char buf[256] = {};
	std::snprintf(buf, sizeof(buf), "%s", m_value.get().c_str());
	if (sized(frame))
		ImGui::SetNextItemWidth((float)frame.width);
	ImGui::PushID(snapshot.id());
	if (ImGui::InputText("##passwordinput", buf, sizeof(buf), ImGuiInputTextFlags_Password))
	{
		m_value.set(buf);
		if (m_onChange)
			m_onChange(m_value.get());
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(m_value.get(), m_nativeWidget);
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
	WidgetSnapshot<std::string> snapshot(m_value);
	char buf[4096] = {};
	std::snprintf(buf, sizeof(buf), "%s", m_value.get().c_str());
	const ImVec2 size = sized(frame)
		? ImVec2((float)frame.width, (float)frame.height)
		: ImVec2(0, 0);
	ImGui::PushID(snapshot.id());
	if (ImGui::InputTextMultiline("##multilinetextctrl", buf, sizeof(buf), size))
	{
		m_value.set(buf);
		if (m_onChange)
			m_onChange(m_value.get());
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(m_value.get(), m_nativeWidget);
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
	WidgetSnapshot<Date> snapshot(m_value);
	// Edited in place: bound, this is the caller's Date; unbound, the snapshot
	// just restored into it -- either way the reference outlives the edit.
	Date& date = m_value.get();
	bool changed = false;
	ImGui::PushID(snapshot.id());
	{
		const ImGuiStyle& style = ImGui::GetStyle();
		const float btnW = (ImGui::GetFrameHeight() + style.ItemInnerSpacing.x) * 2.0f;
		const float padW = style.FramePadding.x * 2.0f;
		ImGui::SetNextItemWidth(ImGui::CalcTextSize("9999").x + padW + btnW);
		changed |= ImGui::InputInt("##dp_year",  &date.year,  1, 10);
		ImGui::SameLine(0, 4);
		ImGui::SetNextItemWidth(ImGui::CalcTextSize("12").x + padW + btnW);
		changed |= ImGui::InputInt("##dp_month", &date.month, 1, 0);
		ImGui::SameLine(0, 4);
		ImGui::SetNextItemWidth(ImGui::CalcTextSize("31").x + padW + btnW);
		changed |= ImGui::InputInt("##dp_day",   &date.day,   1, 0);
	}
	ImGui::PopID();
	if (changed)
	{
		date.month = std::clamp(date.month, 1, 12);
		date.day   = std::clamp(date.day,   1, 31);
		if (m_onChange)
			m_onChange(date);
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(date, m_nativeWidget);
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
	WidgetSnapshot<Time> snapshot(m_value);
	// Edited in place: bound, this is the caller's Time; unbound, the snapshot
	// just restored into it -- either way the reference outlives the edit.
	Time& time = m_value.get();
	bool changed = false;
	ImGui::PushID(snapshot.id());
	{
		const ImGuiStyle& style = ImGui::GetStyle();
		const float btnW  = (ImGui::GetFrameHeight() + style.ItemInnerSpacing.x) * 2.0f;
		const float padW  = style.FramePadding.x * 2.0f;
		const float twoDigitW = ImGui::CalcTextSize("59").x + padW + btnW;
		ImGui::SetNextItemWidth(ImGui::CalcTextSize("23").x + padW + btnW);
		changed |= ImGui::InputInt("##tp_hour",   &time.hour,   1, 0);
		ImGui::SameLine(0, 4);
		ImGui::SetNextItemWidth(twoDigitW);
		changed |= ImGui::InputInt("##tp_minute", &time.minute, 1, 0);
		ImGui::SameLine(0, 4);
		ImGui::SetNextItemWidth(twoDigitW);
		changed |= ImGui::InputInt("##tp_second", &time.second, 1, 0);
	}
	ImGui::PopID();
	if (changed)
	{
		time.hour   = std::clamp(time.hour,   0, 23);
		time.minute = std::clamp(time.minute, 0, 59);
		time.second = std::clamp(time.second, 0, 59);
		if (m_onChange)
			m_onChange(time);
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(time, m_nativeWidget);
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
	WidgetSnapshot<T> snapshot(m_value);
	if (sized(frame))
		ImGui::SetNextItemWidth((float)frame.width);
	ImGui::PushID(snapshot.id());
	bool changed = false;
	if constexpr (std::is_same_v<T, int>)
		changed = ImGui::SliderInt("##slider", &m_value.get(), m_range.min, m_range.max);
	else
		changed = ImGui::SliderFloat("##slider", &m_value.get(), m_range.min, m_range.max);
	ImGui::PopID();

	if (changed)
	{
		if (m_onChange)
			m_onChange(m_value.get());
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(m_value.get(), m_nativeWidget);
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
	WidgetSnapshot<T> snapshot(m_value);
	if (sized(frame))
		ImGui::SetNextItemWidth((float)frame.width);
	ImGui::PushID(snapshot.id());
	bool changed = false;
	if constexpr (std::is_same_v<T, int>)
	{
		changed = ImGui::InputInt("##spinbox", &m_value.get(), static_cast<int>(m_range.step));
		if (changed)
			m_value.set(std::clamp(m_value.get(), m_range.min, m_range.max));
	}
	else
	{
		changed = ImGui::InputFloat("##spinbox", &m_value.get(), m_range.step);
		if (changed)
			m_value.set(std::clamp(m_value.get(), m_range.min, m_range.max));
	}
	ImGui::PopID();

	if (changed)
	{
		if (m_onChange)
			m_onChange(m_value.get());
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(m_value.get(), m_nativeWidget);
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
	WidgetSnapshot<T> snapshot(m_value);
	const char* label = m_label.empty() ? "##radio" : m_label.c_str();
	ImGui::PushID(snapshot.id());
	if constexpr (std::is_same_v<T, bool>)
	{
		if (ImGui::RadioButton(label, m_value.get()))
		{
			m_value.set(!m_value.get());
			if (m_onChange)
				m_onChange(m_value.get());
			else if (m_onChangeWithWidget)
				m_onChangeWithWidget(m_value.get(), m_nativeWidget);
		}
	}
	else
	{
		if (ImGui::RadioButton(label, &m_value.get(), m_index))
		{
			if (m_onChange)
				m_onChange(m_value.get());
			else if (m_onChangeWithWidget)
				m_onChangeWithWidget(m_value.get(), m_nativeWidget);
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
	WidgetSnapshot<bool> snapshot(m_value);
	const char* label = m_label.empty() ? "##checkbox" : m_label.c_str();
	ImGui::PushID(snapshot.id());
	if (ImGui::Checkbox(label, &m_value.get()))
	{
		if (m_onChange)
			m_onChange(m_value.get());
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(m_value.get(), m_nativeWidget);
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
	WidgetSnapshot<bool> snapshot(m_value);
	const char* label = m_label.empty() ? "##toggle" : m_label.c_str();
	ImGui::PushID(snapshot.id());
	const bool wasToggled = m_value.get();
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
		m_value.set(!m_value.get());
		if (m_onChange)
			m_onChange(m_value.get());
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(m_value.get(), m_nativeWidget);
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
	if (m_imgWidth == 0)
		imageSizeFor(m_filePath, m_imgWidth, m_imgHeight);
	const int w = m_displayWidth  > 0 ? m_displayWidth  : m_imgWidth;
	const int h = m_displayHeight > 0 ? m_displayHeight : m_imgHeight;
	return Size { w, h };
}

void ImageWrapper::render(const Rect& frame)
{
	// Decoded and uploaded once per process, however many wrappers name the
	// same file -- the tree is rebuilt every frame, so this wrapper is not the
	// same object it was last frame.
	if (m_textureId == nullptr)
	{
		const CachedTexture& texture = textureFor(m_filePath);
		if (texture.valid())
		{
			m_textureId = texture.id;
			m_imgWidth = texture.width;
			m_imgHeight = texture.height;
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
	WidgetSnapshot<T> snapshot(m_value);
	// The value is authoritative, bound or not: unbound it is the snapshot the
	// store just restored, so the index buildItems() resolved from the declared
	// literal has to be re-resolved against it.
	if constexpr (std::is_same_v<T, int>)
	{
		m_currentItem = m_value.get();
	}
	else
	{
		for (int i = 0; i < static_cast<int>(m_choices.size()); ++i)
		{
			if (m_choices[i] == m_value.get())
			{
				m_currentItem = i;
				break;
			}
		}
	}

	if (sized(frame))
		ImGui::SetNextItemWidth((float)frame.width);
	ImGui::PushID(snapshot.id());
	if (ImGui::Combo("##combo", &m_currentItem, m_items.c_str()))
	{
		if constexpr (std::is_same_v<T, int>)
			m_value.set(m_currentItem);
		else if (m_currentItem >= 0 && m_currentItem < static_cast<int>(m_choices.size()))
			m_value.set(m_choices[m_currentItem]);

		if (m_onChange)
			m_onChange(m_value.get());
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(m_value.get(), m_nativeWidget);
	}
	ImGui::PopID();
}

template class ComboBoxWrapper<std::string>;
template class ComboBoxWrapper<int>;

// ListBoxWrapper -----------------------------------------------------------

template <ListBoxValue T>
Size ListBoxWrapper<T>::measureIntrinsic(const Constraints&)
{
	// Same shape as ImGui's own list-box default (rows of GetTextLineHeightWithSpacing
	// plus vertical frame padding), with the row count coming from the widget and
	// the width from the widest item + a scrollbar's worth of slack.
	const ImGuiStyle& style = ImGui::GetStyle();
	float widest = 0.0f;
	for (const auto& item : m_items)
		widest = std::max(widest, ImGui::CalcTextSize(item.c_str()).x);
	const float w = widest + style.FramePadding.x * 2.0f + style.ScrollbarSize;
	const float h = ImGui::GetTextLineHeightWithSpacing() * (float)m_visibleRows
		+ style.FramePadding.y * 2.0f;
	return Size { ceilInt(w), ceilInt(h) };
}

template <ListBoxValue T>
void ListBoxWrapper<T>::render(const Rect& frame)
{
	WidgetSnapshot<T> snapshot(m_value);
	// Read the selection back from the binding every frame: the tree is rebuilt
	// per frame anyway, so a value written from anywhere else is picked up for
	// free -- no ref sync needed here, unlike the retained backends.
	const std::vector<int> selection = indicesFor(m_items, boundValue());
	const auto isSelected = [&selection](int index) {
		return std::find(selection.begin(), selection.end(), index) != selection.end();
	};

	const ImVec2 box = sized(frame)
		? ImVec2((float)frame.width, (float)frame.height)
		: ImVec2(0.0f, 0.0f); // 0 = ImGui's default list-box size
	ImGui::PushID(snapshot.id());
	if (ImGui::BeginListBox("##listbox", box))
	{
		for (int i = 0; i < (int)m_items.size(); ++i)
		{
			if (!ImGui::Selectable(m_items[i].c_str(), isSelected(i)))
				continue;

			std::vector<int> next { i };
			if constexpr (kMultiSelect)
			{
				// ImGui has no native multi-select: a plain click replaces the
				// selection and ctrl/cmd-click toggles one row. Shift-click range
				// selection is left to the retained backends, which get it from
				// the platform for free.
				const ImGuiIO& io = ImGui::GetIO();
				if (io.KeyCtrl || io.KeySuper)
				{
					next = selection;
					if (const auto it = std::find(next.begin(), next.end(), i); it != next.end())
						next.erase(it);
					else
						next.insert(std::upper_bound(next.begin(), next.end(), i), i);
				}
			}
			commit(next);
		}
		ImGui::EndListBox();
	}
	ImGui::PopID();
}

template class ListBoxWrapper<int>;
template class ListBoxWrapper<std::string>;
template class ListBoxWrapper<std::vector<int>>;
template class ListBoxWrapper<std::vector<std::string>>;

// TreeViewWrapper -----------------------------------------------------------

template <TreeViewValue T>
Size TreeViewWrapper<T>::measureIntrinsic(const Constraints&)
{
	// Widest row is the deepest-indented label plus the arrow ImGui draws in
	// front of it (one indent step per level, including the item's own);
	// height is visibleRows of them, the same shape the ListBox measure has.
	const ImGuiStyle& style = ImGui::GetStyle();
	const float indent = ImGui::GetTreeNodeToLabelSpacing();
	float widest = 0.0f;
	forEachItem(m_items, [&](const TreeItem& item, const std::string&, int depth) {
		widest = std::max(widest,
			indent * (float)(depth + 1) + ImGui::CalcTextSize(item.label.c_str()).x);
	});
	const float w = widest + style.FramePadding.x * 2.0f + style.ScrollbarSize;
	const float h = ImGui::GetTextLineHeightWithSpacing() * (float)m_visibleRows
		+ style.FramePadding.y * 2.0f;
	return Size { ceilInt(w), ceilInt(h) };
}

template <TreeViewValue T>
void TreeViewWrapper<T>::render(const Rect& frame)
{
	WidgetSnapshot<T> snapshot(m_value);
	// Read the selection back from the binding every frame: the tree is rebuilt
	// per frame anyway, so a value written from anywhere else is picked up for
	// free -- no ref sync needed here, unlike the retained backends.
	const std::vector<std::string> selection = pathsFor(boundValue());
	const auto isSelected = [&selection](const std::string& path) {
		return std::find(selection.begin(), selection.end(), path) != selection.end();
	};

	const ImVec2 box = sized(frame)
		? ImVec2((float)frame.width, (float)frame.height)
		: ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * (float)m_visibleRows);

	std::vector<std::string> next;
	bool changed = false;

	ImGui::PushID(snapshot.id());
	// ImGui has no tree container the way it has BeginListBox, so a framed child
	// window is what gives the tree its scrollable box. BeginChild must be paired
	// with EndChild whatever it returns (ImGui >= 1.90).
	ImGui::BeginChild("##tree", box, ImGuiChildFlags_FrameStyle);

	// Expansion state is ImGui's, not ours: the wrapper dies with the frame and
	// could not remember what the user opened. Pushing the item's index per
	// level keeps node IDs stable across frames -- which is what lets ImGui's
	// own ID-keyed storage carry open/closed -- and keeps duplicate sibling
	// labels apart.
	const auto draw = [&](auto&& self, const std::vector<TreeItem>& items, const std::string& parentPath) -> void {
		for (int i = 0; i < (int)items.size(); ++i)
		{
			const TreeItem& item = items[i];
			const std::string path = joinPath(parentPath, item.label);
			const bool isLeaf = item.children.empty();

			// OpenOnArrow separates the two gestures a tree row carries: the
			// arrow toggles, the label selects. Without it a click would only
			// ever expand, and the control could never be selected from.
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
				| ImGuiTreeNodeFlags_OpenOnDoubleClick
				| ImGuiTreeNodeFlags_SpanAvailWidth;
			if (isLeaf)
				flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
			if (item.expanded)
				flags |= ImGuiTreeNodeFlags_DefaultOpen;
			if (isSelected(path))
				flags |= ImGuiTreeNodeFlags_Selected;

			ImGui::PushID(i);
			const bool open = ImGui::TreeNodeEx("##item", flags, "%s", item.label.c_str());
			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			{
				next = { path };
				if (m_multiSelect)
				{
					// ImGui has no native multi-select: a plain click replaces the
					// selection and ctrl/cmd-click toggles one row. Shift-click range
					// selection is left to the retained backends, which get it from
					// the platform for free.
					const ImGuiIO& io = ImGui::GetIO();
					if (io.KeyCtrl || io.KeySuper)
					{
						next = selection;
						if (const auto it = std::find(next.begin(), next.end(), path); it != next.end())
							next.erase(it);
						else
							next.push_back(path);
					}
				}
				changed = true;
			}
			if (open && !isLeaf)
			{
				self(self, item.children, path);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	};
	draw(draw, m_items, std::string {});

	ImGui::EndChild();
	ImGui::PopID();

	// Committed after the child closes: commit() runs the user's callback, which
	// may open a message box or otherwise draw, and that must not land inside
	// the tree's own window.
	if (changed)
		commit(next);
}

template class TreeViewWrapper<std::string>;
template class TreeViewWrapper<std::vector<std::string>>;

// TableWrapper -----------------------------------------------------------

namespace
{

// Text buffer for a cell being edited. Fixed size because imgui_stdlib (the
// std::string InputText overload) is not part of this build; a longer value is
// truncated as it is typed, never in the caller's data behind their back.
constexpr int kCellEditBufferSize = 256;

} // unnamed namespace

template <TableValue T>
Size TableWrapper<T>::measureIntrinsic(const Constraints&)
{
	const ImGuiStyle& style = ImGui::GetStyle();
	const auto measureText = [](const std::string& text) {
		return ceilInt(ImGui::CalcTextSize(text.c_str()).x);
	};

	// Cell padding is charged per column -- ImGui draws it on both sides of
	// every cell -- and the scrollbar once, the same accounting the list box
	// measure does.
	const TableRows& rows = m_rows.get();
	int width = ceilInt(style.ScrollbarSize);
	for (int column = 0; column < (int)m_columns.size(); ++column)
	{
		width += columnWidth(m_columns, rows, column, measureText)
			+ ceilInt(style.CellPadding.x * 2.0f);
	}

	// visibleRows + 1: the header row is always drawn, so it is always measured.
	// The sort arrow rides inside the header cell's own padding.
	const float height = ImGui::GetTextLineHeightWithSpacing() * (float)(m_visibleRows + 1)
		+ style.CellPadding.y * 2.0f;
	return Size { width, ceilInt(height) };
}

template <TableValue T>
void TableWrapper<T>::render(const Rect& frame)
{
	const int columnCount = (int)m_columns.size();
	if (columnCount == 0)
		return;

	// Two values on one control, so the rows take a slot of their own. Both are
	// restored before the reads below: `rows` binds a reference straight into
	// the storage a cell edit writes through.
	WidgetSnapshot<T> snapshot(m_value);
	SnapshotScope<TableRows> rowsSnapshot(snapshot.slotKey(1), m_rows);

	// Read rows and selection back from their bindings every frame: the tree is
	// rebuilt per frame anyway, so a value written from anywhere else is picked
	// up for free -- no ref sync needed here, unlike the retained backends.
	const TableRows& rows = m_rows.get();
	const std::vector<int> selection = rowIndicesFor(rows, boundValue());
	const auto isSelected = [&selection](int row) {
		return std::find(selection.begin(), selection.end(), row) != selection.end();
	};

	const bool anySortable = std::any_of(m_columns.begin(), m_columns.end(),
		[](const TableColumn& column) { return column.sortable; });

	// ScrollX rather than stretching columns to the frame: the three backends
	// should agree on column widths, so a table narrower than its frame leaves
	// the same gap everywhere, and a wider one scrolls rather than being
	// squeezed into a shape wx and Qt would not produce.
	ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders
		| ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;
	// SortTristate is what keeps the first frame UNSORTED. Plain Sortable makes
	// ImGui adopt a default sort column immediately, so a table would come up
	// ordered by its first sortable column here while wx and Qt come up in
	// declaration order -- the same tree looking different on one backend.
	// Tristate costs ImGui users a third click state (asc -> desc -> none) that
	// the retained backends do not offer; showing the caller's own row order
	// until a header is actually clicked is worth that.
	if (anySortable)
		tableFlags |= ImGuiTableFlags_Sortable | ImGuiTableFlags_SortTristate;

	const ImVec2 box = sized(frame)
		? ImVec2((float)frame.width, (float)frame.height)
		: ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * (float)(m_visibleRows + 1));

	ImGui::PushID(snapshot.id());

	// Which cell is open for editing has to outlive the frame, and this wrapper
	// does not -- the declarative tree is rebuilt every time. ImGui's own
	// ID-keyed storage is where that state lives, exactly as the TreeView's
	// open/closed state does. Taken before BeginTable so the keys hash against
	// the enclosing window rather than the table's inner one.
	ImGuiStorage* state = ImGui::GetStateStorage();
	const ImGuiID editRowKey = ImGui::GetID("##editRow");
	const ImGuiID editColKey = ImGui::GetID("##editCol");
	const ImGuiID editFocusKey = ImGui::GetID("##editFocus");
	int editRow = state->GetInt(editRowKey, -1);
	int editColumn = state->GetInt(editColKey, -1);
	bool focusPending = state->GetInt(editFocusKey, 0) != 0;

	// Committed after the table closes: commit()/commitCell() run the user's
	// callback, which may open a message box or otherwise draw, and that must
	// not land inside the table's own window.
	std::vector<int> nextSelection;
	bool selectionChanged = false;
	int editedRow = -1;
	int editedColumn = -1;
	std::string editedText;

	if (ImGui::BeginTable("##table", columnCount, tableFlags, box))
	{
		// The header stays put while the body scrolls.
		ImGui::TableSetupScrollFreeze(0, 1);
		const auto measureText = [](const std::string& text) {
			return ceilInt(ImGui::CalcTextSize(text.c_str()).x);
		};
		for (int column = 0; column < columnCount; ++column)
		{
			const TableColumn& spec = m_columns[column];
			ImGuiTableColumnFlags columnFlags = ImGuiTableColumnFlags_WidthFixed;
			if (!spec.sortable)
				columnFlags |= ImGuiTableColumnFlags_NoSort;
			// The shared width policy, and only the content width: ImGui adds
			// its own cell padding on top, which is exactly what
			// measureIntrinsic() budgeted for above.
			ImGui::TableSetupColumn(spec.label.c_str(), columnFlags,
				(float)columnWidth(m_columns, rows, column, measureText));
		}
		ImGui::TableHeadersRow();

		// ImGui reports which header the user clicked; the ordering itself is
		// ours, so it is the same lexicographic compare on cell text that wx and
		// Qt apply natively. Only the primary sort spec is honoured -- the
		// public API has no way to ask for a secondary one.
		int sortColumn = -1;
		bool ascending = true;
		if (ImGuiTableSortSpecs* specs = ImGui::TableGetSortSpecs(); specs && specs->SpecsCount > 0)
		{
			sortColumn = specs->Specs[0].ColumnIndex;
			ascending = specs->Specs[0].SortDirection == ImGuiSortDirection_Ascending;
		}
		const std::vector<int> order = sortedOrder(rows, sortColumn, ascending);

		for (int row : order)
		{
			ImGui::TableNextRow();
			// Keyed by the ORIGINAL index, not the display position, so a cell's
			// edit state follows its row across a re-sort.
			ImGui::PushID(row);
			for (int column = 0; column < columnCount; ++column)
			{
				ImGui::TableSetColumnIndex(column);
				ImGui::PushID(column);

				const std::string& text = cellText(rows, row, column);
				if (editRow == row && editColumn == column)
				{
					char buffer[kCellEditBufferSize];
					std::snprintf(buffer, sizeof(buffer), "%s", text.c_str());
					if (focusPending)
					{
						ImGui::SetKeyboardFocusHere();
						focusPending = false;
					}
					ImGui::SetNextItemWidth(-FLT_MIN);
					const bool entered = ImGui::InputText("##edit", buffer, sizeof(buffer),
						ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll);
					if (entered || ImGui::IsItemDeactivatedAfterEdit())
					{
						editedRow = row;
						editedColumn = column;
						editedText = buffer;
						editRow = editColumn = -1;
					}
					else if (ImGui::IsItemDeactivated())
					{
						// Escape, or a click elsewhere without an edit. ImGui has
						// already restored the original text, so there is nothing
						// to write back -- just close the editor.
						editRow = editColumn = -1;
					}
				}
				else
				{
					if (column == 0)
					{
						// The row's selectable IS the key column's cell, spanning
						// the whole row. AllowOverlap so the other columns' items
						// still take hover, which is what lets them be
						// double-clicked into an editor.
						if (ImGui::Selectable(text.c_str(), isSelected(row),
							ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap))
						{
							nextSelection = { row };
							if constexpr (kMultiSelect)
							{
								// ImGui has no native multi-select: a plain click
								// replaces the selection and ctrl/cmd-click toggles
								// one row. Shift-click range selection is left to
								// the retained backends, which get it from the
								// platform for free.
								const ImGuiIO& io = ImGui::GetIO();
								if (io.KeyCtrl || io.KeySuper)
								{
									nextSelection = selection;
									if (const auto it = std::find(nextSelection.begin(), nextSelection.end(), row);
										it != nextSelection.end())
										nextSelection.erase(it);
									else
										nextSelection.insert(
											std::upper_bound(nextSelection.begin(), nextSelection.end(), row), row);
								}
							}
							selectionChanged = true;
						}
					}
					else
					{
						ImGui::TextUnformatted(text.c_str());
					}

					// Double-click opens the editor, matching the gesture wx and
					// Qt use. Checked against the item just drawn, so it is the
					// cell under the cursor that opens rather than the row's.
					if (m_columns[column].editable
						&& ImGui::IsItemHovered()
						&& ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						editRow = row;
						editColumn = column;
						focusPending = true;
					}
				}

				ImGui::PopID();
			}
			ImGui::PopID();
		}
		ImGui::EndTable();
	}

	state->SetInt(editRowKey, editRow);
	state->SetInt(editColKey, editColumn);
	state->SetInt(editFocusKey, focusPending ? 1 : 0);
	ImGui::PopID();

	if (editedRow >= 0)
		commitCell(editedRow, editedColumn, editedText);
	if (selectionChanged)
		commit(nextSelection);
}

template class TableWrapper<int>;
template class TableWrapper<std::string>;
template class TableWrapper<std::vector<int>>;
template class TableWrapper<std::vector<std::string>>;

// ToolBarWrapper -----------------------------------------------------------

namespace
{

// ImGui has no toolbar, so one is drawn: a row of buttons with a drawn divider
// for separators. These two agree on the geometry so measure and render never
// disagree about how wide the row is.
constexpr float kToolBarSeparatorWidth = 9.0f;

float toolWidth(const ToolItem& tool, const Size& iconSize, bool labelsForced)
{
	if (tool.isSeparator)
		return kToolBarSeparatorWidth;

	const ImGuiStyle& style = ImGui::GetStyle();
	const bool hasIcon = !tool.iconPath.empty() && textureFor(tool.iconPath).valid();
	const bool showLabel = !hasIcon || labelsForced;

	float width = style.FramePadding.x * 2.0f;
	if (hasIcon)
		width += (float)iconSize.width;
	if (showLabel && !tool.label.empty())
	{
		if (hasIcon)
			width += style.ItemInnerSpacing.x;
		width += ImGui::CalcTextSize(tool.label.c_str()).x;
	}
	return width;
}

} // unnamed namespace

Size ToolBarWrapper::measureIntrinsic(const Constraints&)
{
	const ImGuiStyle& style = ImGui::GetStyle();
	float width = 0.0f;
	for (std::size_t index = 0; index < m_tools.size(); ++index)
	{
		if (index > 0)
			width += style.ItemSpacing.x;
		width += toolWidth(m_tools[index], m_iconSize, m_labelsForced);
	}

	// A tool is one frame high, or as high as its icon needs.
	const float height = std::max(ImGui::GetFrameHeight(),
		(float)m_iconSize.height + style.FramePadding.y * 2.0f);
	return Size { (int)std::ceil(width), (int)std::ceil(height) };
}

void ToolBarWrapper::render(const Rect& frame)
{
	const ImGuiStyle& style = ImGui::GetStyle();
	const float rowHeight = std::max(ImGui::GetFrameHeight(),
		(float)m_iconSize.height + style.FramePadding.y * 2.0f);

	ImGui::PushID(WidgetIdManager::nextWidgetId());
	for (std::size_t index = 0; index < m_tools.size(); ++index)
	{
		if (index > 0)
			ImGui::SameLine(0.0f, style.ItemSpacing.x);

		ToolItem& tool = m_tools[index];
		if (tool.isSeparator)
		{
			// Drawn by hand for the same reason SeparatorWrapper draws its own
			// line: ImGui::SeparatorEx spans the window rather than the space
			// it was given.
			const ImVec2 origin = ImGui::GetCursorScreenPos();
			const float x = origin.x + kToolBarSeparatorWidth * 0.5f;
			ImGui::GetWindowDrawList()->AddLine(
				ImVec2(x, origin.y + style.FramePadding.y),
				ImVec2(x, origin.y + rowHeight - style.FramePadding.y),
				ImGui::GetColorU32(ImGuiCol_Separator));
			ImGui::Dummy(ImVec2(kToolBarSeparatorWidth, rowHeight));
			continue;
		}

		ImGui::PushID((int)index);

		// An unbound toggle has no home in the wrapper -- the tree is rebuilt
		// every frame -- so it lives in the store keyed like the tool's own
		// ImGui state, exactly as every other snapshot value does.
		std::optional<SnapshotScope<bool>> snapshot;
		if (tool.toggledFlag)
			snapshot.emplace(WidgetIdManager::stateKey(WidgetIdManager::nextWidgetId()),
				*tool.toggledFlag);

		const bool down = tool.toggledFlag && tool.toggledFlag->get();
		if (down)
		{
			// The same colour push ToggleButtonWrapper uses, so a check tool
			// and a ToggleButton read as the same thing.
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
		}

		ImGui::BeginDisabled(tool.disabledFlag.get());

		const CachedTexture& icon = tool.iconPath.empty()
			? CachedTexture{}
			: textureFor(tool.iconPath);
		const bool showLabel = !icon.valid() || m_labelsForced;
		const float width = toolWidth(tool, m_iconSize, m_labelsForced);

		bool clicked = false;
		if (icon.valid())
		{
			// An icon tool draws the image and, when labels are forced, the
			// text beside it -- as one button, so the whole thing is clickable.
			const ImVec2 iconExtent((float)m_iconSize.width, (float)m_iconSize.height);
			if (showLabel && !tool.label.empty())
			{
				const ImVec2 cursor = ImGui::GetCursorScreenPos();
				clicked = ImGui::Button("##tool", ImVec2(width, rowHeight));
				ImDrawList* draw = ImGui::GetWindowDrawList();
				const float iconY = cursor.y + (rowHeight - iconExtent.y) * 0.5f;
				draw->AddImage((ImTextureID)(std::uintptr_t)icon.id,
					ImVec2(cursor.x + style.FramePadding.x, iconY),
					ImVec2(cursor.x + style.FramePadding.x + iconExtent.x, iconY + iconExtent.y));
				const ImVec2 textSize = ImGui::CalcTextSize(tool.label.c_str());
				draw->AddText(
					ImVec2(cursor.x + style.FramePadding.x + iconExtent.x + style.ItemInnerSpacing.x,
						cursor.y + (rowHeight - textSize.y) * 0.5f),
					ImGui::GetColorU32(ImGuiCol_Text), tool.label.c_str());
			}
			else
			{
				clicked = ImGui::ImageButton("##tool",
					(ImTextureID)(std::uintptr_t)icon.id, iconExtent);
			}
		}
		else
		{
			// No icon, or one that failed to load: the label IS the tool, so
			// the row is never blank (R9.3).
			const char* label = tool.label.empty() ? "##tool" : tool.label.c_str();
			clicked = ImGui::Button(label, ImVec2(width, rowHeight));
		}

		if (clicked)
		{
			// Value first, then the callback.
			if (tool.toggledFlag)
				tool.toggledFlag->set(!tool.toggledFlag->get());
			if (tool.clickHandler)
				tool.clickHandler();
		}

		ImGui::EndDisabled();
		if (down)
			ImGui::PopStyleColor(2);

		// EndDisabled leaves the item outside the disabled scope, so a disabled
		// tool would still answer IsItemHovered -- suppressed by hand here just
		// as the backend does for a leaf's tooltip.
		if (!tool.tooltip.empty() && !tool.disabledFlag.get())
			ImGui::SetItemTooltip("%s", tool.tooltip.c_str());

		ImGui::PopID();
	}
	ImGui::PopID();
	(void)frame;
}

// StatusBarWrapper -----------------------------------------------------------

Size StatusBarWrapper::measureIntrinsic(const Constraints&)
{
	const ImGuiStyle& style = ImGui::GetStyle();

	// Deliberately NOT measured from the live text: a status bar displays a
	// string written from elsewhere, so measuring it would let an arriving
	// message widen an auto-fit window. Fixed fields contribute their width,
	// stretch fields a constant, and Expand() gets the bar the rest of the row.
	float width = 0.0f;
	for (std::size_t index = 0; index < m_fields.size(); ++index)
	{
		if (index > 0)
			width += style.ItemSpacing.x;
		const StatusField& field = m_fields[index];
		width += (float)(field.width > 0 ? field.width : kDefaultStatusFieldWidth);
	}
	return Size { (int)std::ceil(width), (int)std::ceil(ImGui::GetFrameHeight()) };
}

void StatusBarWrapper::render(const Rect& frame)
{
	const ImGuiStyle& style = ImGui::GetStyle();
	const float height = (float)frame.height;
	const ImVec2 origin = ImGui::GetCursorScreenPos();

	// Fixed fields keep their width; the rest share what is left, equally.
	float fixedTotal = 0.0f;
	int stretchCount = 0;
	for (const StatusField& field : m_fields)
	{
		if (field.width > 0)
			fixedTotal += (float)field.width;
		else
			++stretchCount;
	}
	const float gaps = m_fields.empty() ? 0.0f : style.ItemSpacing.x * (float)(m_fields.size() - 1);
	const float leftover = std::max(0.0f, (float)frame.width - fixedTotal - gaps);
	const float stretchWidth = stretchCount > 0 ? leftover / (float)stretchCount : 0.0f;

	ImDrawList* draw = ImGui::GetWindowDrawList();
	float x = origin.x;
	for (std::size_t index = 0; index < m_fields.size(); ++index)
	{
		const StatusField& field = m_fields[index];
		const float fieldWidth = field.width > 0 ? (float)field.width : stretchWidth;

		if (index > 0)
		{
			// A thin divider between panes, drawn by hand -- ImGui::Separator()
			// would span the window instead of this row.
			const float lineX = x - style.ItemSpacing.x * 0.5f;
			draw->AddLine(ImVec2(lineX, origin.y + style.FramePadding.y),
				ImVec2(lineX, origin.y + height - style.FramePadding.y),
				ImGui::GetColorU32(ImGuiCol_Separator));
		}

		// Clipped to its own pane, so a long string cannot run into the next
		// field or out of the engine's frame.
		const std::string& text = field.text.get();
		const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
		const ImVec4 clip(x, origin.y, x + fieldWidth, origin.y + height);
		draw->PushClipRect(ImVec2(clip.x, clip.y), ImVec2(clip.z, clip.w), true);
		draw->AddText(ImVec2(x, origin.y + (height - textSize.y) * 0.5f),
			ImGui::GetColorU32(ImGuiCol_Text), text.c_str());
		draw->PopClipRect();

		x += fieldWidth + style.ItemSpacing.x;
	}

	// One item for the whole row, so the engine's frame is what it occupies.
	ImGui::Dummy(ImVec2((float)frame.width, height));
}

// ColorPickerWrapper -----------------------------------------------------------

Size ColorPickerWrapper::measureIntrinsic(const Constraints&)
{
	return Size { kDefaultControlWidth, frameHeight() };
}

void ColorPickerWrapper::render(const Rect& frame)
{
	WidgetSnapshot<Color> snapshot(m_value);
	const Color& cur = m_value.get();
	float col[4] = { cur.r, cur.g, cur.b, cur.a };
	if (sized(frame))
		ImGui::SetNextItemWidth((float)frame.width);
	ImGui::PushID(snapshot.id());
	if (ImGui::ColorEdit4("##colorpicker", col))
	{
		m_value.set(Color{ col[0], col[1], col[2], col[3] });
		if (m_onChange)
			m_onChange(m_value.get());
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(m_value.get(), m_nativeWidget);
	}
	ImGui::PopID();
}

// FilePickerWrapper -----------------------------------------------------------

namespace
{

// The Browse button's width, shared by measure and render so the two agree to
// the pixel -- the same reason ToolBarWrapper has a toolWidth() helper, and the
// thing the drift guard catches when they stop agreeing.
float browseButtonWidth()
{
	const ImGuiStyle& style = ImGui::GetStyle();
	return ImGui::CalcTextSize("...").x + style.FramePadding.x * 2.0f;
}

} // unnamed namespace

Size FilePickerWrapper::measureIntrinsic(const Constraints&)
{
	const ImGuiStyle& style = ImGui::GetStyle();
	// The field measures a content-independent floor like every other editable
	// one: the tree is rebuilt every frame, so measuring the live path would
	// grow the picker as the user typed into it.
	const float width = (float)editableFloorWidth() + style.ItemInnerSpacing.x + browseButtonWidth();
	return Size { ceilInt(width), frameHeight() };
}

void FilePickerWrapper::render(const Rect& frame)
{
	WidgetSnapshot<std::string> snapshot(m_value);
	const ImGuiStyle& style = ImGui::GetStyle();
	const float buttonWidth = browseButtonWidth();

	char buf[512] = {};
	std::snprintf(buf, sizeof(buf), "%s", m_value.get().c_str());

	ImGui::PushID(snapshot.id());
	if (sized(frame))
		ImGui::SetNextItemWidth(std::max(1.0f,
			(float)frame.width - buttonWidth - style.ItemInnerSpacing.x));
	if (ImGui::InputText("##path", buf, sizeof(buf)))
	{
		// A typed path is as much a selection as a picked one (R11.4).
		m_value.set(buf);
		if (m_onChange)
			m_onChange(m_value.get());
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(m_value.get(), m_nativeWidget);
	}
	ImGui::SameLine(0, style.ItemInnerSpacing.x);
	const bool browse = ImGui::Button("...", ImVec2(buttonWidth, 0));
	ImGui::PopID();

	if (!browse)
		return;

	// The browser answers on a LATER frame, by which time this wrapper is gone
	// -- the tree is rebuilt every frame. So the result has to be written
	// somewhere that outlives it, and that is exactly the split BoundValue
	// already makes: a bound path is the caller's variable, and an unbound one
	// lives in the SnapshotStore under the key this render() is using.
	std::string* bound = m_value.isBound() ? &m_value.get() : nullptr;
	const std::uint64_t snapshotKey = snapshot.slotKey(0);
	FileBrowser::request(m_dialogTitle, m_mode, m_filters, m_value.get(),
		[bound, snapshotKey, cb = m_onChange, cbw = m_onChangeWithWidget](const std::string& chosen) {
			if (chosen.empty())
				return; // cancel leaves the path alone -- it is not a selection of ""
			if (bound != nullptr)
				*bound = chosen;
			else
			{
				BoundValue<std::string> committed(chosen);
				SnapshotStore<std::string>::commit(snapshotKey, committed);
			}
			// Value first, then the callback, as everywhere else: a handler
			// reading the bound value sees the new one.
			if (cb) cb(chosen);
			else if (cbw) cbw(chosen, nullptr);
		});
}

// SpacerWrapper -----------------------------------------------------------

void SpacerWrapper::render(const Rect& frame)
{
	// Nothing is drawn, but an item of exactly the engine frame still has to be
	// emitted: it keeps the group EndGroup() reads non-empty, so the leaf's
	// item rect stays the frame the drift guard compares against.
	ImGui::Dummy(ImVec2((float)std::max(frame.width, 0), (float)std::max(frame.height, 0)));
}

// SeparatorWrapper -----------------------------------------------------------

Size SeparatorWrapper::measureIntrinsic(const Constraints&)
{
	// a hairline on its own axis and nothing on the other; spans when the caller
	// adds Expand(), exactly as it does on wx and Qt
	return m_orient == Orientation::Vertical ? Size { 1, 0 } : Size { 0, 1 };
}

void SeparatorWrapper::render(const Rect& frame)
{
	// ImGui::Separator() draws across the whole window and ignores the frame it
	// was given -- in a narrow column that overruns the engine's rectangle and
	// trips the drift guard. The line is therefore drawn by hand, inside the
	// frame, and a Dummy of the same size gives the leaf the item rect the
	// guard (and hover) reads. place() has already put the cursor at the frame's
	// top-left, so the cursor's screen position is the frame's origin.
	const ImVec2 origin = ImGui::GetCursorScreenPos();
	const float width = (float)std::max(frame.width, 0);
	const float height = (float)std::max(frame.height, 0);
	const ImU32 color = ImGui::GetColorU32(ImGuiCol_Separator);

	if (m_orient == Orientation::Vertical)
	{
		const float x = origin.x + width * 0.5f;
		ImGui::GetWindowDrawList()->AddLine(
			ImVec2(x, origin.y), ImVec2(x, origin.y + height), color);
	}
	else
	{
		const float y = origin.y + height * 0.5f;
		ImGui::GetWindowDrawList()->AddLine(
			ImVec2(origin.x, y), ImVec2(origin.x + width, y), color);
	}

	ImGui::Dummy(ImVec2(width, height));
}

// SplitterSashWrapper -----------------------------------------------------------

Size SplitterSashWrapper::measureIntrinsic(const Constraints&)
{
	// An UNBOUND position has no home in the wrapper -- the tree is rebuilt every
	// frame, so the caller's literal would win before every arrange and the sash
	// could never be moved. It is parked in the snapshot store instead, and
	// restored HERE rather than in render(): measure runs before arrange, so a
	// value restored now is the one this frame actually lays out at, where one
	// restored during render would always be a frame late and never converge.
	//
	// The key comes from the measure-phase counter because the engine resolves
	// BEFORE ImGui::Begin -- render() therefore draws in a different ImGui scope
	// than the one measure ran in, and the two sequences must not be shared.
	m_snapshotKey = WidgetIdManager::stateKey(
		WidgetIdManager::nextMeasureId(), WidgetIdManager::kMeasureSlot);
	SnapshotStore<int>::restore(m_snapshotKey, m_state->position);

	// A hairline's worth of grip on its own axis and nothing on the other, where
	// the engine stretches the sash across both panes.
	return m_state->orientation == Orientation::Horizontal
		? Size { SplitterState::kSashThickness, 0 }
		: Size { 0, SplitterState::kSashThickness };
}

void SplitterSashWrapper::render(const Rect& frame)
{
	const bool horizontal = m_state->orientation == Orientation::Horizontal;
	const ImVec2 size((float)std::max(frame.width, 0), (float)std::max(frame.height, 0));
	// place() has already put the cursor at the frame's top-left, so the
	// cursor's screen position IS the frame origin (as SeparatorWrapper uses it).
	const ImVec2 origin = ImGui::GetCursorScreenPos();

	// InvisibleButton asserts on a zero extent, and a splitter with no cross
	// band is legal (an empty pane in a collapsed row) -- draw nothing, but
	// still emit the item the drift guard reads.
	if (size.x <= 0.0f || size.y <= 0.0f)
	{
		ImGui::Dummy(size);
		SnapshotStore<int>::commit(m_snapshotKey, m_state->position);
		return;
	}

	ImGui::PushID(WidgetIdManager::nextWidgetId());
	ImGui::InvisibleButton("##sash", size);
	const bool active = ImGui::IsItemActive();
	const bool hovered = ImGui::IsItemHovered();
	if (active || hovered)
		ImGui::SetMouseCursor(horizontal ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_ResizeNS);

	// The drag anchors on the position arrange RESOLVED when the gesture began
	// -- accumulating per-frame mouse deltas instead would drift by a pixel
	// every time a fractional delta was truncated. The anchor outlives the frame
	// in ImGui's own storage, keyed inside this sash's PushID.
	ImGuiStorage* store = ImGui::GetStateStorage();
	const ImGuiID anchorKey = ImGui::GetID("##anchor");
	if (ImGui::IsItemActivated())
		store->SetInt(anchorKey, m_state->resolved);
	if (active)
	{
		const ImVec2 dragged = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 0.0f);
		const int moved = (int)(horizontal ? dragged.x : dragged.y);
		m_state->position.set(std::clamp(store->GetInt(anchorKey, m_state->resolved) + moved,
			m_state->lowerBound, m_state->upperBound));
	}

	const ImU32 color = ImGui::GetColorU32(active
		? ImGuiCol_SeparatorActive
		: (hovered ? ImGuiCol_SeparatorHovered : ImGuiCol_Separator));
	ImGui::GetWindowDrawList()->AddRectFilled(origin,
		ImVec2(origin.x + size.x, origin.y + size.y), color);
	ImGui::PopID();

	SnapshotStore<int>::commit(m_snapshotKey, m_state->position);
}

// ExpanderHeaderWrapper -----------------------------------------------------------

Size ExpanderHeaderWrapper::measureIntrinsic(const Constraints&)
{
	// An UNBOUND open state has no home in the wrapper -- the tree is rebuilt
	// every frame, so the caller's literal would win before every measure and a
	// section the user opened would close itself again. It is parked in the
	// snapshot store instead, and restored HERE rather than in render(): this
	// pass is the one that decides whether the content is measured at all, so a
	// value restored during render would always be a frame late.
	//
	// The key comes from the measure-phase counter because the engine resolves
	// BEFORE ImGui::Begin -- render() therefore draws in a different ImGui scope
	// than the one measure ran in, and the two sequences must not be shared.
	m_snapshotKey = WidgetIdManager::stateKey(
		WidgetIdManager::nextMeasureId(), WidgetIdManager::kMeasureSlot);
	SnapshotStore<bool>::restore(m_snapshotKey, m_state->expanded);

	const ImGuiStyle& style = ImGui::GetStyle();
	const float height = ImGui::GetFrameHeight();
	const ImVec2 text = ImGui::CalcTextSize(m_label.c_str());
	// arrow column (a square the height of the row) + the label, framed
	return Size {
		(int)std::ceil(height + style.ItemInnerSpacing.x + text.x + style.FramePadding.x * 2.0f),
		(int)std::ceil(height),
	};
}

void ExpanderHeaderWrapper::render(const Rect& frame)
{
	// ImGui::CollapsingHeader always spans to the window's work rect, whatever
	// rectangle the engine assigned it, so in a narrow column it would overrun
	// its frame exactly as ImGui::Separator() does. The row is therefore an
	// InvisibleButton of the engine's own size with the arrow and label drawn on
	// the window draw list -- one item, always inside the frame. place() has
	// already put the cursor at the frame's top-left, so the cursor's screen
	// position is the frame origin.
	const ImVec2 size((float)std::max(frame.width, 0), (float)std::max(frame.height, 0));
	const ImVec2 origin = ImGui::GetCursorScreenPos();

	// InvisibleButton asserts on a zero extent, and a header squeezed to nothing
	// is legal -- draw nothing, but still emit the item the drift guard reads.
	if (size.x <= 0.0f || size.y <= 0.0f)
	{
		ImGui::Dummy(size);
		SnapshotStore<bool>::commit(m_snapshotKey, m_state->expanded);
		return;
	}

	ImGui::PushID(WidgetIdManager::nextWidgetId());
	if (ImGui::InvisibleButton("##header", size))
		m_state->expanded.set(!m_state->expanded.get());
	const bool hovered = ImGui::IsItemHovered();
	const bool active = ImGui::IsItemActive();
	const bool open = m_state->expanded.get();

	const ImGuiStyle& style = ImGui::GetStyle();
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	const ImVec2 max(origin.x + size.x, origin.y + size.y);
	drawList->AddRectFilled(origin, max, ImGui::GetColorU32(active
		? ImGuiCol_HeaderActive
		: (hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header)));

	// The disclosure triangle: pointing down while open, right while closed --
	// the same two states wxCollapsibleHeaderCtrl and the Qt tool button show.
	const ImU32 textColor = ImGui::GetColorU32(ImGuiCol_Text);
	const float glyph = ImGui::GetFontSize();
	const ImVec2 centre(origin.x + style.FramePadding.x + glyph * 0.5f,
		origin.y + size.y * 0.5f);
	const float radius = glyph * 0.35f;
	if (open)
	{
		drawList->AddTriangleFilled(
			ImVec2(centre.x - radius, centre.y - radius * 0.6f),
			ImVec2(centre.x + radius, centre.y - radius * 0.6f),
			ImVec2(centre.x, centre.y + radius * 0.7f), textColor);
	}
	else
	{
		drawList->AddTriangleFilled(
			ImVec2(centre.x - radius * 0.6f, centre.y - radius),
			ImVec2(centre.x - radius * 0.6f, centre.y + radius),
			ImVec2(centre.x + radius * 0.7f, centre.y), textColor);
	}

	if (!m_label.empty())
	{
		const float textX = origin.x + style.FramePadding.x + glyph + style.ItemInnerSpacing.x;
		const float textY = origin.y + (size.y - ImGui::GetTextLineHeight()) * 0.5f;
		// Clipped to the frame: a title longer than the section must not spill
		// out of the rectangle the engine budgeted for it.
		const ImVec4 clip(origin.x, origin.y, max.x, max.y);
		drawList->AddText(nullptr, 0.0f, ImVec2(textX, textY), textColor,
			m_label.c_str(), nullptr, 0.0f, &clip);
	}
	ImGui::PopID();

	SnapshotStore<bool>::commit(m_snapshotKey, m_state->expanded);
}

// ProgressBarWrapper -----------------------------------------------------------

Size ProgressBarWrapper::measureIntrinsic(const Constraints&)
{
	return Size { kDefaultControlWidth, frameHeight() };
}

void ProgressBarWrapper::render(const Rect& frame)
{
	const float value = m_value.get();
	const ImVec2 size = sized(frame)
		? ImVec2((float)frame.width, (float)frame.height)
		: ImVec2(0.0f, 0.0f);
	ImGui::PushID(WidgetIdManager::nextWidgetId());
	auto clampedValue = std::clamp(value / 100.0f, 0.0f, 1.0f);
	ImGui::ProgressBar(clampedValue, size);
	ImGui::PopID();
}
