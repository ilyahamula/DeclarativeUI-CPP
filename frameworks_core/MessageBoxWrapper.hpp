#pragma once

#include "frameworks_core/CoreTypes/GeneralTypes.hpp"

#include <functional>
#include <string>

class MessageBoxWrapper
{
public:
	static void show(const std::string& title, const std::string& message,
		MessageBoxStyle style, MessageBoxButtons buttons,
		const std::function<void(MessageBoxResult)>& onResult);

	static void show(const std::string& title, const std::string& message,
		MessageBoxStyle style, MessageBoxButtons buttons,
		const std::function<void(MessageBoxResult)>& onResult,
		bool& visible);
};
