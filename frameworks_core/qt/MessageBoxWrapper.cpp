#include "frameworks_core/MessageBoxWrapper.hpp"

void MessageBoxWrapper::show(const std::string&, const std::string&,
	MessageBoxStyle, MessageBoxButtons,
	const std::function<void(MessageBoxResult)>&)
{
}

void MessageBoxWrapper::show(const std::string&, const std::string&,
	MessageBoxStyle, MessageBoxButtons,
	const std::function<void(MessageBoxResult)>&,
	bool&)
{
}
