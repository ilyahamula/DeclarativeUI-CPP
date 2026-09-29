#pragma once

#include "frameworks_core/ToastWrapper.hpp"

#include <string>

// A timed, non-blocking notice -- "Saved" style feedback that does not need a
// message box. Like MessageBox and FileDialog it is not a widget: it builds no
// node and takes no part in a layout. Call it from a handler.
//
//     Toast{"Project saved"}.withDuration(2000).show();
//
// show() returns at once, the notice takes neither focus nor input, and it goes
// away on its own. Several live toasts stack at the bottom-right of the
// application window instead of overlapping.
struct Toast
{
	explicit Toast(const std::string& message)
		: m_message(message)
	{
	}

	// Info (default), Warning, Error or Question -- the MessageBox enum, drawn
	// as the colour of the toast's accent bar.
	Toast& withStyle(MessageBoxStyle style)
	{
		m_style = style;
		return *this;
	}

	// The whole lifetime in milliseconds, the closing fade included.
	// Non-positive means the default (ToastWrapper::kDefaultDurationMs).
	Toast& withDuration(int ms)
	{
		m_durationMs = ms;
		return *this;
	}

	// The message is copied here, so a toast built from a caller's string keeps
	// saying what it said when it was shown.
	void show() const
	{
		ToastWrapper::show(m_message, m_style, m_durationMs);
	}

private:
	std::string m_message;
	MessageBoxStyle m_style = MessageBoxStyle::Info;
	int m_durationMs = ToastWrapper::kDefaultDurationMs;
};
