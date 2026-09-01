#pragma once

// Umbrella header: pulls in every demo dialog. The per-backend mains include
// this one file so adding a demo never touches main_*.cpp.

#include "controls_gallery.hpp"
#include "selection_gallery.hpp"
#include "tree_gallery.hpp"
#include "table_gallery.hpp"
#include "composite_dialogs.hpp"
#include "container_disabling.hpp"
#include "value_binding.hpp"
#include "layout_probes.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif
