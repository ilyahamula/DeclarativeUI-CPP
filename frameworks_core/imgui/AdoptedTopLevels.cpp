#include "frameworks_core/imgui/AdoptedTopLevels.hpp"
#include "frameworks_core/TopLevelShow.hpp"

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "imgui.h"

namespace
{

struct Adopted
{
	std::string title;
	std::string owner;            // the top-level whose frame issued the show()
	TopLevelShow::Present present;
	bool* callerOpen = nullptr;   // show(bool&): the caller's flag
	bool ownOpen = true;          // show(): the registry is the flag's home
	bool retired = false;         // closed for good, or replaced by a newer adopt()
	int lastDrawnFrame = -1;

	bool* flag() { return callerOpen != nullptr ? callerOpen : &ownOpen; }
};

struct Registry
{
	// Titles of the top-levels mid-frame, innermost last. Non-empty is what
	// "issued from a frame" means.
	std::vector<std::string> scopes;

	// Last frame each top-level drained its adoptees, which is what "the owner
	// has stopped drawing" is measured against.
	std::unordered_map<std::string, int> lastDrained;

	// shared_ptr so a drain can hold an entry across a present() that adopts
	// more windows and so reallocates the vector under it.
	std::vector<std::shared_ptr<Adopted>> entries;

	// Retired entries are erased only when no drain is walking the vector:
	// drains nest (an adoptee drains its own adoptees) and they walk by index.
	int drainDepth = 0;
};

Registry& registry()
{
	static Registry r;
	return r;
}

bool isMidFrame(const Registry& r, const std::string& title)
{
	return std::find(r.scopes.begin(), r.scopes.end(), title) != r.scopes.end();
}

} // unnamed namespace

bool TopLevelShow::issuedFromFrame()
{
	return !registry().scopes.empty();
}

void TopLevelShow::adopt(const std::string& title, bool* open, Present present)
{
	Registry& r = registry();
	// ImGui has one window per title, so a second show() of the same title is
	// the SAME window: the newer declaration replaces the older one. The old
	// entry is only retired, not erased -- a drain may be walking the vector.
	for (auto& entry : r.entries)
	{
		if (entry->title == title)
			entry->retired = true;
	}

	auto entry = std::make_shared<Adopted>();
	entry->title = title;
	entry->owner = r.scopes.back();
	entry->present = std::move(present);
	entry->callerOpen = open;
	r.entries.push_back(std::move(entry));
}

AdoptedTopLevels::FrameScope::FrameScope(const std::string& title)
{
	registry().scopes.push_back(title);
}

AdoptedTopLevels::FrameScope::~FrameScope()
{
	registry().scopes.pop_back();
}

void AdoptedTopLevels::drawAdopted()
{
	Registry& r = registry();
	const std::string me = r.scopes.back();
	const int frame = ImGui::GetFrameCount();
	r.lastDrained[me] = frame;

	++r.drainDepth;
	// By index and re-reading size(): a present() below may adopt further
	// windows, which appends (and may reallocate) while this loop runs.
	for (std::size_t i = 0; i < r.entries.size(); ++i)
	{
		const std::shared_ptr<Adopted> entry = r.entries[i];
		if (entry->retired || entry->lastDrawnFrame == frame)
			continue;
		// An ancestor mid-frame is already being drawn -- a cycle of handlers
		// (A opens B, B opens A) must not submit A inside itself.
		if (isMidFrame(r, entry->title))
			continue;

		if (entry->owner != me)
		{
			// Someone else's, unless its owner has stopped drawing. One frame
			// of grace: the owner may simply come later in this frame.
			const auto it = r.lastDrained.find(entry->owner);
			const bool ownerGone = it == r.lastDrained.end() || frame - it->second > 1;
			if (!ownerGone)
				continue;
			entry->owner = me;
		}

		entry->lastDrawnFrame = frame;
		bool* flag = entry->flag();
		// Already closed on entry: this present() is the cleanup call -- the one
		// that pops a modal's popup and fires onClose() -- and the last.
		const bool closing = !*flag;
		// A copy, so a handler that re-adopts this very title mid-call cannot
		// destroy the function that is running.
		const TopLevelShow::Present present = entry->present;
		present(flag);
		if (closing)
			entry->retired = true;
	}

	if (--r.drainDepth == 0)
	{
		std::erase_if(r.entries, [](const std::shared_ptr<Adopted>& e) { return e->retired; });
	}
}
