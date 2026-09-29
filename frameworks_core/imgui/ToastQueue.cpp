#include "frameworks_core/imgui/ToastQueue.hpp"
#include "frameworks_core/ToastWrapper.hpp"

#include "imgui.h"

#include <algorithm>
#include <vector>

namespace
{

// The geometry every backend's toast shares, so the three look alike.
constexpr float kMargin = 16.0f;       // from the viewport's work-area corner
constexpr float kSpacing = 8.0f;       // between stacked toasts
constexpr float kPadX = 12.0f;
constexpr float kPadY = 10.0f;
constexpr float kAccentW = 4.0f;       // the style-coloured bar on the left
constexpr float kMaxTextW = 360.0f;    // longer messages wrap
constexpr float kRounding = 6.0f;

struct LiveToast
{
	std::string message;
	MessageBoxStyle style = MessageBoxStyle::Info;
	int durationMs = ToastWrapper::kDefaultDurationMs;
	double start = -1.0; // ImGui::GetTime() of the first frame that drew it
};

std::vector<LiveToast>& liveToasts()
{
	static std::vector<LiveToast> toasts;
	return toasts;
}

ImVec4 accentColor(MessageBoxStyle style)
{
	switch (style)
	{
	case MessageBoxStyle::Warning:  return ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
	case MessageBoxStyle::Error:    return ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
	case MessageBoxStyle::Question: return ImVec4(0.4f, 0.8f, 0.4f, 1.0f);
	case MessageBoxStyle::Info:     break;
	}
	return ImVec4(0.2f, 0.6f, 1.0f, 1.0f);
}

} // unnamed namespace

namespace ToastQueue
{

void push(const std::string& message, MessageBoxStyle style, int durationMs)
{
	LiveToast toast;
	toast.message = message;
	toast.style = style;
	toast.durationMs = durationMs > 0 ? durationMs : ToastWrapper::kDefaultDurationMs;
	liveToasts().push_back(std::move(toast));
}

void draw()
{
	static int lastDrawnFrame = -1;
	const int frame = ImGui::GetFrameCount();
	if (frame == lastDrawnFrame)
		return;
	lastDrawnFrame = frame;

	auto& toasts = liveToasts();
	if (toasts.empty())
		return;

	const double now = ImGui::GetTime();
	for (LiveToast& toast : toasts)
	{
		if (toast.start < 0.0)
			toast.start = now;
	}
	std::erase_if(toasts, [now](const LiveToast& toast) {
		return (now - toast.start) * 1000.0 >= toast.durationMs;
	});

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	const ImVec2 corner(viewport->WorkPos.x + viewport->WorkSize.x - kMargin,
		viewport->WorkPos.y + viewport->WorkSize.y - kMargin);
	ImDrawList* drawList = ImGui::GetForegroundDrawList();
	ImFont* font = ImGui::GetFont();
	const float fontSize = ImGui::GetFontSize();

	// The oldest toast sits in the corner and later ones stack upwards, so an
	// expiring one lets the rest slide down rather than jump sideways.
	float bottom = corner.y;
	for (const LiveToast& toast : toasts)
	{
		const double remainingMs = toast.durationMs - (now - toast.start) * 1000.0;
		const float alpha = (float)std::clamp(remainingMs / ToastWrapper::kToastFadeMs, 0.0, 1.0);

		const ImVec2 text = ImGui::CalcTextSize(toast.message.c_str(), nullptr, false, kMaxTextW);
		const ImVec2 size(kAccentW + kPadX * 2.0f + text.x, kPadY * 2.0f + text.y);
		const ImVec2 min(corner.x - size.x, bottom - size.y);
		const ImVec2 max(corner.x, bottom);

		drawList->AddRectFilled(min, max, ImGui::GetColorU32(ImGuiCol_PopupBg, alpha), kRounding);
		ImVec4 accent = accentColor(toast.style);
		accent.w *= alpha;
		drawList->AddRectFilled(min, ImVec2(min.x + kAccentW, max.y),
			ImGui::GetColorU32(accent), kRounding, ImDrawFlags_RoundCornersLeft);
		drawList->AddRect(min, max, ImGui::GetColorU32(ImGuiCol_Border, alpha), kRounding);
		drawList->AddText(font, fontSize, ImVec2(min.x + kAccentW + kPadX, min.y + kPadY),
			ImGui::GetColorU32(ImGuiCol_Text, alpha), toast.message.c_str(), nullptr, kMaxTextW);

		bottom = min.y - kSpacing;
	}
}

} // namespace ToastQueue

void ToastWrapper::show(const std::string& message, MessageBoxStyle style, int durationMs)
{
	ToastQueue::push(message, style, durationMs);
}
