#pragma once

#include "ControlWrapper.hpp"

#include <string>

class DialogWrapper : public ControlWrapper
{
public:
	DialogWrapper(const std::string& title, const Size& size);
	void show();
};
