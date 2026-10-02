// Backend behaviour tests for wx. The app is started without OnRun() and the
// event loop pumped by hand -- wxYield() for events, ProcessIdle() because
// RefSync rides idle and wx defers Destroy() to it. Needs a display: a real
// session on macOS/Windows, xvfb on Linux CI.

#include "../test_framework.hpp"

#include "declarative_ui.hpp"

#include <wx/combobox.h>
#include <wx/listbox.h>
#include <wx/statline.h>
#include <wx/dataview.h>
#include <wx/statusbr.h>
#include <wx/tooltip.h>
#include <wx/treectrl.h>
#include <wx/wx.h>

#include <cstdio>
#include <functional>
#include <thread>
#include <vector>

#ifdef __GLIBC__
#include <execinfo.h>
#include <unistd.h>
#endif

#ifdef __WXOSX__
// A real Cocoa click (performClick), so native radio grouping, if any, happens
// exactly as it would under the mouse. Objective-C++, kept in its own file.
extern "C" void wxTestNativeClick(void* nsView);
#endif

namespace
{

void pump(int rounds = 20)
{
	for (int i = 0; i < rounds; ++i)
	{
		wxYield();
		wxTheApp->ProcessIdle();
	}
}

wxWindow* windowTitled(const char* title)
{
	for (wxWindow* w : wxTopLevelWindows)
	{
		if (w->GetLabel() == title && w->IsShown() && !w->IsBeingDeleted())
			return w;
	}
	return nullptr;
}

template <typename T>
T* find(wxWindow* root, const std::function<bool(T*)>& pred)
{
	for (wxWindow* child : root->GetChildren())
	{
		if (auto* t = dynamic_cast<T*>(child); t != nullptr && pred(t))
			return t;
		if (T* deeper = find<T>(child, pred))
			return deeper;
	}
	return nullptr;
}

wxRadioButton* radio(wxWindow* w, const char* text)
{
	return find<wxRadioButton>(w, [text](wxRadioButton* r) { return r->GetLabelText() == text; });
}

void clickRadio(wxRadioButton* r)
{
#ifdef __WXOSX__
	wxTestNativeClick(r->GetHandle());
#else
	// What the native control does on a click: check itself, then notify.
	r->SetValue(true);
	wxCommandEvent event(wxEVT_RADIOBUTTON, r->GetId());
	event.SetEventObject(r);
	event.SetInt(1);
	r->ProcessWindowEvent(event);
#endif
}

} // namespace

// Review finding 1: groups came from declaration order, and wx chains every
// radio in a parent into one native group -- two groups in one box merged.
// Two rounds: the second is a re-show.
TEST(wx_radio_groups_survive_reshow_and_share_a_box)
{
	int choice = 1;
	int other = 0;
	for (int round = 0; round < 2; ++round)
	{
		choice = 1;
		other = 0;
		Dialog { "Radios",
			VStack {
				RadioButton{choice, 0, "Alpha"},
				RadioButton{choice, 1, "Beta"},
				RadioButton{choice, 2, "Gamma"},
				RadioButton{other, 0, "One"},
				RadioButton{other, 1, "Two"}
			}
		}.show();
		pump();
		wxWindow* w = windowTitled("Radios");
		CHECK(w != nullptr);
		if (w == nullptr)
			return;

		// Showing picks nothing: wxMSW focusing the first radio "clicked" it
		CHECK_EQ(choice, 1);
		CHECK_EQ(other, 0);
		CHECK(radio(w, "Beta")->GetValue());
		CHECK(!radio(w, "Alpha")->GetValue());
		CHECK(radio(w, "One")->GetValue());

		clickRadio(radio(w, "Gamma"));
		pump();
		CHECK_EQ(choice, 2);
		CHECK(!radio(w, "Beta")->GetValue());
		CHECK(radio(w, "Gamma")->GetValue());
		CHECK(radio(w, "One")->GetValue());

		clickRadio(radio(w, "Two"));
		pump();
		CHECK_EQ(other, 1);
		CHECK(radio(w, "Gamma")->GetValue());
		CHECK(!radio(w, "One")->GetValue());

		w->Close();
		pump();
	}
}

// Review finding 2: the combo was editable on wx only.
TEST(wx_combo_is_read_only_and_follows_its_string)
{
	std::string colour = "Green";
	Dialog { "Combo",
		VStack { ComboBox{ {"Red", "Green", "Blue"}, colour } }
	}.show();
	pump();
	wxWindow* w = windowTitled("Combo");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	auto* combo = find<wxComboBox>(w, [](wxComboBox*) { return true; });
	CHECK(combo != nullptr && combo->HasFlag(wxCB_READONLY));
	CHECK(combo != nullptr && combo->GetStringSelection() == "Green");
	colour = "Blue";
	pump();
	CHECK(combo != nullptr && combo->GetStringSelection() == "Blue");
	w->Close();
	pump();
}

// Review finding 6: '&' in a leaf label became a mnemonic.
TEST(wx_labels_keep_ampersands)
{
	bool flag = false;
	Dialog { "Labels",
		VStack {
			Button{"Save & Exit"},
			CheckBox{flag, "A & B"},
			StaticText{"Fish & Chips"}
		}
	}.show();
	pump();
	wxWindow* w = windowTitled("Labels");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	CHECK(find<wxButton>(w, [](wxButton* b) { return b->GetLabelText() == "Save & Exit"; }) != nullptr);
	CHECK(find<wxCheckBox>(w, [](wxCheckBox* c) { return c->GetLabelText() == "A & B"; }) != nullptr);
	CHECK(find<wxStaticText>(w, [](wxStaticText* t) { return t->GetLabelText() == "Fish & Chips"; }) != nullptr);
	w->Close();
	pump();
}

// The collapsed callback path (EventCallback + commitTo): the bound value is
// written first, then the callback runs with the native widget -- for both
// spellings of onChange, bound and unbound.
TEST(wx_change_callbacks_write_the_value_first_then_report)
{
	std::string text = "start";
	std::string seenValue;
	void* seenNative = nullptr;
	int shortCalls = 0;
	std::string unboundSeen;
	Dialog { "Callbacks",
		VStack {
			TextCtrl{text}.onChange([&](const std::string&, void* native) {
				seenValue = text; // the bound variable, read inside the handler
				seenNative = native;
			}),
			TextCtrl{std::string("fixed")}.onChange([&](const std::string& v) {
				++shortCalls;
				unboundSeen = v;
			})
		}
	}.show();
	pump();
	wxWindow* w = windowTitled("Callbacks");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	auto* bound = find<wxTextCtrl>(w, [](wxTextCtrl* t) { return t->GetValue() == "start"; });
	auto* unbound = find<wxTextCtrl>(w, [](wxTextCtrl* t) { return t->GetValue() == "fixed"; });
	CHECK(bound != nullptr && unbound != nullptr);
	if (bound == nullptr || unbound == nullptr)
		return;

	bound->SetValue("typed"); // SetValue, unlike ChangeValue, sends wxEVT_TEXT
	pump();
	CHECK_EQ(text, std::string("typed"));
	CHECK_EQ(seenValue, std::string("typed"));
	CHECK(seenNative == bound);

	unbound->SetValue("other");
	pump();
	CHECK_EQ(shortCalls, 1);
	CHECK_EQ(unboundSeen, std::string("other"));
	w->Close();
	pump();
}

// Review finding 9: a Separator measured 2 px on wx (3 on Qt, 1 on ImGui) and
// a StatusBar took its width from the native bar. Both are now the shared
// rule: a 1 px line, and fixed widths + 120 per stretch field + 8 px gaps.
TEST(wx_separator_and_status_bar_follow_the_shared_rule)
{
	Dialog { "Rules",
		VStack {
			Separator{}.withFlags(LayoutFlags().Expand()),
			StatusBar { StatusField{"fixed", 100}, StatusField{"stretch"} }
				.withFlags(LayoutFlags())
		}
	}.show();
	pump();
	wxWindow* w = windowTitled("Rules");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	auto* line = find<wxStaticLine>(w, [](wxStaticLine*) { return true; });
	auto* bar = find<wxStatusBar>(w, [](wxStatusBar*) { return true; });
	CHECK(line != nullptr && line->GetSize().y == 1);
	CHECK(bar != nullptr && bar->GetSize().x == statusBarContentWidth({ StatusField{"a", 100}, StatusField{"b"} }));
	w->Close();
	pump();
}

// wxSTB_SHOW_TIPS makes wx assert on any tooltip call: a bar without a tooltip
// must make none (not even an unset), and one with a tooltip gives up the
// style. On wxGTK/wxMSW the assert fires through the handler set in main().
TEST(wx_status_bar_tooltips_do_not_assert)
{
	Dialog { "Tips",
		VStack {
			StatusBar { StatusField{"plain"} },
			StatusBar { StatusField{"tipped"} }.withTooltip("A tip")
		}
	}.show();
	pump();
	wxWindow* w = windowTitled("Tips");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	auto* plain = find<wxStatusBar>(w, [](wxStatusBar* b) { return b->GetStatusText(0) == "plain"; });
	auto* tipped = find<wxStatusBar>(w, [](wxStatusBar* b) { return b->GetStatusText(0) == "tipped"; });
	CHECK(plain != nullptr && tipped != nullptr);
	if (plain == nullptr || tipped == nullptr)
		return;
	CHECK(plain->HasFlag(wxSTB_SHOW_TIPS) && plain->GetToolTip() == nullptr);
	CHECK(!tipped->HasFlag(wxSTB_SHOW_TIPS));
	CHECK(tipped->GetToolTip() != nullptr && tipped->GetToolTip()->GetTip() == "A tip");
	w->Close();
	pump();
}

// withId() names the native window; postToUi() runs a worker's task on the UI
// thread.
TEST(wx_with_id_names_the_window_and_post_to_ui_crosses_threads)
{
	bool flag = false;
	Dialog { "Named", VStack { CheckBox{flag, "Box"}.withId("named-box") } }.show();
	pump();
	wxWindow* w = windowTitled("Named");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	auto* box = find<wxCheckBox>(w, [](wxCheckBox* c) { return c->GetName() == "named-box"; });
	CHECK(box != nullptr);

	const auto uiThread = std::this_thread::get_id();
	bool onUiThread = false;
	std::thread([&] {
		postToUi([&] { flag = true; onUiThread = std::this_thread::get_id() == uiThread; });
	}).join();
	pump();
	CHECK(flag);
	CHECK(onUiThread);
	CHECK(box != nullptr && box->GetValue()); // and RefSync mirrored it
	w->Close();
	pump();
}

// A bound StaticText / ReadonlyTextCtrl follows the caller's string but keeps
// the size of its first text, even through a full re-measure (an Expander
// opening forces one).
TEST(wx_bound_labels_follow_text_without_resizing)
{
	std::string status = "Ready";
	bool open = false;
	Dialog { "Status",
		VStack {
			StaticText{status},
			ReadonlyTextCtrl{status},
			Expander { "More", open, StaticText{"x"} }
		}
	}.show();
	pump();
	wxWindow* w = windowTitled("Status");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	auto* label = find<wxStaticText>(w, [](wxStaticText* t) { return t->GetLabelText() == "Ready"; });
	auto* field = find<wxTextCtrl>(w, [](wxTextCtrl*) { return true; });
	CHECK(label != nullptr && field != nullptr);
	if (label == nullptr || field == nullptr)
		return;
	const int labelWidth = label->GetSize().x;

	status = "This status message is far longer than the label it arrives in, by design";
	open = true;
	pump(40);
	CHECK(std::string(label->GetLabelText().ToUTF8()) == status);
	CHECK(std::string(field->GetValue().ToUTF8()) == status);
	CHECK_EQ(label->GetSize().x, labelWidth);
	w->Close();
	pump();
}

// isHidden(): a hidden control and a hidden group box are taken out of view
// and out of the layout; flipping the bound flag back restores both.
TEST(wx_hidden_nodes_leave_and_return)
{
	bool hideButton = false;
	bool hideBox = false;
	Dialog { "Hide",
		VStack {
			Button{"Always"},
			Button{"Sometimes"}.isHidden(hideButton),
			VGroupBox { "Box", Button{"Inside"} }.isHidden(hideBox)
		}
	}.show();
	pump();
	wxWindow* w = windowTitled("Hide");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	auto* sometimes = find<wxButton>(w, [](wxButton* b) { return b->GetLabelText() == "Sometimes"; });
	auto* inside = find<wxButton>(w, [](wxButton* b) { return b->GetLabelText() == "Inside"; });
	auto* box = find<wxStaticBox>(w, [](wxStaticBox*) { return true; });
	CHECK(sometimes && inside && box);
	if (!(sometimes && inside && box))
		return;
	const int fullHeight = w->GetClientSize().y;

	hideButton = true;
	hideBox = true;
	pump(40);
	CHECK(!sometimes->IsShown());
	CHECK(!box->IsShown());
	CHECK(!inside->IsShown());
	CHECK(w->GetClientSize().y < fullHeight);

	hideButton = false;
	hideBox = false;
	pump(40);
	CHECK(sometimes->IsShown());
	CHECK(box->IsShown());
	CHECK(inside->IsShown());
	CHECK_EQ(w->GetClientSize().y, fullHeight);
	w->Close();
	pump();
}

// Bound item lists: the native list and combo are repopulated when the
// caller's vector changes, the selection stays on the same item by value, and
// the window does not resize.
TEST(wx_bound_item_lists_repopulate_and_keep_the_selection)
{
	ItemList items { "alpha", "beta", "gamma" };
	std::string listPick = "beta";
	std::string comboPick = "gamma";
	Dialog { "Items",
		VStack {
			ListBox{ items, listPick }.withVisibleRows(4),
			ComboBox{ items, comboPick }
		}
	}.show();
	pump();
	wxWindow* w = windowTitled("Items");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	auto* list = find<wxListBox>(w, [](wxListBox*) { return true; });
	auto* combo = find<wxComboBox>(w, [](wxComboBox*) { return true; });
	CHECK(list && combo);
	if (!(list && combo))
		return;
	const int width = w->GetClientSize().x;

	items = { "gamma", "a considerably longer new item", "beta" };
	pump(40);
	CHECK_EQ(list->GetCount(), 3u);
	CHECK_EQ(combo->GetCount(), 3u);
	CHECK(list->GetStringSelection() == "beta");
	CHECK(combo->GetStringSelection() == "gamma");
	CHECK_EQ(listPick, std::string("beta"));
	CHECK_EQ(w->GetClientSize().x, width);
	w->Close();
	pump();
}

// VForEach: rows follow a bound vector. Adding one builds its widgets; a row's
// own Remove button removes that row, and its widgets are destroyed, not left
// behind in the window.
TEST(wx_foreach_rows_follow_the_vector)
{
	std::vector<std::string> todos { "one", "two" };
	Dialog { "Todos",
		VForEach { todos, [&todos](const std::string& todo, std::size_t i) {
			return HStack {
				StaticText{todo},
				Button{"Remove " + todo}.onClick([&todos, i] { todos.erase(todos.begin() + (long)i); })
			};
		} }
	}.show();
	pump();
	wxWindow* w = windowTitled("Todos");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	const auto countButtons = [w] {
		int n = 0;
		find<wxButton>(w, [&n](wxButton*) { ++n; return false; });
		return n;
	};
	const int twoRows = w->GetClientSize().y;
	CHECK_EQ(countButtons(), 2);

	todos.push_back("three");
	pump(40);
	CHECK_EQ(countButtons(), 3);
	CHECK(w->GetClientSize().y > twoRows);

	auto* removeOne = find<wxButton>(w, [](wxButton* b) { return b->GetLabelText() == "Remove one"; });
	CHECK(removeOne != nullptr);
	if (removeOne != nullptr)
	{
		wxCommandEvent click(wxEVT_BUTTON, removeOne->GetId());
		click.SetEventObject(removeOne);
		removeOne->ProcessWindowEvent(click);
	}
	pump(40);
	CHECK_EQ(todos.size(), static_cast<std::size_t>(2));
	CHECK_EQ(countButtons(), 2);
	CHECK(find<wxButton>(w, [](wxButton* b) { return b->GetLabelText() == "Remove one"; }) == nullptr);
	CHECK_EQ(w->GetClientSize().y, twoRows);
	w->Close();
	pump();
}

namespace
{
struct TestApp : wxApp
{
	bool OnInit() override { return true; }
};
} // namespace

wxIMPLEMENT_APP_NO_MAIN(TestApp);

// RadioGroup: one radio per option, bound to the index, in a row when asked;
// a write from outside moves the check, onChange reports after the write.
TEST(wx_radio_group_binds_an_index)
{
	int choice = 0;
	int reported = -1;
	int seen = -1;
	Dialog { "Group",
		RadioGroup{choice, {"Red", "Green", "Blue"}}
			.withOrientation(Orientation::Horizontal)
			.onChange([&](int i) { reported = i; seen = choice; })
	}.show();
	pump();
	wxWindow* w = windowTitled("Group");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;

	CHECK(radio(w, "Red")->GetValue());
	CHECK_EQ(radio(w, "Red")->GetPosition().y, radio(w, "Blue")->GetPosition().y);
	CHECK(radio(w, "Red")->GetPosition().x < radio(w, "Blue")->GetPosition().x);

	clickRadio(radio(w, "Blue"));
	pump();
	CHECK_EQ(choice, 2);
	CHECK_EQ(reported, 2);
	CHECK_EQ(seen, 2);
	CHECK(!radio(w, "Red")->GetValue());

	choice = 1;
	pump();
	CHECK(radio(w, "Green")->GetValue());
	CHECK(!radio(w, "Blue")->GetValue());

	w->Close();
	pump();
}

// Default / cancel buttons. Keys are delivered the way wx delivers them first --
// a wxEVT_CHAR_HOOK on the top-level window -- and a single-line field's Enter
// as its wxEVT_TEXT_ENTER; native key synthesis needs OS permissions here.
TEST(wx_default_and_cancel_buttons_answer_enter_and_escape)
{
	std::vector<std::string> log;
	std::string query = "cats";
	std::string notes;
	bool okDisabled = false;
	Dialog { "Keys",
		VStack {
			TextCtrl{query}.onEnter([&](const std::string& t) { log.push_back("enter:" + t); }),
			Button{"OK"}.isDefault().isDisabled(okDisabled).onClick([&] { log.push_back("ok"); }),
			Button{"Cancel"}.isCancel().onClick([&] { log.push_back("cancel"); }),
			MultiLineTextCtrl{notes}
		}
	}.show();
	pump();
	wxWindow* w = windowTitled("Keys");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	auto hook = [w](int code) {
		wxKeyEvent event(wxEVT_CHAR_HOOK);
		event.m_keyCode = code;
		event.SetEventObject(w);
		w->ProcessWindowEvent(event);
		return event.GetSkipped();
	};
	auto* edit = find<wxTextCtrl>(w, [](wxTextCtrl* t) { return !t->IsMultiLine(); });
	auto* ok = find<wxButton>(w, [](wxButton* b) { return b->GetLabelText() == "OK"; });
	CHECK(edit != nullptr && ok != nullptr);
	if (edit == nullptr || ok == nullptr)
		return;
	CHECK(edit->HasFlag(wxTE_PROCESS_ENTER)); // only because it has an onEnter

	wxCommandEvent enter(wxEVT_TEXT_ENTER, edit->GetId());
	enter.SetEventObject(edit);
	edit->ProcessWindowEvent(enter);
	CHECK_EQ(log.size(), std::size_t(2));
	if (log.size() == 2)
	{
		CHECK(log[0] == "enter:cats");
		CHECK(log[1] == "ok");
	}

	log.clear();
	CHECK(!hook(WXK_ESCAPE)); // handled: Cancel pressed, the dialog stays
	pump();
	CHECK_EQ(log.size(), std::size_t(1));
	CHECK(!log.empty() && log[0] == "cancel");
	CHECK(windowTitled("Keys") == w);

	// Enter while the onEnter field has focus is left to that field
	if (wxWindow::FindFocus() == edit)
	{
		log.clear();
		CHECK(hook(WXK_RETURN));
		CHECK(log.empty());
	}
	w->Close();
	pump();

	// A field WITHOUT onEnter does not consume Enter: the hook presses the
	// default button whatever has focus, and skips a disabled one.
	std::string plain;
	Dialog { "Keys2",
		VStack {
			TextCtrl{plain},
			Button{"OK"}.isDefault().isDisabled(okDisabled).onClick([&] { log.push_back("ok"); })
		}
	}.show();
	pump();
	wxWindow* w2 = windowTitled("Keys2");
	CHECK(w2 != nullptr);
	if (w2 == nullptr)
		return;
	auto hook2 = [w2](int code) {
		wxKeyEvent event(wxEVT_CHAR_HOOK);
		event.m_keyCode = code;
		event.SetEventObject(w2);
		w2->ProcessWindowEvent(event);
		return event.GetSkipped();
	};
	auto* plainEdit = find<wxTextCtrl>(w2, [](wxTextCtrl*) { return true; });
	CHECK(plainEdit != nullptr && !plainEdit->HasFlag(wxTE_PROCESS_ENTER));
	log.clear();
	CHECK(!hook2(WXK_RETURN));
	CHECK_EQ(log.size(), std::size_t(1));
	CHECK(!log.empty() && log[0] == "ok");

	okDisabled = true;
	pump();
	log.clear();
	CHECK(hook2(WXK_RETURN)); // no enabled default button: skipped on
	CHECK(log.empty());
	CHECK(hook2(WXK_ESCAPE)); // no cancel button: the dialog's own Escape
	pump();
	if (wxWindow* still = windowTitled("Keys2"))
		still->Close();
	pump();
}

// withIcon: the bitmap is set at the asked size and widens the button (a long
// label, since macOS pads a short one to a minimum width anyway); a path
// that fails to load leaves a plain text button.
TEST(wx_icon_button_carries_its_icon)
{
	Dialog { "Icons",
		VStack {
			Button{"Save the whole document"},
			Button{"Save the whole document "}.withIcon(DUI_TEST_ICON, { 20, 20 }),
			Button{"Save the whole document  "}.withIcon("/no/such/icon.png")
		}
	}.show();
	pump();
	wxWindow* w = windowTitled("Icons");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	auto byLabel = [w](const char* text) {
		return find<wxButton>(w, [text](wxButton* b) { return b->GetLabel() == text; });
	};
	wxButton* plain = byLabel("Save the whole document");
	wxButton* icon = byLabel("Save the whole document ");
	wxButton* broken = byLabel("Save the whole document  ");
	CHECK(plain != nullptr && icon != nullptr && broken != nullptr);
	if (plain == nullptr || icon == nullptr || broken == nullptr)
		return;
	CHECK(icon->GetBitmap().IsOk());
	CHECK(icon->GetBitmap().GetWidth() == 20 || icon->GetBitmap().GetLogicalWidth() == 20);
	CHECK(icon->GetBestSize().GetWidth() > plain->GetBestSize().GetWidth());
	CHECK(!broken->GetBitmap().IsOk());
	w->Close();
	pump();
}

// Bound TreeView items and Table rows follow the caller's data: refilled,
// selection kept by path / key, the user's open state kept for items that are
// still there and a new item's own flag honoured, the window not resized.
TEST(wx_bound_tree_and_table_follow_their_data)
{
	std::vector<TreeItem> tree {
		{ "Fruits", { { "Apple" }, { "Banana" } }, false },
		{ "Veg", { { "Leek" } }, false } };
	std::string treePick = "Fruits/Banana";
	TableRows rows { { "a.txt", "1" }, { "b.txt", "2" } };
	std::string rowPick = "b.txt";
	Dialog { "Data",
		VStack {
			TreeView{ tree, treePick }.withVisibleRows(5),
			Table{ { { "File", -1, true }, { "Size", -1 } }, rows, rowPick }.withVisibleRows(4)
		}
	}.show();
	pump();
	wxWindow* w = windowTitled("Data");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	auto* tc = find<wxTreeCtrl>(w, [](wxTreeCtrl*) { return true; });
	auto* view = find<wxDataViewListCtrl>(w, [](wxDataViewListCtrl*) { return true; });
	CHECK(tc != nullptr && view != nullptr);
	if (tc == nullptr || view == nullptr)
		return;
	auto topLevel = [tc](int n) {
		wxTreeItemIdValue cookie;
		wxTreeItemId id = tc->GetFirstChild(tc->GetRootItem(), cookie);
		for (int i = 0; i < n && id.IsOk(); ++i)
			id = tc->GetNextChild(tc->GetRootItem(), cookie);
		return id;
	};
	const wxSize size = w->GetSize();
	// wxTreeCtrl opens a collapsed parent when it selects a child in it, so
	// "Fruits" may already be open because "Fruits/Banana" is selected; either
	// way the refill keeps it as it was.
	const bool fruitsOpen = tc->IsExpanded(topLevel(0));
	tc->Expand(topLevel(1)); // the user opens "Veg"

	tree[0].children.push_back({ "Cherry with a very long name indeed" });
	tree.push_back({ "Nuts", { { "Almond" } }, true });
	rows.insert(rows.begin(), { "a much longer file name than any before.txt", "3" });
	pump();

	CHECK_EQ(static_cast<int>(tc->GetChildrenCount(tc->GetRootItem(), false)), 3);
	CHECK_EQ(static_cast<int>(tc->GetChildrenCount(topLevel(0), false)), 3);
	CHECK_EQ(tc->IsExpanded(topLevel(0)), fruitsOpen); // as the user left it
	CHECK(tc->IsExpanded(topLevel(1)));  // opened by the user: kept
	CHECK(tc->IsExpanded(topLevel(2)));  // new: its own flag
	CHECK(tc->GetSelection().IsOk() && tc->GetItemText(tc->GetSelection()) == "Banana");
	CHECK_EQ(treePick, std::string("Fruits/Banana"));

	CHECK_EQ(static_cast<int>(view->GetItemCount()), 3);
	const int selectedRow = view->ItemToRow(view->GetSelection());
	CHECK(selectedRow >= 0 && view->GetTextValue(selectedRow, 0) == "b.txt");
	CHECK_EQ(rowPick, std::string("b.txt"));
	CHECK(w->GetSize() == size);

	rows[1][1] = "42"; // a cell written from outside
	pump();
	bool found = false;
	for (int row = 0; row < static_cast<int>(view->GetItemCount()); ++row)
		found = found || view->GetTextValue(row, 1) == "42";
	CHECK(found);
	w->Close();
	pump();
	// wxGTK empties the selection as it tears the controls down: not a pick
	CHECK_EQ(treePick, std::string("Fruits/Banana"));
	CHECK_EQ(rowPick, std::string("b.txt"));
}

// A refill re-applies the user's sort, and only when there is one: wxGTK's
// Resort() with no sorting column compared by column -1 and read past the row.
TEST(wx_table_refill_keeps_the_users_sort)
{
	TableRows rows { { "b.txt" }, { "c.txt" } };
	Dialog { "Sorted",
		VStack { Table{ { { "File", 120, true } }, rows, std::string{} }.withVisibleRows(4) }
	}.show();
	pump();
	wxWindow* w = windowTitled("Sorted");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	auto* view = find<wxDataViewListCtrl>(w, [](wxDataViewListCtrl*) { return true; });
	CHECK(view != nullptr);
	if (view == nullptr)
		return;
	auto shownFirst = [view] {
		wxVariant value;
		view->GetStore()->GetValue(value, view->RowToItem(0), 0);
		return value.GetString();
	};

	rows.push_back({ "a.txt" }); // unsorted: declaration order
	pump();
	CHECK_EQ(static_cast<int>(view->GetItemCount()), 3);
	CHECK(shownFirst() == "b.txt");

	// The user sorts descending. wxOSX's SetSortOrder() only marks the column
	// (the view sorts on a real header click), so the order is checked where
	// the call does sort -- wxGTK, the platform this path broke on.
	view->GetColumn(0)->SetSortOrder(false);
	pump();
	const bool sorted = shownFirst() == "c.txt";
	rows.push_back({ "d.txt" });
	pump();
	CHECK_EQ(static_cast<int>(view->GetItemCount()), 4);
	if (sorted)
		CHECK(shownFirst() == "d.txt");
	w->Close();
	pump();
}

// Observable rows: the poll reads the change counter, so an edit() refills the
// table and a write through the raw binding reference is (by contract) not seen.
TEST(wx_observable_rows_refill_by_their_counter)
{
	Observable<TableRows> rows { TableRows { { "a.txt", "1" }, { "b.txt", "2" } } };
	Dialog { "Observed",
		VStack {
			Table{ { { "File", 120 }, { "Size", 60 } }, rows, std::string{} }.withVisibleRows(4)
		}
	}.show();
	pump();
	wxWindow* w = windowTitled("Observed");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	auto* view = find<wxDataViewListCtrl>(w, [](wxDataViewListCtrl*) { return true; });
	CHECK(view != nullptr);
	if (view == nullptr)
		return;

	rows.edit().push_back({ "c.txt", "3" });
	pump();
	CHECK_EQ(static_cast<int>(view->GetItemCount()), 3);

	static_cast<TableRows&>(rows).push_back({ "uncounted.txt", "0" });
	pump();
	CHECK_EQ(static_cast<int>(view->GetItemCount()), 3); // not counted, not seen
	rows.edit();
	pump();
	CHECK_EQ(static_cast<int>(view->GetItemCount()), 4);
	w->Close();
	pump();
}

// Focus and validity. The focus events are delivered as wx delivers them
// (wxEVT_SET_FOCUS / KILL_FOCUS on the control); whether the platform grants
// real focus to a test window is checked only where it does.
TEST(wx_text_field_focus_follows_the_flag_and_reports)
{
	std::string email;
	bool emailFocused = false;
	bool emailInvalid = false;
	std::vector<std::string> log;
	Dialog { "Focus",
		VStack {
			TextCtrl{email}
				.isFocused(emailFocused)
				.isInvalid(emailInvalid)
				.onFocus([&] { log.push_back("focus"); })
				.onBlur([&] { log.push_back("blur"); })
		}
	}.show();
	pump();
	wxWindow* w = windowTitled("Focus");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	auto* field = find<wxTextCtrl>(w, [](wxTextCtrl*) { return true; });
	CHECK(field != nullptr);
	if (field == nullptr)
		return;

	// a dialog gives its only field focus as it opens -- reported like any other
	if (wxWindow::FindFocus() == field)
		CHECK(emailFocused && !log.empty() && log.back() == "focus");
	log.clear();
	wxFocusEvent in(wxEVT_SET_FOCUS, field->GetId());
	in.SetEventObject(field);
	field->ProcessWindowEvent(in);
	CHECK(!log.empty() && log[0] == "focus");
	CHECK(emailFocused);
	wxFocusEvent out(wxEVT_KILL_FOCUS, field->GetId());
	out.SetEventObject(field);
	field->ProcessWindowEvent(out);
	CHECK(log.size() == 2 && log[1] == "blur");
	CHECK(!emailFocused);

	emailFocused = true; // a request: SetFocus()
	pump();
	if (wxWindow::FindFocus() != nullptr)
		CHECK(wxWindow::FindFocus() == field);

	emailInvalid = true;
	pump();
	CHECK(field->GetBackgroundColour() == wxColour(253, 228, 228));
	emailInvalid = false;
	pump();
	CHECK(field->GetBackgroundColour() != wxColour(253, 228, 228));
	w->Close();
	pump();
}

int main(int argc, char** argv)
{
	if (!wxEntryStart(argc, argv))
		return 2;
	wxTheApp->CallOnInit();
	// wxLogGui turns every wxLogError into a modal box on the next idle, and a
	// GUI app's assert handler opens one too -- either stalls the run forever
	// with nobody there to close it (CI). Both go to stderr; an assert fails
	// the test it fired in.
	delete wxLog::SetActiveTarget(new wxLogStderr);
	wxSetAssertHandler([](const wxString& file, int line, const wxString& func,
							const wxString& cond, const wxString& msg) {
		std::fprintf(stderr, "%s(%d): assert \"%s\" failed in %s(): %s\n",
			static_cast<const char*>(file.utf8_str()), line,
			static_cast<const char*>(cond.utf8_str()),
			static_cast<const char*>(func.utf8_str()),
			static_cast<const char*>(msg.utf8_str()));
#ifdef __GLIBC__
		// Only CI's wxGTK/wxMSW builds assert at all, so the stack is all there
		// is to go on (-rdynamic gives it names).
		void* frames[64];
		backtrace_symbols_fd(frames, backtrace(frames, 64), STDERR_FILENO);
#endif
		++testfw::failureCount();
	});
	// Unbuffered, so a crash still shows the last test that finished.
	std::setvbuf(stdout, nullptr, _IONBF, 0);
	const int result = testfw::runAll();
	wxTheApp->OnExit();
	wxEntryCleanup();
	return result;
}
