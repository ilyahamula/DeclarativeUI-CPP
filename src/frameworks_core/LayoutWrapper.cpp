#include "frameworks_core/LayoutWrapper.hpp"
#include "frameworks_core/LayoutFlags.hpp"
#include "frameworks_core/ControlWrapper.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#ifdef USE_LOGGER
std::string LayoutWrapper::indent()
{
	return std::string(s_depth, '\t');
}
#endif

LayoutWrapper::~LayoutWrapper() = default;

#ifdef USE_WX

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

void LayoutWrapper::add(LayoutWrapper* stack, LayoutFlags& flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(indent() + "LayoutWrapper::add(stack)\t-> wxSizer->Add()\n");
#endif
	m_nativeSizer->Add(stack->nativeHandle(), flags.wx());
}

void LayoutWrapper::add(ControlWrapper* widget, LayoutFlags& flags)
{
	m_nativeSizer->Add(widget->nativeHandle(), flags.wx());
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
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#include "imgui.h"

LayoutWrapper::LayoutWrapper(Orientation orient)
	: m_orientation(orient)
{
#ifdef USE_LOGGER
	Logger::instance().log(indent() + (m_orientation == Orientation::Horizontal
		? "LayoutWrapper::LayoutWrapper() Horizontal\t-> ImGui::BeginGroup()\n"
		: "LayoutWrapper::LayoutWrapper() Vertical\t-> ImGui::BeginGroup()\n"));
	++s_depth;
#endif
	ImGui::BeginGroup();
}

void LayoutWrapper::add(LayoutWrapper* stack, LayoutFlags& flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(indent() + (m_orientation == Orientation::Horizontal
		? "LayoutWrapper::add(stack) Horizontal\n"
		: "LayoutWrapper::add(stack) Vertical\n"));
#endif
}

void LayoutWrapper::add(ControlWrapper* widget, LayoutFlags& flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(indent() + (m_orientation == Orientation::Horizontal
		? "LayoutWrapper::add(widget) Horizontal\t-> ImGui::SameLine()\n"
		: "LayoutWrapper::add(widget) Vertical\n"));
#endif
	if (m_orientation == Orientation::Horizontal)
		ImGui::SameLine();
}

void LayoutWrapper::finilizeLayout()
{
#ifdef USE_LOGGER
	--s_depth;
	Logger::instance().log(indent() + (m_orientation == Orientation::Horizontal
		? "LayoutWrapper::finilizeLayout() Horizontal\t-> ImGui::EndGroup()\n"
		: "LayoutWrapper::finilizeLayout() Vertical\t-> ImGui::EndGroup()\n"));
#endif
	ImGui::EndGroup();
}
#endif
