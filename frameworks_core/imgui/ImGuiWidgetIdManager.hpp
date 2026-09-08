#pragma once

#include "imgui.h"
#include <cstdint>
#include <unordered_map>

// Per-ImGui-scope sequential ID manager (USE_IMGUI only).
//
// ImGui::GetID("__dui_scope__") hashes the current window's ID-stack seed,
// producing a different value for each Begin/BeginChild scope — no imgui_internal.h
// required.  Each scope gets its own counter that resets to 0 at the start of
// each frame, so IDs are always stable regardless of how many other windows or
// dialogs are open simultaneously.
class WidgetIdManager
{
public:
    // Sequential integer for ImGui::PushID() — unique within the current ImGui scope.
    static int nextWidgetId()  { return next(s_widgetIds); }

    // Sequential integer for naming BeginChild windows (GroupBoxes) —
    // unique within the current parent scope (called before BeginChild).
    static int nextGroupBoxId() { return next(s_groupBoxIds); }

    // Key for framework-owned state that has to outlive the frame the way
    // ImGui's own does (see SnapshotStore.hpp). Built from the same scope seed
    // as the widget id, so it must be taken in the scope the id was drawn from —
    // before any PushID() the wrapper makes. `slot` (0..15) separates the
    // several values of a control that carries more than one.
    static std::uint64_t stateKey(int widgetId, int slot = 0)
    {
        const std::uint64_t scope = ImGui::GetID("__dui_scope__");
        return (scope << 32)
            | (static_cast<std::uint32_t>(widgetId) << 4)
            | static_cast<std::uint32_t>(slot & 0xF);
    }

private:
    struct ScopeState { int counter = 0; int lastFrame = -1; };

    static int next(std::unordered_map<ImGuiID, ScopeState>& table)
    {
        ImGuiID key   = ImGui::GetID("__dui_scope__");
        int     frame = ImGui::GetFrameCount();
        auto&   state = table[key];
        if (state.lastFrame != frame) { state.counter = 0; state.lastFrame = frame; }
        return state.counter++;
    }

    static inline std::unordered_map<ImGuiID, ScopeState> s_widgetIds;
    static inline std::unordered_map<ImGuiID, ScopeState> s_groupBoxIds;
};
