#include "frameworks_core/MessageBoxWrapper.hpp"
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
