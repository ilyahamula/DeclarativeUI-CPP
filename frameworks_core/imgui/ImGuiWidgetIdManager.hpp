#pragma once

#include "imgui.h"
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
