#include "frameworks_core/wx/LayoutBackend.hpp"
#include "frameworks_core/wx/MenuBuilder.hpp"

#include "frameworks_core/LayoutNode.hpp"
#include "frameworks_core/wx/RefSync.hpp"
#ifdef __WXOSX__
#include "frameworks_core/wx/OsxButtonBezel.hpp"
#endif

#include <wx/notebook.h>
#include <wx/scrolwin.h>
#include <wx/settings.h>
#include <wx/tooltip.h>
#include <wx/wx.h>

#include <algorithm>

namespace
{

// Floor for editable fields when empty: N average characters wide.
constexpr int kTextFloorChars = 10;
// Allowance for the text frame's own padding around the text extent.
constexpr int kTextFramePadding = 16;

// wx treats '&' as a mnemonic marker in labels; user text must escape it.
wxString labelText(const std::string& label)
{
	wxString text = wxString::FromUTF8(label);
	text.Replace(wxT("&"), wxT("&&"));
	return text;
}

// Applies a node's effective disabled state to the window that was just
// created for it (leaf control, group-box chrome, notebook or page), and keeps
// polling when some flag in the chain is caller-owned. Hooked at the one place
// each window passes through exactly once, rather than repeated in realize().
void applyDisabled(wxWindow* window, const LayoutNode& node)
{
	// IsThisEnabled() reads the window's own flag, so a disabled parent (a
	// notebook over its pages) never provokes a re-Enable of a child.
	window->Enable(!node.isDisabledEffective());

	// A bound flag can flip at any time and wx applies the state only here, at
	// creation -- so poll it like any other external ref. A chain that is
	// fixed, or holds no bound flag at all, can never change again.
	const auto sources = node.disabledSources();
	if (sources.fixed || sources.refs.empty())
		return;

	bindExternalRefSync(window,
		[window] { return window->IsThisEnabled(); },
		[refs = sources.refs] {
			return std::none_of(refs.begin(), refs.end(),
				[](const bool* flag) { return *flag; });
		},
		[window](bool enable) { window->Enable(enable); });
}

// Applies a leaf's tooltip to the control just created for it, and keeps
// polling when the text is caller-owned. No disabled check is needed: wx does
// not deliver tooltip events to a disabled window, which is what Qt does too
// and what the ImGui backend reproduces by hand.
void applyTooltip(wxWindow* window, const ControlWrapper& widget)
{
	auto push = [window](const std::string& text) {
		if (text.empty())
			window->UnsetToolTip();
		else
			window->SetToolTip(wxString::FromUTF8(text));
	};
	push(widget.tooltip());

	// The string is caller-owned and outlives every window, so it may be
	// captured; the wrapper it came from must never be.
	const std::string* bound = widget.boundTooltip();
	if (bound == nullptr)
		return; // a snapshot cannot change behind us

	bindExternalRefSync(window,
		[window] {
			const wxToolTip* tip = window->GetToolTip();
			return tip != nullptr ? std::string(tip->GetTip().ToUTF8()) : std::string();
		},
		[bound] { return *bound; },
		push);
}

// Gives a leaf its right-click menu. The model is COPIED into the handler: the
// wrapper that supplied it must never be captured (its teardown order against
// the native window is not fixed), and the copy is what an unbound check item's
// state then lives in. A bound one writes through to the caller's bool.
//
// Nothing is polled. The menu is rebuilt from the model every time it opens, so
// bound check and disabled flags are read at that moment -- the opposite of the
// menu bar, which is built once and has to poll.
//
// No disabled check is needed here: wx sends no context-menu event to a disabled
// window, exactly as it sends no tooltip event. Qt behaves the same and the
// ImGui backend reproduces it by hand.
void applyContextMenu(wxWindow* window, const ControlWrapper& widget)
{
	if (widget.contextMenu().empty())
		return;

	window->Bind(wxEVT_CONTEXT_MENU,
		[window, items = widget.contextMenu()](wxContextMenuEvent& event) mutable {
			// wxEVT_CONTEXT_MENU carries SCREEN coordinates, except for the
			// keyboard-driven menu key, which carries wxDefaultPosition.
			const wxPoint screen = event.GetPosition();
			const wxPoint local = screen == wxDefaultPosition
				? wxPoint(window->GetSize().x / 2, window->GetSize().y / 2)
				: window->ScreenToClient(screen);
			popupContextMenu(window, items, local);
		});
}

#ifdef __WXOSX__
// The height measure() reported for a button: withSize() when set, else the
// native best size (buttons carry no content floor). A frame taller than
// that is the engine stretching the node, not the caller asking for it.
int buttonMeasuredHeight(const wxWindow* button, const ControlWrapper& widget)
{
	const int explicitHeight = widget.explicitSize().height;
	return explicitHeight > 0 ? explicitHeight : button->GetBestSize().y;
}
#endif

// The content half of an Expander: the child that folds away, as opposed to
// the header leaf beside it. Read from the node's own parent rather than from
// the scope stack, so it holds wherever the subtree is entered from.
bool isExpanderContent(const LayoutNode& node)
{
	return node.parent != nullptr && node.parent->kind == NodeKind::Expander;
}

} // unnamed namespace

WxLayoutBackend::WxLayoutBackend(wxWindow* host)
	: m_host(host)
{
	m_stack.push_back({ nullptr, host, 0, 0 });
}

Rect WxLayoutBackend::toLocal(const Rect& frame) const
{
	const Scope& scope = m_stack.back();
	Rect local { frame.x - scope.originX, frame.y - scope.originY, frame.width, frame.height };
	// Inside a scroll panel the engine's coordinates are the panel's LOGICAL
	// (unscrolled) space, while SetSize positions a child physically. wx moves
	// the children itself as the user scrolls, so taking the offset off here is
	// what keeps a relayout mid-scroll landing where the user left the content
	// instead of snapping back to the top.
	if (auto* scrolled = wxDynamicCast(scope.parent, wxScrolledWindow))
		scrolled->CalcScrolledPosition(local.x, local.y, &local.x, &local.y);
	return local;
}

Size WxLayoutBackend::measure(const LayoutNode& leaf, const Constraints& c)
{
	ControlWrapper* widget = leaf.widget;
	if (widget->nativeHandle() == nullptr)
	{
		widget->realize(m_host); // create the native control + bind events
		// A windowless leaf (Spacer) creates nothing: there is no window to
		// carry the disabled state or the tooltip, and none to hang their
		// polling handlers on. Its realize() runs again on the next pass,
		// which is why creating nothing has to stay idempotent.
		if (auto* created = static_cast<wxWindow*>(widget->nativeHandle()))
		{
			applyDisabled(created, leaf);
			applyTooltip(created, *widget);
			applyContextMenu(created, *widget);
		}
	}

	auto* window = static_cast<wxWindow*>(widget->nativeHandle());
	// Windowless: the wrapper's own measurement is the whole story. It already
	// applies the explicit withSize() overrides, so nothing below is missed --
	// the leaf is pure geometry the engine positions and wx never draws.
	if (window == nullptr)
		return widget->measureContent(c);

	const wxSize best = window->GetBestSize();
	Size size { best.x, best.y };

	// content-based floor for text fields: at least the text content (or
	// kTextFloorChars average characters when empty) plus frame padding.
	// Non-AutoGrow fields sample initial content only (cached first result);
	// AutoGrow fields re-measure the live value on every pass.
	if (auto* text = wxDynamicCast(window, wxTextCtrl))
	{
		auto cached = m_textFloorWidths.find(&leaf);
		if (leaf.flags.autoGrow() || cached == m_textFloorWidths.end())
		{
			const wxString firstLine = text->GetValue().BeforeFirst('\n');
			const wxString probe = firstLine.empty()
				? wxString(wxT('M'), kTextFloorChars)
				: firstLine;
			const int floor = text->GetTextExtent(probe).x + kTextFramePadding;
			cached = m_textFloorWidths.insert_or_assign(&leaf, floor).first;
		}
		size.width = std::max(size.width, cached->second);
		if (text->IsMultiLine())
			size.height = std::max(size.height, text->GetCharHeight() * 4 + kTextFramePadding);
	}

	// explicit withSize() dimensions override the intrinsic size per axis
	const Size& explicitSize = widget->explicitSize();
	if (explicitSize.width > 0)
		size.width = explicitSize.width;
	if (explicitSize.height > 0)
		size.height = explicitSize.height;
	return size;
}

void WxLayoutBackend::place(const LayoutNode& leaf, const Rect& frame)
{
	auto* window = static_cast<wxWindow*>(leaf.widget->nativeHandle());
	if (window == nullptr)
		return;
	if (window->GetParent() != currentParent())
		window->Reparent(currentParent());
	const Rect local = toLocal(frame);
#ifdef __WXOSX__
	// The rounded Cocoa bezel cannot grow in height; switch before SetSize,
	// since wx re-derives the frame from the bezel's insets on that call.
	if (wxDynamicCast(window, wxButton) != nullptr
		&& local.height > buttonMeasuredHeight(window, *leaf.widget))
		wxOsxAllowTallButton(window);
#endif
	window->SetSize(local.x, local.y, local.width, local.height);
	// After the move, so a wrapper that composes its content against the frame
	// is composing against the frame it actually got.
	leaf.widget->placed(local);
}

wxWindow* WxLayoutBackend::ensureContainer(const LayoutNode& node)
{
	if (const auto it = m_containers.find(&node); it != m_containers.end())
		return it->second;

	// created against the host first; beginContainer reparents as needed
	wxWindow* window = nullptr;
	if (node.kind == NodeKind::GroupBox)
		window = new wxStaticBox(m_host, wxID_ANY, labelText(node.label));
	else if (node.kind == NodeKind::TabPanel)
		window = new wxNotebook(m_host, wxID_ANY);
	else if (node.kind == NodeKind::ScrollPanel)
	{
		auto* scrolled = new wxScrolledWindow(m_host, wxID_ANY,
			wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
		// Scroll in single pixels: the engine already decided every rectangle,
		// so wx must not round the view to a "line" of its own choosing.
		scrolled->SetScrollRate(1, 1);
		window = scrolled;
	}
	else if (isExpanderContent(node))
	{
		// A collapsed section has to take its whole subtree out of view, and a
		// panel is the one handle that does it in a single call however deep
		// that subtree runs -- the same trick a notebook page relies on.
		window = new wxPanel(m_host, wxID_ANY);
	}
	if (window != nullptr)
		applyDisabled(window, node);
	m_containers[&node] = window;
	return window;
}

EdgeInsets WxLayoutBackend::containerInsets(const LayoutNode& node)
{
	if (node.kind == NodeKind::GroupBox)
	{
		auto* box = static_cast<wxStaticBox*>(ensureContainer(node));
		int top = 0;
		int other = 0;
		box->GetBordersForSizer(&top, &other);
		return { other, other, top, other };
	}
	if (node.kind == NodeKind::TabPanel)
	{
		auto* notebook = static_cast<wxNotebook*>(ensureContainer(node));
		const wxSize probe(100, 100);
		const wxSize full = notebook->CalcSizeFromPage(probe);
		const int extraW = std::max(0, full.x - probe.x);
		const int extraH = std::max(0, full.y - probe.y);
		// the tab bar takes everything the side borders don't account for
		const int side = extraW / 2;
		return { side, extraW - side, std::max(0, extraH - side), side };
	}
	if (node.kind == NodeKind::ScrollPanel)
	{
		// The scrollbar gutter, reserved on the cross side of each scrolling
		// axis and reserved ALWAYS: containerInsets runs before the content is
		// measured, so "only when it overflows" could read no more than the
		// previous pass and would let a panel near the boundary oscillate.
		wxWindow* window = ensureContainer(node);
		return {
			0,
			scrollsVertically(node.scroll)
				? wxSystemSettings::GetMetric(wxSYS_VSCROLL_X, window) : 0,
			0,
			scrollsHorizontally(node.scroll)
				? wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y, window) : 0,
		};
	}
	return {};
}

bool WxLayoutBackend::beginContainer(const LayoutNode& node, const Rect& frame)
{
	Scope scope { &node, currentParent(), m_stack.back().originX, m_stack.back().originY };
	const bool isTabPage = m_stack.back().node != nullptr
		&& m_stack.back().node->kind == NodeKind::TabPanel;

	if (isExpanderContent(node))
	{
		// The panel becomes the parent and coordinate origin of the section's
		// subtree, exactly as a notebook page does -- so hiding it hides
		// everything in it, whatever was realized while the section was open.
		wxWindow* panel = ensureContainer(node);
		if (panel->GetParent() != scope.parent)
			panel->Reparent(scope.parent);
		const bool expanded = node.parent->expander.applied;
		panel->Show(expanded);
		if (!expanded)
			return false; // collapsed: engine skips the subtree, as for an inactive tab page

		const Rect local = toLocal(frame);
		panel->SetSize(local.x, local.y, local.width, local.height);
		scope.parent = panel;
		scope.originX = frame.x;
		scope.originY = frame.y;
		m_stack.push_back(scope);
		return true;
	}

	if (node.kind == NodeKind::GroupBox || node.kind == NodeKind::TabPanel
		|| node.kind == NodeKind::ScrollPanel)
	{
		wxWindow* window = ensureContainer(node);
		if (window->GetParent() != scope.parent)
			window->Reparent(scope.parent);
		const Rect local = toLocal(frame);
		window->SetSize(local.x, local.y, local.width, local.height);
		if (node.kind == NodeKind::GroupBox)
			window->Lower(); // chrome stays behind its sibling content

		if (node.kind == NodeKind::ScrollPanel)
		{
			// The panel becomes the parent and coordinate origin of its
			// subtree, exactly as a notebook page does. The virtual size is the
			// content's arranged frame -- the engine already sized it to the
			// full extent there is to scroll over.
			auto* scrolled = static_cast<wxScrolledWindow*>(window);
			const Rect virt = node.children.empty() ? Rect{} : node.children.front()->frame;
			scrolled->SetVirtualSize(virt.width, virt.height);
			scrolled->ShowScrollbars(
				scrollsHorizontally(node.scroll) ? wxSHOW_SB_ALWAYS : wxSHOW_SB_NEVER,
				scrollsVertically(node.scroll) ? wxSHOW_SB_ALWAYS : wxSHOW_SB_NEVER);
			scope.parent = scrolled;
			scope.originX = frame.x;
			scope.originY = frame.y;
		}
	}
	else if (isTabPage)
	{
		// a page: a wxPanel owned by the notebook becomes the parent and
		// coordinate origin of the page's subtree; the notebook manages
		// which page is visible, so every page is still traversed
		auto* notebook = static_cast<wxNotebook*>(m_containers[m_stack.back().node]);
		wxWindow* page = nullptr;
		if (const auto it = m_containers.find(&node); it != m_containers.end())
		{
			page = it->second;
		}
		else
		{
			page = new wxPanel(notebook);
			notebook->AddPage(page, labelText(node.label));
			applyDisabled(page, node);
			m_containers[&node] = page;
		}
		scope.parent = page;
		scope.originX = frame.x;
		scope.originY = frame.y;
	}

	m_stack.push_back(scope);
	return true;
}

void WxLayoutBackend::endContainer(const LayoutNode&)
{
	m_stack.pop_back();
}
