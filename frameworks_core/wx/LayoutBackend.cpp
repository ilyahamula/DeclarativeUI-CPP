#include "frameworks_core/wx/LayoutBackend.hpp"

#include "frameworks_core/LayoutNode.hpp"
#include "frameworks_core/wx/RefSync.hpp"

#include <wx/notebook.h>
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

} // unnamed namespace

WxLayoutBackend::WxLayoutBackend(wxWindow* host)
	: m_host(host)
{
	m_stack.push_back({ nullptr, host, 0, 0 });
}

Rect WxLayoutBackend::toLocal(const Rect& frame) const
{
	const Scope& scope = m_stack.back();
	return { frame.x - scope.originX, frame.y - scope.originY, frame.width, frame.height };
}

Size WxLayoutBackend::measure(const LayoutNode& leaf, const Constraints&)
{
	ControlWrapper* widget = leaf.widget;
	if (widget->nativeHandle() == nullptr)
	{
		widget->realize(m_host); // create the native control + bind events
		applyDisabled(static_cast<wxWindow*>(widget->nativeHandle()), leaf);
	}

	auto* window = static_cast<wxWindow*>(widget->nativeHandle());
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
	window->SetSize(local.x, local.y, local.width, local.height);
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
	return {};
}

bool WxLayoutBackend::beginContainer(const LayoutNode& node, const Rect& frame)
{
	Scope scope { &node, currentParent(), m_stack.back().originX, m_stack.back().originY };
	const bool isTabPage = m_stack.back().node != nullptr
		&& m_stack.back().node->kind == NodeKind::TabPanel;

	if (node.kind == NodeKind::GroupBox || node.kind == NodeKind::TabPanel)
	{
		wxWindow* window = ensureContainer(node);
		if (window->GetParent() != scope.parent)
			window->Reparent(scope.parent);
		const Rect local = toLocal(frame);
		window->SetSize(local.x, local.y, local.width, local.height);
		if (node.kind == NodeKind::GroupBox)
			window->Lower(); // chrome stays behind its sibling content
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
