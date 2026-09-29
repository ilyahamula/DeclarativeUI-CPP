#include "frameworks_core/MessageBoxWrapper.hpp"
#include "frameworks_core/TopLevelShow.hpp"
#include "imgui.h"

#include <cfloat>
#include <string>
#include <unordered_map>

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

// Draw the box for one frame against `open`, the flag that says whether it is
// up. The same two rules imgui/DialogWrapper.cpp follows for a Modal() dialog:
//
//  * OpenPopup only while the popup is not already open -- ImGui treats an
//    OpenPopup() every frame as a mistake and merely suppresses it;
//  * once `open` is cleared, one last BeginPopupModal with the cleared flag.
//    ImGui never drops a popup just because nobody submitted it, so skipping
//    that call would leave an invisible modal blocking every window behind it.
void present(const std::string& title, const std::string& message,
	MessageBoxStyle style, MessageBoxButtons buttons,
	const std::function<void(MessageBoxResult)>& onResult,
	bool* open)
{
	if (!*open)
	{
		if (ImGui::IsPopupOpen(title.c_str()) && ImGui::BeginPopupModal(title.c_str(), open))
			ImGui::EndPopup();
		return;
	}
	if (!ImGui::IsPopupOpen(title.c_str()))
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
		renderButtons(buttons, onResult, *open);
		ImGui::EndPopup();
	}
}

// The flag for a box shown with no caller flag from the caller's own frame
// loop, keyed by title as ImGui keys the popup -- the move DialogWrapper.cpp
// makes for a flagless Dialog. Answering the box clears it, so a loop that
// keeps calling show() does not reopen what the user just dismissed.
bool& ownedOpenFlag(const std::string& title)
{
	static std::unordered_map<std::string, bool> flags;
	return flags.try_emplace(title, true).first->second;
}

// Both show()s end here. From a HANDLER (a click, a menu action, another box's
// onResult) a single call has to keep the box up until it is answered, which
// is what every other backend does -- so the box is adopted and redrawn every
// frame, exactly as a handler's Dialog is (TopLevelShow.hpp). From the
// caller's frame loop, the call IS the frame.
void showImpl(const std::string& title, const std::string& message,
	MessageBoxStyle style, MessageBoxButtons buttons,
	const std::function<void(MessageBoxResult)>& onResult,
	bool* open)
{
	if (TopLevelShow::issuedFromFrame())
	{
		TopLevelShow::adopt(title, open,
			[title, message, style, buttons, onResult](bool* flag) {
				present(title, message, style, buttons, onResult, flag);
			});
		return;
	}
	present(title, message, style, buttons, onResult,
		open != nullptr ? open : &ownedOpenFlag(title));
}

} // namespace

void MessageBoxWrapper::show(const std::string& title, const std::string& message,
	MessageBoxStyle style, MessageBoxButtons buttons,
	const std::function<void(MessageBoxResult)>& onResult)
{
	showImpl(title, message, style, buttons, onResult, nullptr);
}

void MessageBoxWrapper::show(const std::string& title, const std::string& message,
	MessageBoxStyle style, MessageBoxButtons buttons,
	const std::function<void(MessageBoxResult)>& onResult,
	bool& visible)
{
	// Not an early return on !visible: a box whose flag was just cleared still
	// needs its closing call (see present()).
	showImpl(title, message, style, buttons, onResult, &visible);
}
