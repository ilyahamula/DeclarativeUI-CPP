#pragma once

#include "ControlWrapper.hpp"
#include "LayoutWrapper.hpp"
#include "LayoutFlags.hpp"

#include <string>

#ifdef USE_WX
#include <wx/notebook.h>
#include <wx/panel.h>
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif

class TabPanelWrapper
{
public:
	TabPanelWrapper(ControlWrapper* parent);

	void addToLayout(LayoutWrapper* parentLayout, LayoutFlags flags);
	void fitAsRoot(ControlWrapper* parent);

	bool beginTab(const std::string& label);
	ControlWrapper* tabPageParent();
	void endTab();

	void finalize();

private:
#ifdef USE_WX
	wxNotebook* m_notebook = nullptr;
	wxPanel* m_currentPanel = nullptr;
	std::string m_currentLabel;
	ControlWrapper m_panelWrapper;
	LayoutWrapper* m_parentLayout = nullptr;
	LayoutFlags m_layoutFlags;
	ControlWrapper* m_rootParent = nullptr;
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
	ControlWrapper* m_parent = nullptr;
	bool m_tabBarOpen = false;
#endif
};
