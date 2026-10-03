#pragma once

#include "frameworks_core/CoreTypes/Spinner.hpp"

#include <wx/graphics.h>
#include <wx/wx.h>

#include <algorithm>
#include <memory>

// The native half of a Spinner{}.withImage() on wx: a borderless panel that
// draws the picture turned about its centre. wxActivityIndicator cannot show a
// picture, so this is the one spinner wx draws itself.
//
// wxGraphicsContext does the rotation (Translate + Rotate + DrawBitmap), so no
// rotated copies are kept; a plain wxPaintDC under it lets the parent's
// background show through, as RichTextPanel does. The clock is a wxTimer the
// panel owns, running only while the spinner does -- the idle event is the
// wrong driver for an animation (see ProgressBarWrapper's busy bar).
class SpinnerPanel : public wxPanel
{
public:
	SpinnerPanel(wxWindow* parent, const wxImage& image)
		: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
		, m_bitmap(image)
		, m_timer(this)
	{
		Bind(wxEVT_PAINT, &SpinnerPanel::onPaint, this);
		Bind(wxEVT_SIZE, [this](wxSizeEvent& e) {
			Refresh();
			e.Skip();
		});
		Bind(wxEVT_TIMER, [this](wxTimerEvent&) { Refresh(); });
	}

	void setRunning(bool running)
	{
		if (running == m_running)
			return;
		m_running = running;
		if (running)
		{
			m_started = wxGetLocalTimeMillis();
			m_timer.Start(kSpinnerFrameMs);
		}
		else
		{
			m_timer.Stop();
		}
		Refresh();
	}

	bool isRunning() const { return m_running; }

	// Not every port repaints on an enable change, and a disabled spinner
	// draws dimmed.
	bool Enable(bool enable = true) override
	{
		const bool changed = wxPanel::Enable(enable);
		Refresh();
		return changed;
	}

private:
	void onPaint(wxPaintEvent&)
	{
		wxPaintDC dc(this);
		std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
		if (!gc || !m_bitmap.IsOk())
			return;
		const wxSize client = GetClientSize();
		// Fit the square, aspect kept, so the picture turns inside its frame.
		const double side = std::min(client.x, client.y);
		const double scale = side / std::max(m_bitmap.GetWidth(), m_bitmap.GetHeight());
		const double w = m_bitmap.GetWidth() * scale;
		const double h = m_bitmap.GetHeight() * scale;
		const double angle = m_running ? spinnerAngle((wxGetLocalTimeMillis() - m_started).GetValue()) : 0.0;
		gc->Translate(client.x * 0.5, client.y * 0.5);
		gc->Rotate(angle);
		if (!IsEnabled())
			gc->BeginLayer(0.4);
		gc->DrawBitmap(m_bitmap, -w * 0.5, -h * 0.5, w, h);
		if (!IsEnabled())
			gc->EndLayer();
	}

	wxBitmap m_bitmap;
	wxTimer m_timer;
	bool m_running = false;
	wxLongLong m_started;
};
