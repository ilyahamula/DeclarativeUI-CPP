// Backend behaviour tests for ImGui, run fully headless: an ImGui context with
// a font atlas and no renderer, fed real input events frame by frame. They
// drive the real framework -- widgets, wrappers, engine, ImGui backend -- so
// they catch what the backend-free layout_tests cannot.

#include "../test_framework.hpp"

#include "declarative_ui.hpp"
#include "frameworks_core/LayoutEngine.hpp"
#include "imgui.h"
#include "imgui_internal.h"

#include <cmath>
#include <cstdio>
#include <functional>
#include <thread>
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

	// Answer it again, so no modal is left blocking the tests that follow.
	if (const ImGuiWindow* box = ImGui::GetTopMostPopupModal())
	{
		const ImGuiStyle& st = ImGui::GetStyle();
		click(box->Pos.x + st.WindowPadding.x + 20,
			box->Pos.y + box->Size.y - st.WindowPadding.y - ImGui::GetFrameHeight() * 0.5f);
	}
	frames(3);
	CHECK(ImGui::GetTopMostPopupModal() == nullptr);
	g_ui = nullptr;
	frames(3);
}

// Review finding 11: the ImGui table sorted and measured every row every frame
// and drew every row. It now clips to the visible rows and caches the order;
// this checks the behaviour survives that -- a large table sorted descending
// by header clicks selects the right ORIGINAL row from the first visible one.
TEST(imgui_large_table_sorts_clips_and_selects_by_original_index)
{
	TableRows rows;
	for (int i = 0; i < 5000; ++i)
	{
		char key[16];
		std::snprintf(key, sizeof(key), "row%04d", i);
		rows.push_back({ key, std::to_string(i % 7) });
	}
	int selected = -1;
	g_ui = [&] {
		Dialog { "BigTable",
			VStack {
				Table{ { {"Key", -1, true}, {"Mod", -1, true} }, rows, selected }
					.withVisibleRows(10)
			}
		}.setPosition({0, 0}).show();
	};
	frames(3);

	const ImGuiWindow* w = ImGui::FindWindowByName("BigTable");
	const ImGuiStyle& st = ImGui::GetStyle();
	const float rowH = ImGui::GetTextLineHeight() + st.CellPadding.y * 2.0f;
	const float top = w->Pos.y + ImGui::GetFrameHeight() + st.WindowPadding.y;
	const float x = w->Pos.x + st.WindowPadding.x + 20.0f;

	// Tristate sorting: first click ascending, second descending.
	click(x, top + rowH * 0.5f);
	frames(2);
	click(x, top + rowH * 0.5f);
	frames(2);
	click(x, top + rowH * 1.5f); // first body row
	frames(2);
	CHECK_EQ(selected, 4999);
	g_ui = nullptr;
	frames(2);
}

// Review finding 9: the ImGui date/time pickers drew their three fields at
// natural width whatever frame they were given, so a narrower frame was
// overrun and demos had to pin them at 230 px. They now fill the frame.
TEST(imgui_pickers_stay_inside_their_frame)
{
	Date date { 2026, 9, 29 };
	Time time { 12, 30, 0 };
	g_ui = [&] {
		Dialog { "Pickers",
			VStack {
				DatePicker{date}.withSize({180, -1}),
				TimePicker{time}.withSize({180, -1})
			}
		}.setPosition({0, 0}).show();
	};
	frames(3);
	const ImGuiWindow* w = ImGui::FindWindowByName("Pickers");
	CHECK(w != nullptr);
	if (w != nullptr)
		CHECK(w->ContentSize.x <= 181.0f);
	g_ui = nullptr;
	frames(2);
}

// withId(): an unbound value keyed by position moves when the tree's shape
// changes above it; one keyed by id stays. The Expander sits in a fixed-width
// left column so opening it shifts the ImGui ids of the check boxes on the
// right without moving them on screen.
TEST(imgui_with_id_keeps_unbound_state_across_a_fold)
{
	bool open = false;
	int namedReports = 0;
	bool namedLast = false;
	int unnamedReports = 0;
	bool unnamedLast = false;
	g_ui = [&] {
		Dialog { "Ids",
			HStack {
				Expander { "Section", LayoutFlags().MinSize({200, -1}).MaxSize({200, 100000}), open,
					VStack { TextCtrl{std::string("a")}, TextCtrl{std::string("b")} } },
				VStack {
					CheckBox{false, "Named"}.withId("named-box")
						.onChange([&](bool v) { ++namedReports; namedLast = v; }),
					CheckBox{false, "Unnamed"}
						.onChange([&](bool v) { ++unnamedReports; unnamedLast = v; })
				}
			}
		}.setPosition({0, 0}).show();
	};
	frames(3);
	const ImGuiWindow* w = ImGui::FindWindowByName("Ids");
	const ImGuiStyle& st = ImGui::GetStyle();
	const float h = ImGui::GetFrameHeight();
	const float x = w->Pos.x + st.WindowPadding.x + 200.0f + LayoutEngine::kDefaultGap + 6.0f;
	const float y0 = w->Pos.y + h + st.WindowPadding.y + h * 0.5f;
	const float y1 = y0 + h + LayoutEngine::kDefaultGap;

	click(x, y0);
	click(x, y1);
	CHECK(namedReports == 1 && namedLast);
	CHECK(unnamedReports == 1 && unnamedLast);

	open = true; // shifts every sequential id after the section
	frames(3);
	click(x, y0);
	click(x, y1);
	CHECK_EQ(namedReports, 2);
	CHECK(!namedLast);   // it was still ticked, so the click un-ticked it
	CHECK_EQ(unnamedReports, 2);
	CHECK(unnamedLast);  // its tick was lost with its position: ticked afresh
	g_ui = nullptr;
	frames(2);
}

// postToUi(): a task posted from another thread runs on the UI thread, in the
// next frame's first framework window.
TEST(imgui_post_to_ui_runs_on_the_ui_thread)
{
	const auto uiThread = std::this_thread::get_id();
	int value = 0;
	bool onUiThread = false;
	g_ui = [&] { Dialog { "Post", VStack { StaticText{"x"} } }.show(); };
	std::thread([&] {
		postToUi([&] { value = 42; onUiThread = std::this_thread::get_id() == uiThread; });
	}).join();
	CHECK_EQ(value, 0); // not before a frame drains it
	frames(2);
	CHECK_EQ(value, 42);
	CHECK(onUiThread);
	g_ui = nullptr;
	frames(1);
}

// A bound StaticText / ReadonlyTextCtrl follows the caller's string, but keeps
// the size of its first text: an arriving message must not resize an auto-fit
// window.
TEST(imgui_bound_labels_do_not_resize_the_window)
{
	std::string status = "Ready";
	g_ui = [&] {
		Dialog { "Status",
			VStack { StaticText{status}, ReadonlyTextCtrl{status} }
		}.setPosition({0, 0}).show();
	};
	frames(3);
	const ImGuiWindow* w = ImGui::FindWindowByName("Status");
	const float before = w->Size.x;
	status = "This status message is far longer than the label it arrives in, by design";
	frames(3);
	CHECK_EQ(w->Size.x, before);
	g_ui = nullptr;
	frames(2);
}

// isHidden(): a hidden control takes no space, so an auto-fit window shrinks
// by its height and its gap, and grows back when it is shown again.
TEST(imgui_hidden_control_shrinks_and_restores_the_window)
{
	bool hide = false;
	g_ui = [&] {
		Dialog { "Hide",
			VStack {
				Button{"Always"},
				Button{"Sometimes"}.isHidden(hide),
				Button{"Also always"}
			}
		}.setPosition({0, 0}).show();
	};
	frames(3);
	const ImGuiWindow* w = ImGui::FindWindowByName("Hide");
	const float shown = w->Size.y;
	hide = true;
	frames(3);
	const float expected = ImGui::GetFrameHeight() + LayoutEngine::kDefaultGap;
	CHECK(std::fabs((shown - w->Size.y) - expected) < 1.0f);
	hide = false;
	frames(3);
	CHECK_EQ(w->Size.y, shown);
	g_ui = nullptr;
	frames(2);
}

// Bound item lists: a ListBox on a non-const vector draws the caller's current
// items, and keeps the width it had for the first ones.
TEST(imgui_bound_list_items_follow_the_vector)
{
	ItemList items { "alpha", "beta" };
	std::string picked;
	g_ui = [&] {
		Dialog { "Items",
			VStack { ListBox{ items, picked }.withVisibleRows(4) }
		}.setPosition({0, 0}).show();
	};
	frames(3);
	const ImGuiWindow* w = ImGui::FindWindowByName("Items");
	const float width = w->Size.x;

	items = { "a much longer first item than before", "beta" };
	frames(3);
	CHECK_EQ(w->Size.x, width);

	const ImGuiStyle& st = ImGui::GetStyle();
	const float x = w->Pos.x + st.WindowPadding.x + 20.0f;
	const float y = w->Pos.y + ImGui::GetFrameHeight() + st.WindowPadding.y
		+ st.FramePadding.y + ImGui::GetTextLineHeightWithSpacing() * 0.5f;
	click(x, y);
	CHECK_EQ(picked, std::string("a much longer first item than before"));
	g_ui = nullptr;
	frames(2);
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
