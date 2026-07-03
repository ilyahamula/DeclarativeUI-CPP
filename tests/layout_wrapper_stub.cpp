// Backend-free stand-ins for the LayoutWrapper methods that
// ControlWrapper's (inline) vtable references. The real definitions live in
// each backend; the engine tests never lay out through a LayoutWrapper, but
// destroying tree-owned ControlWrappers ODR-uses the class.
#include "frameworks_core/ControlWrapper.hpp"
#include "frameworks_core/LayoutFlags.hpp"
#include "frameworks_core/LayoutWrapper.hpp"

void LayoutWrapper::add(LayoutFlags&) {}
void LayoutWrapper::add(LayoutWrapper*, LayoutFlags&) {}
void LayoutWrapper::add(ControlWrapper*, LayoutFlags&) {}
