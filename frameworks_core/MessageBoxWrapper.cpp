#include "frameworks_core/MessageBoxWrapper.hpp"

#ifdef USE_WX
#include <wx/msgdlg.h>

namespace {

long mapButtons(MessageBoxButtons buttons)
{
	switch (buttons) {
	case MessageBoxButtons::OK:          return wxOK;
	case MessageBoxButtons::OKCancel:    return wxOK | wxCANCEL;
	case MessageBoxButtons::YesNo:       return wxYES_NO;
	case MessageBoxButtons::YesNoCancel: return wxYES_NO | wxCANCEL;
	}
	return wxOK;
}

long mapStyle(MessageBoxStyle style)
{
	switch (style) {
	case MessageBoxStyle::Info:     return wxICON_INFORMATION;
	case MessageBoxStyle::Warning:  return wxICON_WARNING;
	case MessageBoxStyle::Error:    return wxICON_ERROR;
	case MessageBoxStyle::Question: return wxICON_QUESTION;
	}
	return wxICON_INFORMATION;
}

MessageBoxResult mapResult(int wxResult)
{
	switch (wxResult) {
	case wxID_OK:     return MessageBoxResult::OK;
	case wxID_CANCEL: return MessageBoxResult::Cancel;
	case wxID_YES:    return MessageBoxResult::Yes;
	case wxID_NO:     return MessageBoxResult::No;
	}
	return MessageBoxResult::OK;
}

void showImpl(const std::string& title, const std::string& message,
	MessageBoxStyle style, MessageBoxButtons buttons,
	const std::function<void(MessageBoxResult)>& onResult)
{
	int result = wxMessageBox(message, title, mapButtons(buttons) | mapStyle(style));
	if (onResult)
		onResult(mapResult(result));
}

} // namespace

void MessageBoxWrapper::show(const std::string& title, const std::string& message,
	MessageBoxStyle style, MessageBoxButtons buttons,
	const std::function<void(MessageBoxResult)>& onResult)
{
	showImpl(title, message, style, buttons, onResult);
}

void MessageBoxWrapper::show(const std::string& title, const std::string& message,
	MessageBoxStyle style, MessageBoxButtons buttons,
	const std::function<void(MessageBoxResult)>& onResult,
	bool& visible)
{
	if (!visible)
		return;
	showImpl(title, message, style, buttons, onResult);
	visible = false;
}

#elif defined(USE_QT)
// Qt implementations would go here

void MessageBoxWrapper::show(const std::string&, const std::string&,
	MessageBoxStyle, MessageBoxButtons,
	const std::function<void(MessageBoxResult)>&)
{
}

void MessageBoxWrapper::show(const std::string&, const std::string&,
	MessageBoxStyle, MessageBoxButtons,
	const std::function<void(MessageBoxResult)>&,
	bool&)
{
}

#elif defined(USE_IMGUI)
#include "imgui.h"

namespace {

void renderButtons(MessageBoxButtons buttons,
	const std::function<void(MessageBoxResult)>& onResult,
	bool& visible)
{
	const float buttonWidth = 120.0f;

	auto closeWith = [&](MessageBoxResult result) {
		if (onResult)
			onResult(result);
		ImGui::CloseCurrentPopup();
		visible = false;
	};

	switch (buttons) {
	case MessageBoxButtons::OK:
		if (ImGui::Button("OK", ImVec2(buttonWidth, 0)))
			closeWith(MessageBoxResult::OK);
		break;

	case MessageBoxButtons::OKCancel:
		if (ImGui::Button("OK", ImVec2(buttonWidth, 0)))
			closeWith(MessageBoxResult::OK);
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0)))
			closeWith(MessageBoxResult::Cancel);
		break;

	case MessageBoxButtons::YesNo:
		if (ImGui::Button("Yes", ImVec2(buttonWidth, 0)))
			closeWith(MessageBoxResult::Yes);
		ImGui::SameLine();
		if (ImGui::Button("No", ImVec2(buttonWidth, 0)))
			closeWith(MessageBoxResult::No);
		break;

	case MessageBoxButtons::YesNoCancel:
		if (ImGui::Button("Yes", ImVec2(buttonWidth, 0)))
			closeWith(MessageBoxResult::Yes);
		ImGui::SameLine();
		if (ImGui::Button("No", ImVec2(buttonWidth, 0)))
			closeWith(MessageBoxResult::No);
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0)))
			closeWith(MessageBoxResult::Cancel);
		break;
	}
}

ImVec4 styleColor(MessageBoxStyle style)
{
	switch (style) {
	case MessageBoxStyle::Info:     return ImVec4(0.2f, 0.6f, 1.0f, 1.0f);
	case MessageBoxStyle::Warning:  return ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
	case MessageBoxStyle::Error:    return ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
	case MessageBoxStyle::Question: return ImVec4(0.4f, 0.8f, 0.4f, 1.0f);
	}
	return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
}

const char* styleLabel(MessageBoxStyle style)
{
	switch (style) {
	case MessageBoxStyle::Info:     return "[i]";
	case MessageBoxStyle::Warning:  return "/!\\";
	case MessageBoxStyle::Error:    return "[X]";
	case MessageBoxStyle::Question: return "[?]";
	}
	return "";
}

void showPopup(const std::string& title, const std::string& message,
	MessageBoxStyle style, MessageBoxButtons buttons,
	const std::function<void(MessageBoxResult)>& onResult,
	bool& visible)
{
	ImGui::OpenPopup(title.c_str());

	ImGui::SetNextWindowSizeConstraints(ImVec2(300, 0), ImVec2(FLT_MAX, FLT_MAX));
	if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::PushStyleColor(ImGuiCol_Text, styleColor(style));
		ImGui::TextUnformatted(styleLabel(style));
		ImGui::PopStyleColor();
		ImGui::SameLine();
		ImGui::TextUnformatted(message.c_str());
		ImGui::Separator();
		renderButtons(buttons, onResult, visible);
		ImGui::EndPopup();
	}
}

} // namespace

void MessageBoxWrapper::show(const std::string& title, const std::string& message,
	MessageBoxStyle style, MessageBoxButtons buttons,
	const std::function<void(MessageBoxResult)>& onResult)
{
	bool alwaysVisible = true;
	showPopup(title, message, style, buttons, onResult, alwaysVisible);
}

void MessageBoxWrapper::show(const std::string& title, const std::string& message,
	MessageBoxStyle style, MessageBoxButtons buttons,
	const std::function<void(MessageBoxResult)>& onResult,
	bool& visible)
{
	if (!visible)
		return;
	showPopup(title, message, style, buttons, onResult, visible);
}

#endif
