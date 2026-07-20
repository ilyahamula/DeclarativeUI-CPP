#include "frameworks_core/MessageBoxWrapper.hpp"

#include <QMessageBox>

namespace
{

QMessageBox::Icon toIcon(MessageBoxStyle style)
{
	switch (style)
	{
	case MessageBoxStyle::Warning: return QMessageBox::Warning;
	case MessageBoxStyle::Error: return QMessageBox::Critical;
	case MessageBoxStyle::Question: return QMessageBox::Question;
	case MessageBoxStyle::Info: break;
	}
	return QMessageBox::Information;
}

QMessageBox::StandardButtons toButtons(MessageBoxButtons buttons)
{
	switch (buttons)
	{
	case MessageBoxButtons::OKCancel: return QMessageBox::Ok | QMessageBox::Cancel;
	case MessageBoxButtons::YesNo: return QMessageBox::Yes | QMessageBox::No;
	case MessageBoxButtons::YesNoCancel:
		return QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
	case MessageBoxButtons::OK: break;
	}
	return QMessageBox::Ok;
}

MessageBoxResult toResult(int standardButton)
{
	switch (standardButton)
	{
	case QMessageBox::Cancel: return MessageBoxResult::Cancel;
	case QMessageBox::Yes: return MessageBoxResult::Yes;
	case QMessageBox::No: return MessageBoxResult::No;
	default: break;
	}
	return MessageBoxResult::OK;
}

} // unnamed namespace

void MessageBoxWrapper::show(const std::string& title, const std::string& message,
	MessageBoxStyle style, MessageBoxButtons buttons,
	const std::function<void(MessageBoxResult)>& onResult)
{
	QMessageBox box(toIcon(style), QString::fromStdString(title),
		QString::fromStdString(message), toButtons(buttons));
	const int result = box.exec();
	if (onResult)
		onResult(toResult(result));
}

void MessageBoxWrapper::show(const std::string& title, const std::string& message,
	MessageBoxStyle style, MessageBoxButtons buttons,
	const std::function<void(MessageBoxResult)>& onResult,
	bool& visible)
{
	if (!visible)
		return;
	visible = false;
	show(title, message, style, buttons, onResult);
}
