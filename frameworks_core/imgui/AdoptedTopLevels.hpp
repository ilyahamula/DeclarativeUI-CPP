#pragma once

#include <string>

// ImGui's half of TopLevelShow (frameworks_core/TopLevelShow.hpp): the registry
// of Dialogs and Windows a handler showed, and the two hooks the top-level
// wrappers call to drive it.
namespace AdoptedTopLevels
{

// Marks one top-level's runLayoutEngine() as in progress. Held for the WHOLE
// call -- including the closed early-return, whose onClose() is a handler too --
// because that span is exactly where a show() cannot be the caller's frame.
// Nests: an adopted window drawn inside its owner opens a scope of its own.
class FrameScope
{
public:
	explicit FrameScope(const std::string& title);
	~FrameScope();

	FrameScope(const FrameScope&) = delete;
	FrameScope& operator=(const FrameScope&) = delete;
};

// Draw every adopted window this top-level owns. Called by the innermost
// FrameScope's window from inside its Begin/End, after engine.render() -- the
// same point FileBrowser::drawPending() uses, and for the same reason: outside
// every BeginGroup/BeginDisabled that place() wraps a render in, but inside the
// window's ID scope, which is what lets a Modal() child's BeginPopupModal nest
// under a modal parent instead of closing it.
//
// An adopted window whose owner has stopped drawing (closed, collapsed, or no
// longer shown by the caller) is taken over by whichever window drains next --
// on wx and Qt closing a parent does not close what its handler opened either.
void drawAdopted();

} // namespace AdoptedTopLevels
