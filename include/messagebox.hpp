#pragma once

#include "frameworks_core/MessageBoxWrapper.hpp"

#include <functional>
#include <string>

struct MessageBox
{
	MessageBox(const std::string& title, const std::string& message)
		: m_title(title)
		, m_message(message)
	{
	}

	MessageBox& withStyle(MessageBoxStyle style)
	{
		m_style = style;
		return *this;
	}

	MessageBox& withButtons(MessageBoxButtons buttons)
	{
		m_buttons = buttons;
		return *this;
	}

	MessageBox& onResult(std::function<void(MessageBoxResult)> callback)
	{
		m_onResult = std::move(callback);
		return *this;
	}

	void show()
	{
		MessageBoxWrapper::show(m_title, m_message, m_style, m_buttons, m_onResult);
	}

	void show(bool& visible)
	{
		MessageBoxWrapper::show(m_title, m_message, m_style, m_buttons, m_onResult, visible);
	}

private:
	std::string m_title;
	std::string m_message;
	MessageBoxStyle m_style = MessageBoxStyle::Info;
	MessageBoxButtons m_buttons = MessageBoxButtons::OK;
	std::function<void(MessageBoxResult)> m_onResult;
};
