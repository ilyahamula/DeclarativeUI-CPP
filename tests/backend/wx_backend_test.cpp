// Backend behaviour tests for wx. The app is started without OnRun() and the
// event loop pumped by hand -- wxYield() for events, ProcessIdle() because
// RefSync rides idle and wx defers Destroy() to it. Needs a display: a real
// session on macOS/Windows, xvfb on Linux CI.

#include "../test_framework.hpp"

#include "declarative_ui.hpp"

#include <wx/combobox.h>
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

namespace
{
struct TestApp : wxApp
{
	bool OnInit() override { return true; }
};
} // namespace

wxIMPLEMENT_APP_NO_MAIN(TestApp);

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
