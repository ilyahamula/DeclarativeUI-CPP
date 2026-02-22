#pragma once

#include "ControlWrapper.hpp"
#include "LayoutWrapper.hpp"

#include <string>

class GroupBoxWrapper : public LayoutWrapper
{
public:
	GroupBoxWrapper(ControlWrapper* parent, Orientation orient, const std::string& label);
	ControlWrapper* staticBox();

	void finilizeLayout() override;

private:
	ControlWrapper m_staticBox;
};
