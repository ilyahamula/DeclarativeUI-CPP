#include "frameworks_core/DialogWrapper.hpp"
#include "frameworks_core/ControlWrappers.hpp"

#include "frameworks_core/LayoutEngine.hpp"
#include "frameworks_core/LayoutNode.hpp"
#include "frameworks_core/wx/LayoutBackend.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#include <wx/wx.h>

#include <algorithm>

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

namespace
{

// Retained engine state for one shown dialog: the node tree (owning the
// wrappers), the adapter, and the engine. The dialog is non-modal, so this
// lives on the heap until the window is destroyed.
struct EngineSession
{
	wxDialog* dialog = nullptr;
	std::unique_ptr<WxLayoutBackend> backend;
	std::unique_ptr<LayoutEngine> engine;
	std::unique_ptr<LayoutNode> root;
	bool autoFit = true;
	bool resizable = false;
	Size fixedContent { -1, -1 };
	bool busy = false; // re-entrancy guard: SetClientSize fires wxEVT_SIZE

	Size minClient() const
	{
		const EdgeInsets margin = root->flags.border();
		return { root->desired.width + margin.left + margin.right,
				 root->desired.height + margin.top + margin.bottom };
	}

	// Window size changed, measures still valid: arrange-only from the
	// cached desired sizes.
	void rearrange()
	{
		if (busy || dialog->IsBeingDeleted())
			return;
		busy = true;
		const wxSize client = dialog->GetClientSize();
		engine->render(*root, { client.x, client.y });
		busy = false;
	}

	// Content or display metrics changed: full re-measure. Auto-fit windows
	// follow their content; resizable ones keep the user's size but never
	// below the new content floor.
	void relayout()
	{
		if (busy || dialog->IsBeingDeleted())
			return;
		busy = true;

		const Size content = engine->resolve(*root, autoFit ? Size { -1, -1 } : fixedContent);
		if (autoFit && resizable)
		{
			const Size floor = minClient();
			dialog->SetMinClientSize(wxSize(floor.width, floor.height));
			wxSize client = dialog->GetClientSize();
			if (client.x < floor.width || client.y < floor.height)
			{
				dialog->SetClientSize(std::max(client.x, floor.width),
					std::max(client.y, floor.height));
				client = dialog->GetClientSize();
			}
			engine->render(*root, { client.x, client.y });
		}
		else if (autoFit)
		{
			dialog->SetClientSize(content.width, content.height);
			engine->render(*root, content);
		}
		else
		{
			const wxSize client = dialog->GetClientSize();
			engine->render(*root, { client.x, client.y });
		}

		busy = false;
	}

	// AutoGrow text fields re-measure their live content as it changes.
	void bindAutoGrow(LayoutNode& node)
	{
		if (node.isLeaf())
		{
			if (node.flags.autoGrow() && node.widget != nullptr)
			{
				if (auto* window = static_cast<wxWindow*>(node.widget->nativeHandle()))
				{
					window->Bind(wxEVT_TEXT, [this](wxCommandEvent& event) {
						// after the control has applied the edit
						dialog->CallAfter([this]() { relayout(); });
						event.Skip();
					});
				}
			}
			return;
		}
		for (auto& child : node.children)
			bindAutoGrow(*child);
	}
};

} // unnamed namespace

void DialogWrapper::runLayoutEngine(const std::string& title, const Size& size,
	std::unique_ptr<LayoutNode> root, bool resizable)
{
	long style = wxDEFAULT_DIALOG_STYLE; // not user-resizable by default
	if (resizable)
		style |= wxRESIZE_BORDER;
	auto* dialog = new wxDialog(nullptr, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, style);

	auto* session = new EngineSession;
	session->dialog = dialog;
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
	session->bindAutoGrow(*session->root);

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
