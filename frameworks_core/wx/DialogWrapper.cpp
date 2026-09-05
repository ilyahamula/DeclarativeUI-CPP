#include "frameworks_core/DialogWrapper.hpp"
#include "frameworks_core/ControlWrappers.hpp"

#include "frameworks_core/LayoutEngine.hpp"
#include "frameworks_core/LayoutNode.hpp"
#include "frameworks_core/wx/EngineSession.hpp"
#include "frameworks_core/wx/LayoutBackend.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#include <wx/wx.h>

DialogWrapper::DialogWrapper(const std::string& title, const Size& size)
{
#ifdef USE_LOGGER
	Logger::instance().log("DialogWrapper::DialogWrapper()\t-> new wxDialog()\n");
#endif
	m_nativeWidget = new wxDialog(nullptr, wxID_ANY, title,
		wxDefaultPosition, wxSize(size.width, size.height));
}

void DialogWrapper::show()
{
#ifdef USE_LOGGER
	Logger::instance().log("DialogWrapper::show()\t-> wxDialog->Show()\n");
#endif
	static_cast<wxDialog*>(m_nativeWidget)->Show();
}

void DialogWrapper::runLayoutEngine(const std::string& title, const Size& size,
	std::unique_ptr<LayoutNode> root, bool resizable)
{
	long style = wxDEFAULT_DIALOG_STYLE; // not user-resizable by default
	if (resizable)
		style |= wxRESIZE_BORDER;
	auto* dialog = new wxDialog(nullptr, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, style);

	auto* session = new EngineSession;
	session->window = dialog;
	session->backend = std::make_unique<WxLayoutBackend>(dialog);
	session->engine = std::make_unique<LayoutEngine>(*session->backend);
	session->root = std::move(root);
	session->resizable = resizable;

	const wxRect workArea = wxGetClientDisplayRect();
	session->engine->setMaxAutoFitWidth((workArea.width * 9) / 10);

	// explicit Size is the total window size; the engine works in client space
	const bool fixed = size.width > 0 && size.height > 0;
	session->autoFit = !fixed;
	if (fixed)
	{
		dialog->SetSize(size.width, size.height);
		const wxSize client = dialog->GetClientSize();
		session->fixedContent = { client.x, client.y };
	}
	const Size content = session->engine->resolve(*session->root, session->autoFit
		? Size { -1, -1 }
		: session->fixedContent);
	if (session->autoFit)
		dialog->SetClientSize(content.width, content.height);

	if (resizable)
	{
		// the measured content is the floor: the user can grow the window
		// but never shrink content into clipping
		const Size floor = session->minClient();
		dialog->SetMinClientSize(wxSize(floor.width, floor.height));
	}

	session->engine->render(*session->root, content);

	// invalidation wiring -------------------------------------------------
	session->bindInvalidation(*session->root);

	// user resize (Resizable only): arrange-only within the new client area
	dialog->Bind(wxEVT_SIZE, [session](wxSizeEvent& event) {
		session->rearrange();
		event.Skip();
	});
	// display metrics changed: full re-measure (the "native sizer long
	// tail" from architecture Decision 3)
#if wxCHECK_VERSION(3, 1, 3)
	dialog->Bind(wxEVT_DPI_CHANGED, [session](wxDPIChangedEvent& event) {
		session->relayout();
		event.Skip();
	});
#endif
	dialog->Bind(wxEVT_SYS_COLOUR_CHANGED, [session](wxSysColourChangedEvent& event) {
		session->relayout();
		event.Skip();
	});
	dialog->Bind(wxEVT_DESTROY, [session, dialog](wxWindowDestroyEvent& event) {
		if (event.GetWindow() == dialog)
			delete session;
		event.Skip();
	});

	dialog->Show();
}
