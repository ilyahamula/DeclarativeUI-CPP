#pragma once

#include <string>

// Process-wide image cache for the ImGui backend (USE_IMGUI only).
//
// ImGui rebuilds the declarative tree every frame, so a wrapper that owned its
// own GL texture would either reload the file constantly or leak one texture per
// wrapper. Keying by path instead means an icon is decoded and uploaded exactly
// once per process however many toolbars, buttons or images use it -- which is
// the difference between a toolbar that draws and one that stutters.
//
// Shared by ImageWrapper and ToolBarWrapper.
struct CachedTexture
{
	void* id = nullptr; // ImTextureID (void*) holding the GL texture handle
	int width = 0;
	int height = 0;

	bool valid() const { return id != nullptr; }
};

// Decodes and uploads on first request, then returns the same entry forever.
// A FAILED load is cached as a failure too -- otherwise a bad path would be
// retried and re-logged on every frame of every window that mentions it.
const CachedTexture& textureFor(const std::string& path);

// The image's natural size WITHOUT decoding it (a header probe), for the
// measure pass -- which runs before ImGui::Begin and must stay cheap.
// Returns false and leaves the outputs alone if the file cannot be read.
bool imageSizeFor(const std::string& path, int& width, int& height);
