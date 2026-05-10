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
