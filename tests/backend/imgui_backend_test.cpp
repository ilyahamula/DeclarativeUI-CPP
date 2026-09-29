// Backend behaviour tests for ImGui, run fully headless: an ImGui context with
// a font atlas and no renderer, fed real input events frame by frame. They
// drive the real framework -- widgets, wrappers, engine, ImGui backend -- so
// they catch what the backend-free layout_tests cannot.

#include "../test_framework.hpp"

#include "declarative_ui.hpp"
#include "frameworks_core/LayoutEngine.hpp"
#include "imgui.h"
#include "imgui_internal.h"

#include <functional>
#include <string>

namespace
{

std::function<void()> g_ui;

void frame(float mx = -1, float my = -1, bool down = false, unsigned typed = 0)
{
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = 1.0f / 60.0f;
	if (mx >= 0)
		io.AddMousePosEvent(mx, my);
	io.AddMouseButtonEvent(0, down);
	if (typed != 0)
		io.AddInputCharacter(typed);
	ImGui::NewFrame();
	if (g_ui)
		g_ui();
	ImGui::Render();
}

void frames(int n)
{
	for (int i = 0; i < n; ++i)
		frame();
}

void click(float x, float y)
{
	frame(x, y, false);
	frame(x, y, true);
	frame(x, y, false);
}

// Centre of the n-th row of single-line leaves stacked in a VStack that is the
// whole content of a Dialog placed at the origin: rows are one frame height
// tall and LayoutEngine::kDefaultGap apart.
ImVec2 rowCentre(const char* window, int row, float dx = 6.0f)
{
	const ImGuiWindow* w = ImGui::FindWindowByName(window);
	const ImGuiStyle& st = ImGui::GetStyle();
	const float h = ImGui::GetFrameHeight();
	return ImVec2(w->Pos.x + st.WindowPadding.x + dx,
		w->Pos.y + h + st.WindowPadding.y + h * 0.5f + (h + LayoutEngine::kDefaultGap) * (float)row);
}

} // namespace

// Review finding 1: radio groups used a process-wide counter keyed on
// declaration order, so on ImGui (rebuilt every frame) the indices grew every
// frame and a click wrote a meaningless number into the caller's int.
TEST(imgui_radio_groups_survive_rebuilds_and_share_a_box)
{
	int choice = 1;
	int other = 0;
	g_ui = [&] {
		Dialog { "Radios",
			VStack {
				RadioButton{choice, 0, "Alpha"},
				RadioButton{choice, 1, "Beta"},
				RadioButton{choice, 2, "Gamma"},
				RadioButton{other, 0, "One"},
				RadioButton{other, 1, "Two"}
			}
		}.setPosition({0, 0}).show();
	};
	frames(10);

	const ImVec2 gamma = rowCentre("Radios", 2);
	click(gamma.x, gamma.y);
	CHECK_EQ(choice, 2);
	CHECK_EQ(other, 0);

	const ImVec2 two = rowCentre("Radios", 4);
	click(two.x, two.y);
	CHECK_EQ(other, 1);
	CHECK_EQ(choice, 2);

	// a checked radio stays checked when clicked again
	click(gamma.x, gamma.y);
	CHECK_EQ(choice, 2);
	g_ui = nullptr;
}

// Review finding 4: fixed char buffers truncated long bound text and wrote the
// truncated copy back on the first keystroke.
TEST(imgui_text_field_keeps_long_bound_text)
{
	std::string text(1000, 'x');
	g_ui = [&] {
		Dialog { "LongText",
			VStack { TextCtrl{text}.withSize({200, -1}) }
		}.setPosition({0, 0}).show();
	};
	frames(3);

	const ImVec2 field = rowCentre("LongText", 0, 50.0f);
	click(field.x, field.y);
	ImGuiIO& io = ImGui::GetIO();
	io.AddKeyEvent(ImGuiKey_End, true);
	frame();
	io.AddKeyEvent(ImGuiKey_End, false);
	frame();
	frame(-1, -1, false, 'y');
	frame();
	CHECK_EQ(text.size(), static_cast<std::size_t>(1001));
	CHECK(text.back() == 'y');

	// release focus so later tests start clean
	click(700, 500);
	g_ui = nullptr;
}

// Review finding 5: a MessageBox shown once from a handler was drawn for one
// frame and then left an invisible modal blocking all input.
TEST(imgui_message_box_from_handler_stays_up_until_answered)
{
	int clicks = 0;
	int results = 0;
	MessageBoxResult last = MessageBoxResult::Cancel;
	g_ui = [&] {
		Dialog { "Host",
			VStack {
				Button{"Ask"}.onClick([&] {
					++clicks;
					MessageBox{"Box", "Proceed?"}
						.onResult([&](MessageBoxResult r) { ++results; last = r; })
						.show();
				})
			}
		}.setPosition({0, 0}).show();
	};
	frames(3);
	auto modalIsBox = [] {
		const ImGuiWindow* m = ImGui::GetTopMostPopupModal();
		return m != nullptr && std::string(m->Name).find("Box") != std::string::npos;
	};

	const ImVec2 ask = rowCentre("Host", 0, 10.0f);
	click(ask.x, ask.y);
	frames(10);
	CHECK_EQ(clicks, 1);
	CHECK(modalIsBox());

	if (const ImGuiWindow* box = ImGui::GetTopMostPopupModal())
	{
		const ImGuiStyle& st = ImGui::GetStyle();
		click(box->Pos.x + st.WindowPadding.x + 20,
			box->Pos.y + box->Size.y - st.WindowPadding.y - ImGui::GetFrameHeight() * 0.5f);
	}
	frames(3);
	CHECK_EQ(results, 1);
	CHECK(last == MessageBoxResult::OK);
	CHECK(ImGui::GetTopMostPopupModal() == nullptr);

	click(ask.x, ask.y);
	frames(3);
	CHECK_EQ(clicks, 2);
	CHECK(modalIsBox());
	g_ui = nullptr;
	frames(3);
}

int main()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(800, 600);
	io.IniFilename = nullptr;
	io.Fonts->AddFontDefault();
	unsigned char* pixels = nullptr;
	int width = 0;
	int height = 0;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
	io.Fonts->SetTexID((ImTextureID)(intptr_t)1);

	const int result = testfw::runAll();
	ImGui::DestroyContext();
	return result;
}
