#include "frameworks_core/UiThread.hpp"

#include "imgui.h"

#include <mutex>
#include <vector>

namespace
{

// ImGui has no event loop of its own to post into -- the caller owns the frame
// loop -- so tasks wait here until the next frame's first framework window.
struct Queue
{
	std::mutex mutex;
	std::vector<std::function<void()>> tasks;
	int lastDrainedFrame = -1;
};

Queue& queue()
{
	static Queue q;
	return q;
}

} // unnamed namespace

void postToUi(std::function<void()> task)
{
	if (!task)
		return;
	Queue& q = queue();
	const std::lock_guard<std::mutex> lock(q.mutex);
	q.tasks.push_back(std::move(task));
}

void UiThreadQueue::drain()
{
	Queue& q = queue();
	// Once per frame, like the toast queue: a frame with three windows drains
	// in the first of them.
	const int frame = ImGui::GetFrameCount();
	if (q.lastDrainedFrame == frame)
		return;
	q.lastDrainedFrame = frame;

	// Taken out under the lock and run outside it, so a task may post more
	// (they run next frame) without deadlocking.
	std::vector<std::function<void()>> ready;
	{
		const std::lock_guard<std::mutex> lock(q.mutex);
		ready.swap(q.tasks);
	}
	for (auto& task : ready)
		task();
}
