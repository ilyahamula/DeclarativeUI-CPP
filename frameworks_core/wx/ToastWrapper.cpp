#include "frameworks_core/ToastWrapper.hpp"
#include "frameworks_core/wx/ToastWindow.hpp"

#include <wx/display.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/settings.h>
#include <wx/stattext.h>
#include <wx/time.h>

#include <algorithm>

namespace
{

// The geometry every backend's toast shares (imgui/ToastQueue.cpp), so the
// three look alike.
constexpr int kMargin = 16;
constexpr int kSpacing = 8;
constexpr int kPadX = 12;
constexpr int kPadY = 10;
constexpr int kAccentW = 4;
constexpr int kMaxTextW = 360;
constexpr int kTickMs = 30;

wxColour accentColor(MessageBoxStyle style)
{
	switch (style)
	{
	case MessageBoxStyle::Warning:  return wxColour(255, 204, 0);
	case MessageBoxStyle::Error:    return wxColour(255, 51, 51);
	case MessageBoxStyle::Question: return wxColour(102, 204, 102);
	case MessageBoxStyle::Info:     break;
	}
	return wxColour(51, 153, 255);
}

bool isToast(const wxWindow* window)
{
	return dynamic_cast<const ToastWindow*>(window) != nullptr;
}

wxRect clientRectOnScreen(const wxTopLevelWindow* window)
{
	return wxRect(window->ClientToScreen(wxPoint(0, 0)), window->GetClientSize());
}

// Where the stack hangs from: the client area of the active window, else of any
// shown application window, else the display. A toast never activates, so it
// can never be the active window itself; isToast() keeps it from being the
// fallback either.
wxRect anchorRect()
{
	const wxTopLevelWindow* fallback = nullptr;
	for (wxWindow* window : wxTopLevelWindows)
	{
		auto* tlw = dynamic_cast<wxTopLevelWindow*>(window);
		if (tlw == nullptr || isToast(tlw) || !tlw->IsShown() || tlw->IsBeingDeleted())
			continue;
		if (tlw->IsActive())
			return clientRectOnScreen(tlw);
		if (fallback == nullptr)
			fallback = tlw;
	}
	if (fallback != nullptr)
		return clientRectOnScreen(fallback);
	return wxDisplay().GetClientArea();
}

} // unnamed namespace

ToastWindow::ToastWindow(const std::string& message, MessageBoxStyle style, int durationMs)
	: wxFrame(nullptr, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
		wxFRAME_TOOL_WINDOW | wxFRAME_NO_TASKBAR | wxSTAY_ON_TOP | wxBORDER_NONE)
	, m_durationMs(durationMs > 0 ? durationMs : ToastWrapper::kDefaultDurationMs)
	, m_timer(this)
{
	auto* frame = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
	frame->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

	auto* accent = new wxPanel(frame, wxID_ANY, wxDefaultPosition, wxSize(kAccentW, -1));
	accent->SetBackgroundColour(accentColor(style));
	accent->SetMinSize(wxSize(kAccentW, -1));

	auto* label = new wxStaticText(frame, wxID_ANY, wxString::FromUTF8(message));
	label->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
	// Wrap() is a no-op for text that already fits, so short messages keep
	// their own width and only long ones break -- at exactly the cap.
	label->Wrap(kMaxTextW);

	// A private sizer inside a native helper, like Qt's FilePicker composite:
	// nothing in here is a LayoutNode, so the engine has nothing to own.
	// A wx border is one width per item, so the text's two paddings take two
	// sizers; the accent sits outside both and runs the full height.
	auto* text = new wxBoxSizer(wxVERTICAL);
	text->Add(label, 0, wxTOP | wxBOTTOM, kPadY);
	auto* row = new wxBoxSizer(wxHORIZONTAL);
	row->Add(accent, 0, wxEXPAND);
	row->Add(text, 0, wxLEFT | wxRIGHT, kPadX);
	frame->SetSizer(row);

	auto* outer = new wxBoxSizer(wxVERTICAL);
	outer->Add(frame, 1, wxEXPAND);
	SetSizerAndFit(outer);

	Bind(wxEVT_TIMER, &ToastWindow::onTick, this);
}

ToastWindow::~ToastWindow()
{
	m_timer.Stop();
	auto& live = stack();
	live.erase(std::remove(live.begin(), live.end(), this), live.end());
	reflow();
}

void ToastWindow::popUp()
{
	stack().push_back(this);
	reflow();
	ShowWithoutActivating();

	m_shownAt = wxGetLocalTimeMillis();
	m_timer.Start(kTickMs);
}

void ToastWindow::onTick(wxTimerEvent&)
{
	const long elapsed = (wxGetLocalTimeMillis() - m_shownAt).ToLong();
	const long remaining = m_durationMs - elapsed;
	if (remaining <= 0)
	{
		m_timer.Stop();
		Destroy();
		return;
	}
	if (remaining < ToastWrapper::kToastFadeMs && CanSetTransparent())
		SetTransparent(static_cast<wxByte>(255 * remaining / ToastWrapper::kToastFadeMs));
}

void ToastWindow::reflow()
{
	const auto& live = stack();
	if (live.empty())
		return;

	const wxRect anchor = anchorRect();
	const int right = anchor.GetRight() - kMargin;
	int bottom = anchor.GetBottom() - kMargin;
	for (ToastWindow* toast : live)
	{
		const wxSize size = toast->GetSize();
		toast->Move(right - size.x + 1, bottom - size.y + 1);
		bottom -= size.y + kSpacing;
	}
}

std::vector<ToastWindow*>& ToastWindow::stack()
{
	static std::vector<ToastWindow*> live;
	return live;
}

void ToastWrapper::show(const std::string& message, MessageBoxStyle style, int durationMs)
{
	(new ToastWindow(message, style, durationMs))->popUp();
}
