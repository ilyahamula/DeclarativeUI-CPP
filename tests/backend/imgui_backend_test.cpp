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
#include <algorithm>
#include <thread>
#include <vector>
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

// VForEach: rows follow a bound vector -- the window grows with an added row
// and a row's own Remove button removes it.
TEST(imgui_foreach_rows_follow_the_vector)
{
	std::vector<std::string> todos { "one", "two" };
	g_ui = [&] {
		Dialog { "Todos",
			VForEach { todos, [&todos](const std::string& todo, std::size_t i) {
				return HStack {
					StaticText{todo}.withSize({60, -1}),
					Button{"Remove"}.onClick([&todos, i] { todos.erase(todos.begin() + (long)i); })
				};
			} }
		}.setPosition({0, 0}).show();
	};
	frames(3);
	const ImGuiWindow* w = ImGui::FindWindowByName("Todos");
	const float twoRows = w->Size.y;
	const float rowStep = ImGui::GetFrameHeight() + LayoutEngine::kDefaultGap;

	todos.push_back("three");
	frames(3);
	CHECK(std::fabs(w->Size.y - (twoRows + rowStep)) < 1.0f);

	// Remove the first row with its own button.
	const ImGuiStyle& st = ImGui::GetStyle();
	click(w->Pos.x + st.WindowPadding.x + 60.0f + LayoutEngine::kDefaultGap + 10.0f,
		w->Pos.y + ImGui::GetFrameHeight() + st.WindowPadding.y + ImGui::GetFrameHeight() * 0.5f);
	frames(3);
	CHECK_EQ(todos.size(), static_cast<std::size_t>(2));
	CHECK_EQ(todos.front(), std::string("two"));
	CHECK(std::fabs(w->Size.y - twoRows) < 1.0f);
	g_ui = nullptr;
	frames(2);
}

// RadioGroup: one radio per option, bound to the index; a write from outside
// moves the selection, and onChange reports after the int is written.
TEST(imgui_radio_group_binds_an_index)
{
	int choice = 0;
	int reported = -1;
	int seen = -1;
	g_ui = [&] {
		Dialog { "Group",
			RadioGroup{choice, {"Red", "Green", "Blue"}}
				.onChange([&](int i) { reported = i; seen = choice; })
		}.setPosition({0, 0}).show();
	};
	frames(10);

	const ImVec2 blue = rowCentre("Group", 2);
	click(blue.x, blue.y);
	CHECK_EQ(choice, 2);
	CHECK_EQ(reported, 2);
	CHECK_EQ(seen, 2);

	choice = 1;
	frames(2);
	const ImVec2 red = rowCentre("Group", 0);
	click(red.x, red.y);
	CHECK_EQ(choice, 0);
	g_ui = nullptr;
}

// Default / cancel buttons: Enter in a single-line field reports onEnter and
// then presses the default button; Escape presses cancel; Enter inside a
// multi-line field is a newline; a disabled default button is skipped.
namespace
{
void key(ImGuiKey k)
{
	ImGui::GetIO().AddKeyEvent(k, true);
	frame();
	ImGui::GetIO().AddKeyEvent(k, false);
	frame();
}
} // namespace

TEST(imgui_default_and_cancel_buttons_answer_enter_and_escape)
{
	std::vector<std::string> log;
	std::string query = "cats";
	std::string notes;
	bool okDisabled = false;
	g_ui = [&] {
		Dialog { "Keys",
			VStack {
				TextCtrl{query}.withSize({200, -1})
					.onEnter([&](const std::string& t) { log.push_back("enter:" + t); }),
				Button{"OK"}.isDefault().isDisabled(okDisabled)
					.onClick([&] { log.push_back("ok"); }),
				Button{"Cancel"}.isCancel()
					.onClick([&] { log.push_back("cancel"); }),
				MultiLineTextCtrl{notes}.withSize({200, 60})
			}
		}.setPosition({0, 0}).show();
	};
	frames(10);

	const ImVec2 field = rowCentre("Keys", 0);
	click(field.x, field.y); // focuses the window and activates the field
	key(ImGuiKey_Enter);
	CHECK_EQ(log.size(), std::size_t(2));
	if (log.size() == 2)
	{
		CHECK(log[0] == "enter:cats"); // onEnter first ...
		CHECK(log[1] == "ok");         // ... then the default button
	}

	log.clear();
	key(ImGuiKey_Escape);
	CHECK_EQ(log.size(), std::size_t(1));
	CHECK(!log.empty() && log[0] == "cancel");

	// Enter while the multi-line field is active is its newline
	log.clear();
	const ImVec2 multi = rowCentre("Keys", 3);
	click(multi.x, multi.y + 10);
	key(ImGuiKey_Enter);
	CHECK(log.empty());
	CHECK(notes.find('\n') != std::string::npos);

	// a disabled default button does not answer
	const ImVec2 cancel = rowCentre("Keys", 2);
	okDisabled = true;
	click(cancel.x + 200, cancel.y); // empty space: deactivates the field, keeps focus
	log.clear();
	key(ImGuiKey_Enter);
	CHECK(log.empty());
	g_ui = nullptr;
}

// withIcon: headless there is no GL context to upload a texture into, so this
// covers the fallback half -- a path that fails to load is exactly a text
// button: same size, still clickable. The icon itself is covered on wx/Qt.
TEST(imgui_icon_button_falls_back_to_its_label)
{
	ButtonWrapper plain("Save", {}, {}, 0);
	ButtonWrapper broken("Save", {}, {}, 0, {}, kNoDialogKey, "/no/such/icon.png", { 16, 16 });
	CHECK_EQ(plain.measureIntrinsic({}).width, broken.measureIntrinsic({}).width);
	CHECK_EQ(plain.measureIntrinsic({}).height, broken.measureIntrinsic({}).height);

	int clicks = 0;
	g_ui = [&] {
		Dialog { "Icons",
			VStack {
				Button{"Save"}.withIcon("/no/such/icon.png").onClick([&] { ++clicks; })
			}
		}.setPosition({0, 0}).show();
	};
	frames(10);
	const ImVec2 save = rowCentre("Icons", 0);
	click(save.x, save.y);
	CHECK_EQ(clicks, 1);
	g_ui = nullptr;
}

// Bound TreeView items and Table rows: the tree is rebuilt every frame, so
// following the data is free -- what matters is that it does not RESIZE the
// window, the parity wx and Qt keep by measuring the first content once.
TEST(imgui_bound_tree_and_table_keep_their_first_size)
{
	std::vector<TreeItem> tree { { "Fruits", { { "Apple" }, { "Banana" } }, true } };
	std::string treePick = "Fruits/Banana";
	TableRows rows { { "a.txt", "1" }, { "b.txt", "2" } };
	std::string rowPick = "b.txt";
	g_ui = [&] {
		Dialog { "Data",
			VStack {
				TreeView{ tree, treePick }.withVisibleRows(5),
				Table{ { { "File", -1, true }, { "Size", -1 } }, rows, rowPick }.withVisibleRows(4)
			}
		}.setPosition({0, 0}).show();
	};
	frames(3);
	const ImGuiWindow* w = ImGui::FindWindowByName("Data");
	const ImVec2 size = w->Size;

	tree[0].children.push_back({ "Cherry with a very long name indeed" });
	tree.push_back({ "Nuts", { { "Almond" } }, true });
	rows.insert(rows.begin(), { "a much longer file name than any before.txt", "3" });
	frames(3);
	CHECK_EQ(w->Size.x, size.x);
	CHECK_EQ(w->Size.y, size.y);
	CHECK_EQ(treePick, std::string("Fruits/Banana"));
	CHECK_EQ(rowPick, std::string("b.txt"));

	rows.clear();
	tree.clear();
	frames(3); // an emptied table and tree draw nothing and do not crash
	CHECK_EQ(w->Size.x, size.x);
	g_ui = nullptr;
	frames(2);
}

// Escape with no cancel button closes a Dialog -- as a wxDialog and a QDialog
// do -- and leaves a Window alone, as a wxFrame and a QMainWindow do.
TEST(imgui_escape_closes_a_dialog_without_a_cancel_button)
{
	bool dialogOpen = true;
	bool windowOpen = true;
	int closed = 0;
	g_ui = [&] {
		Window { "EscWindow", VStack { Button{"Nothing"} } }.show(windowOpen);
		Dialog { "EscDialog", VStack { Button{"Nothing either"} } }
			.onClose([&] { ++closed; })
			.setPosition({0, 0})
			.show(dialogOpen);
	};
	frames(5);
	const ImVec2 inside = rowCentre("EscDialog", 0);
	click(inside.x, inside.y); // focus the dialog
	key(ImGuiKey_Escape);
	CHECK(!dialogOpen);
	CHECK_EQ(closed, 1);

	// the Window, focused, ignores it
	const ImGuiWindow* w = ImGui::FindWindowByName("EscWindow");
	CHECK(w != nullptr);
	if (w != nullptr)
	{
		click(w->Pos.x + 20.0f, w->Pos.y + ImGui::GetFrameHeight() + 20.0f);
		key(ImGuiKey_Escape);
	}
	CHECK(windowOpen);
	g_ui = nullptr;
	frames(2);
}

// Observable<ItemList> binds like ItemList&: on ImGui there is no poll, the
// tree is rebuilt from the live value every frame.
TEST(imgui_observable_items_bind_like_a_reference)
{
	Observable<ItemList> items { ItemList { "alpha", "beta" } };
	std::string picked;
	g_ui = [&] {
		Dialog { "Observed",
			VStack { ListBox{ items, picked }.withVisibleRows(4) }
		}.setPosition({0, 0}).show();
	};
	frames(3);
	items.edit()[0] = "gamma";
	frames(2);
	const ImGuiWindow* w = ImGui::FindWindowByName("Observed");
	const ImGuiStyle& st = ImGui::GetStyle();
	const float x = w->Pos.x + st.WindowPadding.x + 20.0f;
	const float y = w->Pos.y + ImGui::GetFrameHeight() + st.WindowPadding.y
		+ st.FramePadding.y + ImGui::GetTextLineHeightWithSpacing() * 0.5f;
	click(x, y);
	CHECK_EQ(picked, std::string("gamma"));
	g_ui = nullptr;
	frames(2);
}

// Focus and validity: the field opened with isFocused() takes typing first; a
// bound flag set true moves focus (onFocus), clicking away blurs (onBlur, flag
// back to false); isInvalid draws without moving anything.
TEST(imgui_text_field_focus_follows_the_flag_and_reports)
{
	std::string name;
	std::string email;
	bool emailFocused = false;
	bool emailInvalid = true;
	std::vector<std::string> log;
	g_ui = [&] {
		Dialog { "Focus",
			VStack {
				TextCtrl{name}.withSize({200, -1}).isFocused(),
				TextCtrl{email}.withSize({200, -1})
					.isFocused(emailFocused)
					.isInvalid(emailInvalid)
					.onFocus([&] { log.push_back("focus"); })
					.onBlur([&] { log.push_back("blur"); })
			}
		}.setPosition({0, 0}).show();
	};
	frames(4);
	const ImGuiWindow* w = ImGui::FindWindowByName("Focus");
	const ImVec2 size = w->Size;
	frame(-1, -1, false, 'a');
	frames(2);
	CHECK_EQ(name, std::string("a")); // the starting focus took the typing

	emailFocused = true;
	frames(3);
	CHECK_EQ(log.size(), std::size_t(1));
	CHECK(!log.empty() && log[0] == "focus");
	frame(-1, -1, false, 'b');
	frames(2);
	CHECK_EQ(email, std::string("b"));
	CHECK_EQ(name, std::string("a"));

	click(w->Pos.x + w->Size.x - 10.0f, w->Pos.y + w->Size.y - 6.0f); // empty space
	frames(2);
	CHECK_EQ(log.size(), std::size_t(2));
	CHECK(log.size() == 2 && log[1] == "blur");
	CHECK(!emailFocused);

	emailInvalid = false;
	frames(2);
	CHECK_EQ(w->Size.x, size.x); // validity never moves anything
	CHECK_EQ(w->Size.y, size.y);
	g_ui = nullptr;
	frames(2);
}

// SearchField: typing writes the bound query, Enter runs onSearch and does
// NOT press the window's default button, the clear button empties the field
// as an ordinary edit (onChange with "", no onSearch).
TEST(imgui_search_field_owns_its_enter_and_clears)
{
	std::string query;
	std::vector<std::string> log;
	g_ui = [&] {
		Dialog { "Search",
			VStack {
				SearchField{query}.withSize({220, -1})
					.onChange([&](const std::string& q) { log.push_back("change:" + q); })
					.onSearch([&](const std::string& q) { log.push_back("search:" + q); }),
				Button{"OK"}.isDefault().onClick([&] { log.push_back("ok"); })
			}
		}.setPosition({0, 0}).show();
	};
	frames(4);
	const ImVec2 field = rowCentre("Search", 0);
	click(field.x + 60.0f, field.y);
	frame(-1, -1, false, 'c');
	frame(-1, -1, false, 'a');
	frames(1);
	CHECK_EQ(query, std::string("ca"));

	log.clear();
	key(ImGuiKey_Enter);
	CHECK(std::find(log.begin(), log.end(), "search:ca") != log.end());
	CHECK(std::find(log.begin(), log.end(), "ok") == log.end());

	// the clear button: the right-hand icon slot of the 220 px field
	const ImGuiStyle& st = ImGui::GetStyle();
	const ImGuiWindow* w = ImGui::FindWindowByName("Search");
	const float clearX = w->Pos.x + st.WindowPadding.x + 220.0f - st.FramePadding.x - ImGui::GetFontSize() * 0.5f;
	log.clear();
	click(clearX, field.y);
	CHECK(query.empty());
	CHECK(std::find(log.begin(), log.end(), "change:") != log.end());
	CHECK(std::find(log.begin(), log.end(), "search:") == log.end());
	g_ui = nullptr;
	frames(2);
}

// EditableCombo: typing writes free text, the arrow opens the list and a pick
// writes that item's text (one onChange each), an outside write shows.
TEST(imgui_editable_combo_takes_typing_and_picks)
{
	std::string font = "Arial";
	ItemList fonts { "Arial", "Courier", "Helvetica" };
	int changes = 0;
	g_ui = [&] {
		Dialog { "Fonts",
			VStack {
				EditableCombo{font, fonts}.withSize({200, -1})
					.onChange([&](const std::string&) { ++changes; })
			}
		}.setPosition({0, 0}).show();
	};
	frames(4);
	const ImGuiWindow* w = ImGui::FindWindowByName("Fonts");
	const ImGuiStyle& st = ImGui::GetStyle();
	const float y = w->Pos.y + ImGui::GetFrameHeight() + st.WindowPadding.y + ImGui::GetFrameHeight() * 0.5f;
	const float x0 = w->Pos.x + st.WindowPadding.x;

	click(x0 + 60.0f, y);
	frame(-1, -1, false, 'X');
	frames(1);
	CHECK_EQ(font, std::string("ArialX"));
	CHECK_EQ(changes, 1);

	// the arrow: last frame-height square of the 200 px control
	click(x0 + 200.0f - ImGui::GetFrameHeight() * 0.5f, y);
	frames(1);
	ImGuiContext& g = *ImGui::GetCurrentContext();
	CHECK(!g.OpenPopupStack.empty());
	if (!g.OpenPopupStack.empty() && g.OpenPopupStack.back().Window != nullptr)
	{
		const ImGuiWindow* popup = g.OpenPopupStack.back().Window;
		const float rowY = popup->Pos.y + st.WindowPadding.y + ImGui::GetTextLineHeightWithSpacing() * 1.5f;
		click(popup->Pos.x + 30.0f, rowY); // the second item
		CHECK_EQ(font, std::string("Courier"));
		CHECK_EQ(changes, 2);
	}

	font = "Times";
	fonts.push_back("Times");
	frames(2);
	CHECK_EQ(font, std::string("Times"));
	g_ui = nullptr;
	frames(2);
}

// Slider orientation and ticks: a vertical slider has its minimum at the
// bottom (a click near the top is a high value), and ticks are measured into
// the frame -- one band beside the track -- so drawing them never overflows.
TEST(imgui_vertical_slider_with_ticks)
{
	int level = 50;
	g_ui = [&] {
		Dialog { "Levels",
			HStack {
				Slider{ Range<int>{ .min = 0, .max = 100 }, level }
					.withOrientation(Orientation::Vertical)
					.withTicks(25)
					.withSize({-1, 200})
			}
		}.setPosition({0, 0}).show();
	};
	frames(4);
	const ImGuiWindow* w = ImGui::FindWindowByName("Levels");
	const ImGuiStyle& st = ImGui::GetStyle();
	const float x = w->Pos.x + st.WindowPadding.x + ImGui::GetFrameHeight() * 0.5f;
	const float top = w->Pos.y + ImGui::GetFrameHeight() + st.WindowPadding.y;
	click(x, top + 6.0f);
	CHECK(level >= 95);
	click(x, top + 194.0f);
	CHECK(level <= 5);

	SliderWrapper<int> plain(Range<int>{ 0, 100, 1 }, BoundValue<int>(level), {}, {}, 0);
	SliderWrapper<int> ticked(Range<int>{ 0, 100, 1 }, BoundValue<int>(level), {}, {}, 0, {},
		Orientation::Vertical, 25);
	CHECK_EQ(ticked.measureIntrinsic({}).width, ImGui::GetFrameHeight() + 6.0f);
	CHECK_EQ(plain.measureIntrinsic({}).height, ImGui::GetFrameHeight());
	const auto fractions = SliderWrapper<int>::tickFractions(Range<int>{ 0, 100, 1 }, 25);
	CHECK_EQ(fractions.size(), std::size_t(5));
	CHECK(SliderWrapper<int>::tickFractions(Range<int>{ 0, 100000, 1 }, 1).empty()); // too dense
	g_ui = nullptr;
	frames(2);
}

// Spinner: measures kDefaultSpinnerSize square unless sized, draws inside its
// frame (the drift guard would report otherwise), and an image path that
// cannot load is the default spinner. A real image is covered on wx and Qt.
TEST(imgui_spinner_measures_one_square_and_stays_in_frame)
{
	SpinnerWrapper plain({}, BoundValue<bool>(true), {}, {}, 0);
	CHECK_EQ(plain.measureIntrinsic({}).width, kDefaultSpinnerSize);
	CHECK_EQ(plain.measureIntrinsic({}).height, kDefaultSpinnerSize);
	CHECK_EQ(plain.measureContent({}).width, kDefaultSpinnerSize);
	SpinnerWrapper sized({}, BoundValue<bool>(true), {}, Size { 48, 48 }, 0);
	CHECK_EQ(sized.measureContent({}).width, 48);

	bool busy = true;
	g_ui = [&] {
		Dialog { "Busy",
			HStack {
				Spinner{}.isRunning(busy),
				// no loadable image here: headless there is no GL context
				// for the texture cache to upload into (see the icon test)
				Spinner{}.withSize({48, 48}),
				Spinner{}.withImage("/no/such/logo.png")
			}
		}.setPosition({0, 0}).show();
	};
	frames(5);
	busy = false;
	frames(3);
	const ImGuiWindow* w = ImGui::FindWindowByName("Busy");
	CHECK(w != nullptr);
	g_ui = nullptr;
	frames(2);
}

// Measure-phase state is per window. The measure pass runs before Begin, so
// it used to share one id scope and one per-frame counter across every
// top-level: closing the first window shifted the second window's measure
// keys onto the first one's stored sizes, and its frozen bound-content size
// changed under it. Each top-level now pushes its title around the measure.
TEST(imgui_measure_state_is_per_window)
{
	std::string status = "x";
	ItemList items { "a considerably long first item", "beta" };
	std::string picked;
	bool firstOpen = true;
	g_ui = [&] {
		if (firstOpen)
			Dialog { "First", VStack { StaticText{status} } }.setPosition({0, 0}).show();
		Dialog { "Second", VStack { ListBox{ items, picked }.withVisibleRows(3) } }
			.setPosition({300, 0}).show();
	};
	frames(3);
	const ImGuiWindow* second = ImGui::FindWindowByName("Second");
	const ImVec2 size = second->Size;
	firstOpen = false;
	frames(3);
	CHECK_EQ(second->Size.x, size.x);
	CHECK_EQ(second->Size.y, size.y);
	g_ui = nullptr;
	frames(2);
}

// Calendar: the drawn month grid. The same cell is a different day for a
// Monday-first and a Sunday-first week (October 2026 starts on a Thursday),
// a pick writes the bound date and reports it, the arrow moves the month on
// show without touching the date, and an outside write brings its month back.
namespace
{
ImVec2 calendarCellCentre(const char* window, int index)
{
	const ImGuiWindow* w = ImGui::FindWindowByName(window);
	const ImGuiStyle& st = ImGui::GetStyle();
	const ImVec2 origin(w->Pos.x + st.WindowPadding.x, w->Pos.y + ImGui::GetFrameHeight() + st.WindowPadding.y);
	const float cellW = std::max(ImGui::CalcTextSize("00").x, ImGui::CalcTextSize("We").x) + st.FramePadding.x * 2.0f + 2.0f;
	const float cellH = ImGui::GetFrameHeight();
	const float gridTop = ImGui::GetFrameHeight() + 4.0f + ImGui::GetTextLineHeight() + 4.0f;
	return ImVec2(origin.x + (index % 7) * (cellW + 2.0f) + cellW * 0.5f,
		origin.y + gridTop + (index / 7) * (cellH + 2.0f) + cellH * 0.5f);
}
} // namespace

TEST(imgui_calendar_picks_by_the_first_day_of_the_week)
{
	Date monday { 2026, 10, 3 };
	Date sunday { 2026, 10, 3 };
	Date reported {};
	g_ui = [&] {
		Dialog { "CalMon", VStack { Calendar{monday}.onChange([&](const Date& d) { reported = d; }) } }
			.setPosition({0, 0}).show();
		Dialog { "CalSun", VStack { Calendar{sunday}.withFirstDayOfWeek(FirstDayOfWeek::Sunday) } }
			.setPosition({400, 0}).show();
	};
	frames(4);
	const ImVec2 mon = calendarCellCentre("CalMon", 17);
	click(mon.x, mon.y);
	CHECK(monday == (Date { 2026, 10, 15 }));
	CHECK(reported == (Date { 2026, 10, 15 }));
	const ImVec2 sun = calendarCellCentre("CalSun", 17);
	click(sun.x, sun.y);
	CHECK(sunday == (Date { 2026, 10, 14 }));

	// the next-month arrow: the right end of the header row
	const ImGuiWindow* w = ImGui::FindWindowByName("CalMon");
	const ImGuiStyle& st = ImGui::GetStyle();
	const float headerY = w->Pos.y + ImGui::GetFrameHeight() + st.WindowPadding.y + ImGui::GetFrameHeight() * 0.5f;
	click(w->Pos.x + st.WindowPadding.x + (float)w->ContentSize.x - ImGui::GetFrameHeight() * 0.5f, headerY);
	CHECK(monday == (Date { 2026, 10, 15 })); // navigating is not picking
	click(mon.x, mon.y);                       // November 2026 starts on a Sunday: index 17 is the 12th
	CHECK(monday == (Date { 2026, 11, 12 }));

	monday = Date { 2026, 10, 3 };             // from outside: October comes back
	frames(2);
	click(mon.x, mon.y);
	CHECK(monday == (Date { 2026, 10, 15 }));
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
