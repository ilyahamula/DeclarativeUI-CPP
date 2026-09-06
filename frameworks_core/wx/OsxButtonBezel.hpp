#pragma once

class wxWindow;

// macOS only. wx creates a plain push button with the rounded Cocoa bezel,
// which has a fixed height: placed in a taller frame it stays standard-sized
// and centers itself, so the engine's cross-axis stretch never shows. The
// bevel bezel resizes freely, and wx itself switches a rounded button to it
// when a bitmap is set -- this does the same for a button the engine
// stretched. Idempotent; other bezels (help, textured) are left alone.
void wxOsxAllowTallButton(wxWindow* button);
