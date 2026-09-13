#include "frameworks_core/WindowWrapper.hpp"

#include "frameworks_core/LayoutEngine.hpp"
#include "frameworks_core/LayoutNode.hpp"
#include "frameworks_core/wx/EngineSession.hpp"
#include "frameworks_core/wx/LayoutBackend.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#include <wx/wx.h>

void WindowWrapper::runLayoutEngine(const std::string& title, const Size& size,
	std::unique_ptr<LayoutNode> root, bool resizable, std::function<void()> onClose, bool* open)
{
#ifdef USE_LOGGER
	Logger::instance().log("WindowWrapper::runLayoutEngine()\t-> new wxFrame()\n");
#endif
	// The frame is what a Dialog cannot be: wxMenuBar attaches to a wxFrame and
	// nothing else (T2.2). Resizable is the default here, so Fixed() is what
	// takes wxRESIZE_BORDER away rather than Resizable() adding it.
	long style = wxDEFAULT_FRAME_STYLE;
	if (!resizable)
		style &= ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX);
	auto* frame = new wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, style);

	auto* session = new EngineSession;
	session->window = frame;
	// The frame's CLIENT area is the engine's content space -- menu, tool and
	// status bars all live outside it on every platform, so the engine needs to
	// know nothing about them.
	session->backend = std::make_unique<WxLayoutBackend>(frame);
	session->engine = std::make_unique<LayoutEngine>(*session->backend);
	session->root = std::move(root);
	session->resizable = resizable;
	session->openFlag = open;
	session->onClose = std::move(onClose);

	const wxRect workArea = wxGetClientDisplayRect();
	session->engine->setMaxAutoFitWidth((workArea.width * 9) / 10);

	// explicit Size is the total window size; the engine works in client space
	const bool fixed = size.width > 0 && size.height > 0;
	session->autoFit = !fixed;
	if (fixed)
	{
		frame->SetSize(size.width, size.height);
		const wxSize client = frame->GetClientSize();
		session->fixedContent = { client.x, client.y };
	}
	const Size content = session->engine->resolve(*session->root, session->autoFit
		? Size { -1, -1 }
		: session->fixedContent);
	if (session->autoFit)
		frame->SetClientSize(content.width, content.height);

	if (resizable)
	{
		// the measured content is the floor: the user can grow the window
		// but never shrink content into clipping
		const Size floor = session->minClient();
		frame->SetMinClientSize(wxSize(floor.width, floor.height));
	}

	session->engine->render(*session->root, content);

	// invalidation wiring -------------------------------------------------
	session->bindInvalidation(*session->root);
	session->bindOpenFlag();

	// user resize: arrange-only within the new client area
	frame->Bind(wxEVT_SIZE, [session](wxSizeEvent& event) {
		session->rearrange();
		event.Skip();
	});
	// display metrics changed: full re-measure
#if wxCHECK_VERSION(3, 1, 3)
	frame->Bind(wxEVT_DPI_CHANGED, [session](wxDPIChangedEvent& event) {
		session->relayout();
		event.Skip();
	});
#endif
	frame->Bind(wxEVT_SYS_COLOUR_CHANGED, [session](wxSysColourChangedEvent& event) {
		session->relayout();
		event.Skip();
	});
	// The single close path: the user's close button lands here, and so does
	// bindOpenFlag()'s Close() when the caller clears the flag. Skip() lets
	// wxFrame's default handler destroy the frame.
	frame->Bind(wxEVT_CLOSE_WINDOW, [session](wxCloseEvent& event) {
		session->notifyClosed();
		event.Skip();
	});
	frame->Bind(wxEVT_DESTROY, [session, frame](wxWindowDestroyEvent& event) {
		if (event.GetWindow() == frame)
		{
			// the app shutting down destroys top-level windows without always
			// routing through a close event
			session->notifyClosed();
			delete session;
		}
		event.Skip();
	});

	frame->Show();
}
