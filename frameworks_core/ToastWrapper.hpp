#pragma once

#include "frameworks_core/CoreTypes/GeneralTypes.hpp"

#include <string>

// A timed, non-blocking notice, on the terms MessageBoxWrapper and
// FileDialogWrapper are one-shots: a static show(), no wrapper instance, no
// node and no engine. A toast is drawn OVER the application's windows, never
// in one of them, so it never becomes a LayoutNode and never reaches
// ILayoutBackend.
//
// All three backends draw it themselves -- a framework-owned overlay at the
// bottom-right of the application, not the OS notification centre -- so it
// looks and sits the same everywhere:
//   * wx: a borderless ToastWindow shown without activation (wx/ToastWindow.hpp);
//   * Qt: a frameless ToastWindow with Qt::WindowDoesNotAcceptFocus (qt/ToastWindow.hpp);
//   * ImGui: ToastQueue (imgui/ToastQueue.hpp), drawn on the foreground draw
//     list by the next Dialog/Window of the frame -- nothing extra for the app
//     to call.
//
// None of them takes focus or input, several live toasts stack upwards rather
// than overlap, and each fades out over its last kToastFadeMs.
class ToastWrapper
{
public:
	static constexpr int kDefaultDurationMs = 2500;
	static constexpr int kToastFadeMs = 300;

	// Returns at once on every backend. `durationMs` is the whole lifetime,
	// fade included; a non-positive value means kDefaultDurationMs.
	static void show(const std::string& message, MessageBoxStyle style, int durationMs);
};
