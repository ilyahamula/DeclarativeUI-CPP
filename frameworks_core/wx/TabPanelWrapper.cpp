#include "frameworks_core/TabPanelWrapper.hpp"

TabPanelWrapper::TabPanelWrapper(ControlWrapper* parent)
{
	m_notebook = new wxNotebook(reinterpret_cast<wxWindow*>(parent->nativeHandle()), wxID_ANY);
}

void TabPanelWrapper::addToLayout(LayoutWrapper* parentLayout, LayoutFlags flags)
{
	m_parentLayout = parentLayout;
	m_layoutFlags = flags;
}

void TabPanelWrapper::fitAsRoot(ControlWrapper* parent)
{
	m_rootParent = parent;
}

bool TabPanelWrapper::beginTab(const std::string& label)
{
	m_currentPanel = new wxPanel(m_notebook);
	m_currentLabel = label;
	m_panelWrapper = ControlWrapper(m_currentPanel);
	return true;
}

ControlWrapper* TabPanelWrapper::tabPageParent()
{
	return &m_panelWrapper;
}

void TabPanelWrapper::endTab()
{
	m_notebook->AddPage(m_currentPanel, m_currentLabel);
}

void TabPanelWrapper::finalize()
{
	if (m_parentLayout)
	{
		ControlWrapper notebookWrapper(m_notebook);
		m_parentLayout->add(&notebookWrapper, m_layoutFlags);
	}
	else if (m_rootParent)
	{
		auto* sizer = new wxBoxSizer(wxVERTICAL);
		sizer->Add(m_notebook, 1, wxEXPAND);
		LayoutWrapper wrapperLayout(sizer);
		m_rootParent->setLayout(&wrapperLayout);
	}
}
