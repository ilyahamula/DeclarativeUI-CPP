#pragma once

#include "frameworks_core/CoreTypes/RichTextRuns.hpp"

#include <wx/wx.h>
#include <wx/settings.h>

#include <array>
#include <functional>
#include <string>
#include <utility>
#include <vector>

// The native half of a RichText on wx: a borderless panel that draws the
// fragments layoutRichText() places, in wxFont variants of its own font.
//
// Chosen over wxRichTextCtrl, which is a text EDITOR with a caret, a scroll
// bar and a document model to fight, and over wxHtmlWindow, which has no
// reliable intrinsic height -- and either would wrap by its own rule where the
// point is that all three backends wrap by the shared one.
//
// A plain wxPaintDC rather than a buffered one: the system erases the
// background first, so the panel shows whatever its parent is painted with (a
// themed notebook page on MSW) and never has to guess that colour itself.
class RichTextPanel : public wxPanel
{
public:
	RichTextPanel(wxWindow* parent, std::vector<TextRun> runs)
		: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
		, m_runs(std::move(runs))
	{
		for (int i = 0; i < 8; ++i)
		{
			wxFont font = GetFont();
			if (i & kBold)
				font.MakeBold();
			if (i & kItalic)
				font.MakeItalic();
			if (i & kUnderline)
				font.MakeUnderlined();
			m_fonts[i] = font;
		}
		for (int i = 0; i < 4; ++i)
		{
			int w = 0;
			int h = 0;
			GetTextExtent(wxT("Ag"), &w, &h, nullptr, nullptr, &m_fonts[i]);
			m_lineHeight = std::max(m_lineHeight, h);
		}

		Bind(wxEVT_PAINT, &RichTextPanel::onPaint, this);
		// The wrap depends on the width, so any resize can move every line.
		Bind(wxEVT_SIZE, [this](wxSizeEvent& e) {
			Refresh();
			e.Skip();
		});
		Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& e) {
			m_pressed = linkUnder(e.GetPosition());
			e.Skip();
		});
		// A link fires when the press and the release both land on it. wx
		// delivers LEFT_UP to whatever window is under the pointer, pressed
		// there or not, so the press has to be remembered.
		Bind(wxEVT_LEFT_UP, [this](wxMouseEvent& e) {
			const std::string* released = linkUnder(e.GetPosition());
			const bool fire = released != nullptr && released == m_pressed;
			m_pressed = nullptr;
			e.Skip();
			if (fire && m_onLink)
			{
				const std::string url = *released;
				m_onLink(url);
			}
		});
		Bind(wxEVT_MOTION, [this](wxMouseEvent& e) {
			SetCursor(linkUnder(e.GetPosition()) != nullptr ? wxCursor(wxCURSOR_HAND) : wxNullCursor);
			e.Skip();
		});
		Bind(wxEVT_LEAVE_WINDOW, [this](wxMouseEvent& e) {
			SetCursor(wxNullCursor);
			m_pressed = nullptr;
			e.Skip();
		});
	}

	void setOnLink(std::function<void(const std::string&)> onLink)
	{
		m_onLink = std::move(onLink);
	}

	RichTextLayout layoutFor(int width) const
	{
		return layoutRichText(m_runs, width, m_lineHeight,
			[this](const TextRun& run, std::string_view text) {
				int w = 0;
				int h = 0;
				GetTextExtent(wxString::FromUTF8(text.data(), text.size()), &w, &h,
					nullptr, nullptr, &fontFor(run));
				return w;
			});
	}

	// Text is painted grey while disabled, and wx does not repaint a window
	// whose enabled state changed on every port.
	bool Enable(bool enable = true) override
	{
		const bool changed = wxPanel::Enable(enable);
		if (changed)
			Refresh();
		return changed;
	}

	bool AcceptsFocus() const override
	{
		return false;
	}

private:
	static constexpr int kBold = 1;
	static constexpr int kItalic = 2;
	static constexpr int kUnderline = 4;

	const wxFont& fontFor(const TextRun& run) const
	{
		return m_fonts[(run.bold ? kBold : 0) | (run.italic ? kItalic : 0)
			| (run.isLink() ? kUnderline : 0)];
	}

	wxColour colourFor(const TextRun& run) const
	{
		if (!IsEnabled())
			return wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
		if (run.colour)
			return wxColour((unsigned char)(run.colour->r * 255.0f + 0.5f),
				(unsigned char)(run.colour->g * 255.0f + 0.5f),
				(unsigned char)(run.colour->b * 255.0f + 0.5f),
				(unsigned char)(run.colour->a * 255.0f + 0.5f));
		if (run.isLink())
			return wxSystemSettings::GetColour(wxSYS_COLOUR_HOTLIGHT);
		return GetForegroundColour();
	}

	const std::string* linkUnder(const wxPoint& p) const
	{
		return linkAt(layoutFor(GetClientSize().x), m_runs, p.x, p.y);
	}

	void onPaint(wxPaintEvent&)
	{
		wxPaintDC dc(this);
		dc.SetBackgroundMode(wxBRUSHSTYLE_TRANSPARENT);
		const RichTextLayout layout = layoutFor(GetClientSize().x);
		for (const RichTextFragment& f : layout.fragments)
		{
			const TextRun& run = m_runs[f.run];
			dc.SetFont(fontFor(run));
			dc.SetTextForeground(colourFor(run));
			dc.DrawText(wxString::FromUTF8(f.text.data(), f.text.size()), f.x, f.y);
		}
	}

	std::vector<TextRun> m_runs;
	std::function<void(const std::string&)> m_onLink;
	std::array<wxFont, 8> m_fonts;
	int m_lineHeight = 0;
	const std::string* m_pressed = nullptr;
};
