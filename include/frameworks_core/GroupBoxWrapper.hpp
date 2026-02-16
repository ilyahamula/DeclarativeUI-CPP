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

#ifdef USE_IMGUI
	static void resetId() { s_groupBoxId = 0; }
#endif

private:
	ControlWrapper m_staticBox;
#ifdef USE_IMGUI
	static inline int s_groupBoxId = 0;
#endif
};
