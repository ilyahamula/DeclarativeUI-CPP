#include "frameworks_core/LayoutWrapper.hpp"
#include "frameworks_core/LayoutFlags.hpp"
#include "frameworks_core/ControlWrapper.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

LayoutWrapper::~LayoutWrapper() = default;

#ifdef USE_WX

LayoutWrapper::LayoutWrapper(Orientation orient)
	: m_orientation(orient)
{
#ifdef USE_LOGGER
	Logger::instance().log(orient == Orientation::Horizontal
		? "LayoutWrapper::LayoutWrapper() Horizontal -> new wxBoxSizer()\n"
		: "LayoutWrapper::LayoutWrapper() Vertical -> new wxBoxSizer()\n");
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
	Logger::instance().log("LayoutWrapper::add() -> wxSizer->Add()\n");
#endif
	m_nativeSizer->Add(stack->nativeHandle(), flags.wx());
}

void LayoutWrapper::add(ControlWrapper* widget, LayoutFlags& flags)
{
	m_nativeSizer->Add(widget->nativeHandle(), flags.wx());
}

void LayoutWrapper::finilizeLayout()
{
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
	Logger::instance().log(std::string(m_orientation == Orientation::Horizontal
		? "LayoutWrapper::LayoutWrapper() Horizontal\n"
		: "LayoutWrapper::LayoutWrapper() Vertical\n") + "ImGui::BeginGroup()\n");
#endif
	ImGui::BeginGroup();
}

void LayoutWrapper::add(LayoutWrapper* stack, LayoutFlags& flags)
{
}

void LayoutWrapper::add(ControlWrapper* widget, LayoutFlags& flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(m_orientation == Orientation::Horizontal
		? "LayoutWrapper::LayoutWrapper() Horizontal -> ImGui::SameLine()\n"
		: "LayoutWrapper::LayoutWrapper() Vertical\n");
#endif
	if (m_orientation == Orientation::Horizontal)
		ImGui::SameLine();
}

void LayoutWrapper::finilizeLayout()
{
#ifdef USE_LOGGER
	Logger::instance().log("LayoutWrapper::finilizeLayout() -> ImGui::EndGroup()\n");
#endif
	ImGui::EndGroup();
}
#endif
