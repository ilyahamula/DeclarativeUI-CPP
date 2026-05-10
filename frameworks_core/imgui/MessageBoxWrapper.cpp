#include "frameworks_core/MessageBoxWrapper.hpp"
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
