// Backend behaviour tests for wx. The app is started without OnRun() and the
// event loop pumped by hand -- wxYield() for events, ProcessIdle() because
// RefSync rides idle and wx defers Destroy() to it. Needs a display: a real
// session on macOS/Windows, xvfb on Linux CI.

#include "../test_framework.hpp"

#include "declarative_ui.hpp"

#include <wx/combobox.h>
#include <wx/listbox.h>
#include <wx/statline.h>
#include <wx/statusbr.h>
#include <wx/wx.h>

#include <functional>
#include <thread>

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

int main(int argc, char** argv)
{
	if (!wxEntryStart(argc, argv))
		return 2;
	wxTheApp->CallOnInit();
	const int result = testfw::runAll();
	wxTheApp->OnExit();
	wxEntryCleanup();
	return result;
}
