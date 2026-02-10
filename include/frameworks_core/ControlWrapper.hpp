#pragma once

#include "GeneralTypes.hpp"
#include "LayoutFlags.hpp"
#include "LayoutWrapper.hpp"

#include <wx/window.h>

enum class ControlType
{
	Button,
	TextCtrl,
	StaticText,
	Slider,
	RadioButton,
	CheckBox,
	GroupBox
};

class ControlWrapper
{
public:
	ControlWrapper();
	virtual ~ControlWrapper();

	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags);
	void setLayout(LayoutWrapper* layout);

	ControlWrapper(wxWindow* nativeWidget);
	wxWindow* nativeHandle() const;

protected:
	wxWindow* m_nativeWidget = nullptr;
};
