#include "frameworks_core/imgui/TextureCache.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#elif defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

// The decoder's one home in the project: this cache is now the only thing that
// reads an image file, so the implementation moved here with it.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cstdint>
#include <unordered_map>

namespace
{

std::unordered_map<std::string, CachedTexture>& textures()
{
	static std::unordered_map<std::string, CachedTexture> cache;
	return cache;
}

std::unordered_map<std::string, bool>& sizeProbes()
{
	static std::unordered_map<std::string, bool> probes;
	return probes;
}

} // unnamed namespace

const CachedTexture& textureFor(const std::string& path)
{
	auto& cache = textures();
	const auto found = cache.find(path);
	if (found != cache.end())
		return found->second;

	CachedTexture entry;
	if (!path.empty())
	{
		stbi_set_flip_vertically_on_load(0);
		unsigned char* data = stbi_load(path.c_str(), &entry.width, &entry.height, nullptr, 4);
		if (data != nullptr)
		{
			GLuint texture = 0;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, entry.width, entry.height, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
			entry.id = reinterpret_cast<void*>(static_cast<std::uintptr_t>(texture));
		}
#ifdef USE_LOGGER
		else
		{
			// Logged once, not once per frame: the failure is cached below.
			Logger::instance().log("textureFor()\t-> \"" + path + "\" failed to load\n");
		}
#endif
	}

	// Inserted whether or not it loaded, so a bad path is tried exactly once.
	return cache.emplace(path, entry).first->second;
}

bool imageSizeFor(const std::string& path, int& width, int& height)
{
	if (path.empty())
		return false;

	// A texture already uploaded knows its own size; no need to touch the file.
	const auto& cache = textures();
	const auto found = cache.find(path);
	if (found != cache.end())
	{
		if (!found->second.valid())
			return false;
		width = found->second.width;
		height = found->second.height;
		return true;
	}

	int w = 0;
	int h = 0;
	if (stbi_info(path.c_str(), &w, &h, nullptr) == 0)
	{
#ifdef USE_LOGGER
		// Once per path, like the decode failure above.
		if (sizeProbes().find(path) == sizeProbes().end())
			Logger::instance().log("imageSizeFor()\t-> \"" + path + "\" could not be read\n");
#endif
		sizeProbes()[path] = false;
		return false;
	}
	sizeProbes()[path] = true;
	width = w;
	height = h;
	return true;
}
