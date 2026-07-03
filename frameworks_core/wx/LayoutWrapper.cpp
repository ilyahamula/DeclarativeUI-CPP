#include "frameworks_core/LayoutWrapper.hpp"
#include "frameworks_core/LayoutFlags.hpp"
#include "frameworks_core/ControlWrapper.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#include <algorithm>

#ifdef USE_LOGGER
std::string LayoutWrapper::indent()
{
	return std::string(s_depth, '\t');
}
#endif

namespace
{

// Temporary shim while the wx backend still lays out through wxSizer: the
// pure-data LayoutFlags are translated at the Add() call sites. Deleted in
// Phase 4 together with the sizer itself (tasks.md, T4.2). wxSizerFlags holds
// a single border width, so differing per-side widths collapse to their max.
wxSizerFlags toWxSizerFlags(const LayoutFlags& flags)
{
	wxSizerFlags wxFlags(flags.proportion());

	// Applied independently, exactly as the pre-refactor code forwarded each
	// builder call to wxSizerFlags — combinations like Expand+CenterVertical
	// must keep resolving the way wx resolved them before.
	if (flags.expand())
		wxFlags.Expand();
	if (flags.centerVertical())
		wxFlags.CentreVertical();
	if (flags.centerHorizontal())
		wxFlags.CentreHorizontal();

	const EdgeInsets border = flags.border();
	int direction = 0;
	if (border.left > 0)   direction |= wxLEFT;
	if (border.right > 0)  direction |= wxRIGHT;
	if (border.top > 0)    direction |= wxTOP;
	if (border.bottom > 0) direction |= wxBOTTOM;
	if (direction != 0)
		wxFlags.Border(direction, std::max({ border.left, border.right, border.top, border.bottom }));

	return wxFlags;
}

} // unnamed namespace

LayoutWrapper::~LayoutWrapper() = default;

LayoutWrapper::LayoutWrapper(Orientation orient)
	: m_orientation(orient)
{
#ifdef USE_LOGGER
	Logger::instance().log(indent() + (orient == Orientation::Horizontal
		? "LayoutWrapper::LayoutWrapper() Horizontal\t-> new wxBoxSizer()\n"
		: "LayoutWrapper::LayoutWrapper() Vertical\t-> new wxBoxSizer()\n"));
	++s_depth;
#endif
	m_nativeSizer = new wxBoxSizer(orient == Orientation::Horizontal ? wxHORIZONTAL : wxVERTICAL);
}

LayoutWrapper::LayoutWrapper(wxSizer* sizer)
	: m_nativeSizer(sizer)
{
}

void LayoutWrapper::add(LayoutFlags& flags)
{
}

void LayoutWrapper::add(LayoutWrapper* stack, LayoutFlags& flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(indent() + "LayoutWrapper::add(stack)\t-> wxSizer->Add()\n");
#endif
	m_nativeSizer->Add(stack->nativeHandle(), toWxSizerFlags(flags));
}

void LayoutWrapper::add(ControlWrapper* widget, LayoutFlags& flags)
{
	m_nativeSizer->Add(reinterpret_cast<wxWindow*>(widget->nativeHandle()), toWxSizerFlags(flags));
}

void LayoutWrapper::finilizeLayout()
{
#ifdef USE_LOGGER
	--s_depth;
#endif
}

wxSizer* LayoutWrapper::nativeHandle() const
{
	return m_nativeSizer;
}
