// Backend behaviour tests for Qt, run on the offscreen platform plugin so no
// display is needed. Windows are real Qt widgets; the event loop is pumped by
// hand with wall-clock time passing, because RefSync polls on a QTimer.

#include "../test_framework.hpp"

#include "declarative_ui.hpp"

#include <QApplication>

#include <thread>
#include <vector>
#include <QElapsedTimer>
#include <QCheckBox>
#include <QComboBox>
#include <QListWidget>
#include <QFrame>
#include <QGroupBox>
#include <QStatusBar>
#include <QLabel>
#include <QPlainTextEdit>
#include <QStyleOption>
#include <QTabWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QTimer>
#include <QTreeWidget>

namespace
{

void pump(int ms = 80)
{
	QElapsedTimer timer;
	timer.start();
	while (timer.elapsed() < ms)
	{
		QApplication::processEvents();
		QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
	}
}

QWidget* windowTitled(const char* title)
{
	for (QWidget* w : QApplication::topLevelWidgets())
	{
		if (w->windowTitle() == title && w->isVisible())
			return w;
	}
	return nullptr;
}

QRadioButton* radio(QWidget* w, const char* text)
{
	for (auto* r : w->findChildren<QRadioButton*>())
	{
		if (r->text() == text)
			return r;
	}
	return nullptr;
}

} // namespace

// Review finding 1: groups came from declaration order and a static
// QButtonGroup, so a re-shown dialog joined a freed group and two groups in one
// box merged. Two rounds: the second is the re-show.
TEST(qt_radio_groups_survive_reshow_and_share_a_box)
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
		QWidget* w = windowTitled("Radios");
		CHECK(w != nullptr);
		if (w == nullptr)
			return;

		CHECK(radio(w, "Beta")->isChecked());
		CHECK(radio(w, "One")->isChecked());
		radio(w, "Gamma")->click();
		pump();
		CHECK_EQ(choice, 2);
		CHECK(!radio(w, "Beta")->isChecked());
		CHECK(radio(w, "Gamma")->isChecked());
		CHECK(radio(w, "One")->isChecked());

		radio(w, "Two")->click();
		pump();
		CHECK_EQ(other, 1);
		CHECK(radio(w, "Gamma")->isChecked());

		// a checked radio stays checked when clicked again
		radio(w, "Gamma")->click();
		pump();
		CHECK(radio(w, "Gamma")->isChecked());
		CHECK_EQ(choice, 2);

		w->close();
		pump();
	}
}

// Review findings 3 and 6: labels auto-detected HTML, and '&' became a
// mnemonic.
TEST(qt_labels_are_plain_text_and_keep_ampersands)
{
	bool flag = false;
	Dialog { "Labels",
		VStack {
			Button{"Save & Exit"},
			CheckBox{flag, "A & B"},
			StaticText{"<b>not bold</b>"}
		}
	}.show();
	pump();
	QWidget* w = windowTitled("Labels");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;

	bool sawButton = false;
	for (auto* b : w->findChildren<QPushButton*>())
		sawButton |= b->text() == QStringLiteral("Save && Exit");
	CHECK(sawButton);

	bool sawPlain = false;
	for (auto* l : w->findChildren<QLabel*>())
		sawPlain |= l->text().contains(QStringLiteral("not bold")) && l->textFormat() == Qt::PlainText;
	CHECK(sawPlain);

	w->close();
	pump();
}

// The collapsed callback path (EventCallback + commitTo): the bound value is
// written first, then the callback runs with the native widget -- for both
// spellings of onChange, bound and unbound.
TEST(qt_change_callbacks_write_the_value_first_then_report)
{
	std::string text = "start";
	std::string seenValue;
	void* seenNative = nullptr;
	int shortCalls = 0;
	std::string unboundSeen;
	Dialog { "Callbacks",
		VStack {
			TextCtrl{text}.onChange([&](const std::string& v, void* native) {
				seenValue = text; // the bound variable, read inside the handler
				seenNative = native;
				(void)v;
			}),
			TextCtrl{std::string("fixed")}.onChange([&](const std::string& v) {
				++shortCalls;
				unboundSeen = v;
			})
		}
	}.show();
	pump();
	QWidget* w = windowTitled("Callbacks");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	const auto edits = w->findChildren<QLineEdit*>();
	CHECK_EQ(edits.size(), 2);
	if (edits.size() != 2)
		return;
	QLineEdit* bound = edits[0]->text() == "start" ? edits[0] : edits[1];
	QLineEdit* unbound = bound == edits[0] ? edits[1] : edits[0];

	bound->setText("typed");
	pump();
	CHECK_EQ(text, std::string("typed"));
	CHECK_EQ(seenValue, std::string("typed"));
	CHECK(seenNative == bound);

	unbound->setText("other");
	pump();
	CHECK_EQ(shortCalls, 1);
	CHECK_EQ(unboundSeen, std::string("other"));
	w->close();
	pump();
}

// Review finding 7: RefSync ran one 16 ms QTimer per bound property. It now runs
// one per window, and every binding under it still follows external writes.
TEST(qt_ref_sync_runs_one_timer_per_window)
{
	bool a = false;
	bool b = false;
	std::string text = "one";
	Dialog { "Clock",
		VStack {
			CheckBox{a, "A"},
			CheckBox{b, "B"},
			TextCtrl{text}
		}
	}.show();
	pump();
	QWidget* w = windowTitled("Clock");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	CHECK_EQ(w->findChildren<QTimer*>().size(), 1);

	a = true;
	b = true;
	text = "two";
	pump();
	int checked = 0;
	for (auto* box : w->findChildren<QCheckBox*>())
		checked += box->isChecked() ? 1 : 0;
	CHECK_EQ(checked, 2);
	bool sawText = false;
	for (auto* edit : w->findChildren<QLineEdit*>())
		sawText |= edit->text() == QStringLiteral("two");
	CHECK(sawText);
	w->close();
	pump();
}

// Review finding 9: Qt group-box and tab insets were guessed from the font, so
// content drifted against the frame Qt draws. They now come from the style's
// own contents rectangles: an expanding child fills its tab page exactly and
// sits exactly inside the group box's contents rect.
TEST(qt_container_insets_match_the_style)
{
	std::string text = "x";
	std::string other = "y";
	Dialog { "Insets",
		VStack {
			TabPanel {
				LayoutFlags().MinSize({300, 150}),
				Tab { "Page",
					VStack {
						LayoutFlags().Expand().Proportion(1),
						MultiLineTextCtrl{text}.withFlags(LayoutFlags().Expand().Proportion(1))
					}
				}
			},
			VGroupBox { "Box",
				LayoutFlags().MinSize({300, 120}),
				MultiLineTextCtrl{other}.withFlags(LayoutFlags().Expand().Proportion(1))
			}
		}
	}.show();
	pump();
	QWidget* w = windowTitled("Insets");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;

	auto* tabs = w->findChild<QTabWidget*>();
	CHECK(tabs != nullptr);
	QPlainTextEdit* inPage = nullptr;
	QPlainTextEdit* inBox = nullptr;
	for (auto* e : w->findChildren<QPlainTextEdit*>())
		(e->toPlainText() == "x" ? inPage : inBox) = e;
	CHECK(inPage != nullptr && inBox != nullptr);
	if (tabs == nullptr || inPage == nullptr || inBox == nullptr)
		return;

	// The page is whatever QTabWidget made it; the child must fill it.
	QWidget* page = tabs->currentWidget();
	CHECK(inPage->parentWidget() == page);
	CHECK_EQ(inPage->geometry().bottom(), page->rect().bottom());
	CHECK_EQ(inPage->geometry().right(), page->rect().right());

	// The box's child is a sibling placed over it: it must fill the contents
	// rect the style gives the box, in the box's own coordinates.
	auto* box = w->findChild<QGroupBox*>();
	CHECK(box != nullptr);
	if (box != nullptr)
	{
		QStyleOptionGroupBox option;
		option.initFrom(box);
		option.text = box->title();
		option.lineWidth = 1;
		option.subControls = QStyle::SC_GroupBoxFrame | QStyle::SC_GroupBoxLabel;
		option.rect = box->rect();
		const QRect contents = box->style()->subControlRect(QStyle::CC_GroupBox, &option,
			QStyle::SC_GroupBoxContents, box);
		// Siblings, so both are mapped through the dialog rather than one onto
		// the other (mapTo() requires an ancestor).
		const QRect child(inBox->mapTo(w, QPoint(0, 0)) - box->mapTo(w, QPoint(0, 0)), inBox->size());
		CHECK(contents.contains(child));
		CHECK_EQ(child.bottom(), contents.bottom());
	}
	w->close();
	pump();
}

// Review finding 9: see the wx twin -- a 1 px Separator and the shared
// StatusBar width rule, where Qt used to measure 3 px and its native bar.
TEST(qt_separator_and_status_bar_follow_the_shared_rule)
{
	Dialog { "Rules",
		VStack {
			Separator{}.withFlags(LayoutFlags().Expand()),
			StatusBar { StatusField{"fixed", 100}, StatusField{"stretch"} }
				.withFlags(LayoutFlags())
		}
	}.show();
	pump();
	QWidget* w = windowTitled("Rules");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	QFrame* line = nullptr;
	for (auto* f : w->findChildren<QFrame*>())
		if (f->frameShape() == QFrame::HLine)
			line = f;
	auto* bar = w->findChild<QStatusBar*>();
	CHECK(line != nullptr && line->height() == 1);
	CHECK(bar != nullptr && bar->width() == statusBarContentWidth({ StatusField{"a", 100}, StatusField{"b"} }));
	w->close();
	pump();
}

// withId() names the native object; postToUi() runs a worker's task on the UI
// thread.
TEST(qt_with_id_names_the_widget_and_post_to_ui_crosses_threads)
{
	bool flag = false;
	Dialog { "Named", VStack { CheckBox{flag, "Box"}.withId("named-box") } }.show();
	pump();
	QWidget* w = windowTitled("Named");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	CHECK(w->findChild<QCheckBox*>(QStringLiteral("named-box")) != nullptr);

	const auto uiThread = std::this_thread::get_id();
	bool onUiThread = false;
	std::thread([&] {
		postToUi([&] { flag = true; onUiThread = std::this_thread::get_id() == uiThread; });
	}).join();
	pump();
	CHECK(flag);
	CHECK(onUiThread);
	CHECK(w->findChild<QCheckBox*>(QStringLiteral("named-box"))->isChecked()); // and RefSync mirrored it
	w->close();
	pump();
}

// A bound StaticText / ReadonlyTextCtrl follows the caller's string but keeps
// the size of its first text, even through a full re-measure (an Expander
// opening forces one).
TEST(qt_bound_labels_follow_text_without_resizing)
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
	QWidget* w = windowTitled("Status");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	QLabel* label = nullptr;
	for (auto* l : w->findChildren<QLabel*>())
		if (l->text() == "Ready")
			label = l;
	auto* field = w->findChild<QLineEdit*>();
	CHECK(label != nullptr && field != nullptr);
	if (label == nullptr || field == nullptr)
		return;
	const int labelWidth = label->width();

	status = "This status message is far longer than the label it arrives in, by design";
	open = true;
	pump(200);
	CHECK(label->text().toStdString() == status);
	CHECK(field->text().toStdString() == status);
	CHECK_EQ(label->width(), labelWidth);
	w->close();
	pump();
}

// isHidden(): a hidden control and a hidden group box are taken out of view
// and out of the layout; flipping the bound flag back restores both.
TEST(qt_hidden_nodes_leave_and_return)
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
	QWidget* w = windowTitled("Hide");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	QPushButton* sometimes = nullptr;
	QPushButton* inside = nullptr;
	for (auto* b : w->findChildren<QPushButton*>())
	{
		if (b->text() == "Sometimes") sometimes = b;
		if (b->text() == "Inside") inside = b;
	}
	auto* box = w->findChild<QGroupBox*>();
	CHECK(sometimes && inside && box);
	if (!(sometimes && inside && box))
		return;
	const int fullHeight = w->height();

	hideButton = true;
	hideBox = true;
	pump(200);
	CHECK(!sometimes->isVisible());
	CHECK(!box->isVisible());
	CHECK(!inside->isVisible());
	CHECK(w->height() < fullHeight);

	hideButton = false;
	hideBox = false;
	pump(200);
	CHECK(sometimes->isVisible());
	CHECK(box->isVisible());
	CHECK(inside->isVisible());
	CHECK_EQ(w->height(), fullHeight);
	w->close();
	pump();
}

// Bound item lists: the native list and combo are repopulated when the
// caller's vector changes, the selection stays on the same item by value, and
// the window does not resize.
TEST(qt_bound_item_lists_repopulate_and_keep_the_selection)
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
	QWidget* w = windowTitled("Items");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	auto* list = w->findChild<QListWidget*>();
	auto* combo = w->findChild<QComboBox*>();
	CHECK(list && combo);
	if (!(list && combo))
		return;
	const int width = w->width();

	items = { "gamma", "a considerably longer new item", "beta" };
	pump(200);
	CHECK_EQ(list->count(), 3);
	CHECK_EQ(combo->count(), 3);
	CHECK(list->selectedItems().size() == 1 && list->selectedItems().front()->text() == "beta");
	CHECK(combo->currentText() == "gamma");
	CHECK_EQ(listPick, std::string("beta"));
	CHECK_EQ(w->width(), width);
	w->close();
	pump();
}

// VForEach: rows follow a bound vector. Adding one builds its widgets; a row's
// own Remove button removes that row, and its widgets are destroyed, not left
// behind in the window.
TEST(qt_foreach_rows_follow_the_vector)
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
	QWidget* w = windowTitled("Todos");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	const int twoRows = w->height();
	CHECK_EQ(w->findChildren<QPushButton*>().size(), 2);

	todos.push_back("three");
	pump(200);
	CHECK_EQ(w->findChildren<QPushButton*>().size(), 3);
	CHECK(w->height() > twoRows);

	QPushButton* removeOne = nullptr;
	for (auto* b : w->findChildren<QPushButton*>())
		if (b->text() == "Remove one")
			removeOne = b;
	CHECK(removeOne != nullptr);
	if (removeOne != nullptr)
		removeOne->click();
	pump(200);
	CHECK_EQ(todos.size(), static_cast<std::size_t>(2));
	const auto buttons = w->findChildren<QPushButton*>();
	CHECK_EQ(buttons.size(), 2);
	bool oneGone = true;
	for (auto* b : buttons)
		oneGone &= b->text() != "Remove one";
	CHECK(oneGone);
	CHECK_EQ(w->height(), twoRows);
	w->close();
	pump();
}

// RadioGroup: one radio per option, bound to the index, in a row when asked;
// a write from outside moves the check, onChange reports after the write.
TEST(qt_radio_group_binds_an_index)
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
	QWidget* w = windowTitled("Group");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;

	CHECK(radio(w, "Red")->isChecked());
	// a row: every radio on one line
	CHECK_EQ(radio(w, "Red")->y(), radio(w, "Blue")->y());
	CHECK(radio(w, "Red")->x() < radio(w, "Blue")->x());

	radio(w, "Blue")->click();
	pump();
	CHECK_EQ(choice, 2);
	CHECK_EQ(reported, 2);
	CHECK_EQ(seen, 2);
	CHECK(!radio(w, "Red")->isChecked());

	choice = 1;
	pump();
	CHECK(radio(w, "Green")->isChecked());
	CHECK(!radio(w, "Blue")->isChecked());

	w->close();
	pump();
}

// Default / cancel buttons: Enter in a single-line field reports onEnter and
// then presses the default button; Escape presses cancel instead of closing
// the dialog; Enter in a multi-line field or on a plain button behaves as
// described in DialogKeys.hpp; a disabled default button is skipped.
TEST(qt_default_and_cancel_buttons_answer_enter_and_escape)
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
			Button{"Other"}.onClick([&] { log.push_back("other"); }),
			MultiLineTextCtrl{notes}
		}
	}.show();
	pump();
	QWidget* w = windowTitled("Keys");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	auto send = [](QWidget* target, int k) {
		QKeyEvent press(QEvent::KeyPress, k, Qt::NoModifier);
		QApplication::sendEvent(target, &press);
	};
	auto* edit = w->findChild<QLineEdit*>();
	auto* multi = w->findChild<QPlainTextEdit*>();
	QPushButton* other = nullptr;
	QPushButton* ok = nullptr;
	for (auto* b : w->findChildren<QPushButton*>())
	{
		if (b->text() == "Other")
			other = b;
		if (b->text() == "OK")
			ok = b;
	}
	CHECK(edit != nullptr && multi != nullptr && other != nullptr && ok != nullptr);
	if (edit == nullptr || multi == nullptr || other == nullptr || ok == nullptr)
		return;
	CHECK(ok->isDefault()); // the native look

	send(edit, Qt::Key_Return);
	CHECK_EQ(log.size(), std::size_t(2));
	if (log.size() == 2)
	{
		CHECK(log[0] == "enter:cats");
		CHECK(log[1] == "ok");
	}

	log.clear();
	send(edit, Qt::Key_Escape);
	pump();
	CHECK_EQ(log.size(), std::size_t(1));
	CHECK(!log.empty() && log[0] == "cancel");
	CHECK(windowTitled("Keys") == w); // pressed Cancel, did not reject()

	// a focused plain button passes Enter on to the default button
	log.clear();
	send(other, Qt::Key_Return);
	CHECK_EQ(log.size(), std::size_t(1));
	CHECK(!log.empty() && log[0] == "ok");

	log.clear();
	send(multi, Qt::Key_Return);
	CHECK(log.empty());

	okDisabled = true;
	pump();
	log.clear();
	send(edit, Qt::Key_Return);
	CHECK_EQ(log.size(), std::size_t(1)); // onEnter only
	w->close();
	pump();
}

// withIcon: the icon is set at the asked size and widens the button (a long
// label, since macOS pads a short one to a minimum width anyway); a path
// that fails to load leaves a plain text button.
TEST(qt_icon_button_carries_its_icon)
{
	Dialog { "Icons",
		VStack {
			Button{"Save the whole document"},
			Button{"Save the whole document "}.withIcon(DUI_TEST_ICON, { 20, 20 }),
			Button{"Save the whole document  "}.withIcon("/no/such/icon.png")
		}
	}.show();
	pump();
	QWidget* w = windowTitled("Icons");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	QPushButton* plain = nullptr;
	QPushButton* icon = nullptr;
	QPushButton* broken = nullptr;
	for (auto* b : w->findChildren<QPushButton*>())
	{
		if (b->text() == "Save the whole document")
			plain = b;
		else if (b->text() == "Save the whole document ")
			icon = b;
		else if (b->text() == "Save the whole document  ")
			broken = b;
	}
	CHECK(plain != nullptr && icon != nullptr && broken != nullptr);
	if (plain == nullptr || icon == nullptr || broken == nullptr)
		return;
	CHECK(!icon->icon().isNull());
	CHECK(icon->iconSize() == QSize(20, 20));
	CHECK(icon->sizeHint().width() > plain->sizeHint().width());
	CHECK(broken->icon().isNull());
	w->close();
	pump();
}

// Bound TreeView items and Table rows follow the caller's data: refilled,
// selection kept by path / key, the user's open state kept for items that are
// still there and a new item's own flag honoured, the window not resized.
TEST(qt_bound_tree_and_table_follow_their_data)
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
	QWidget* w = windowTitled("Data");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	auto* tw = w->findChild<QTreeWidget*>();
	auto* table = w->findChild<QTableWidget*>();
	CHECK(tw != nullptr && table != nullptr);
	if (tw == nullptr || table == nullptr)
		return;
	const QSize size = w->size();
	tw->topLevelItem(1)->setExpanded(true); // the user opens "Veg"

	tree[0].children.push_back({ "Cherry with a very long name indeed" });
	tree.push_back({ "Nuts", { { "Almond" } }, true });
	rows.insert(rows.begin(), { "a much longer file name than any before.txt", "3" });
	pump(200);

	CHECK_EQ(tw->topLevelItemCount(), 3);
	CHECK_EQ(tw->topLevelItem(0)->childCount(), 3);
	CHECK(!tw->topLevelItem(0)->isExpanded()); // left closed by the user
	CHECK(tw->topLevelItem(1)->isExpanded());  // opened by the user: kept
	CHECK(tw->topLevelItem(2)->isExpanded());  // new: its own flag
	CHECK(tw->selectedItems().size() == 1 && tw->selectedItems().front()->text(0) == "Banana");
	CHECK_EQ(treePick, std::string("Fruits/Banana"));

	CHECK_EQ(table->rowCount(), 3);
	const auto picked = table->selectedItems();
	CHECK(!picked.isEmpty() && table->item(picked.front()->row(), 0)->text() == "b.txt");
	CHECK_EQ(rowPick, std::string("b.txt"));
	CHECK(w->size() == size);

	rows[1][1] = "42"; // a cell written from outside
	pump(200);
	CHECK(!table->findItems("42", Qt::MatchExactly).isEmpty());
	w->close();
	pump();
}

// Focus and validity: the starting focus, a bound flag that moves focus and
// follows it, onFocus / onBlur, and isInvalid tinting the palette's Base.
TEST(qt_text_field_focus_follows_the_flag_and_reports)
{
	std::string name;
	std::string email;
	bool emailFocused = false;
	bool emailInvalid = false;
	std::vector<std::string> log;
	Dialog { "Focus",
		VStack {
			TextCtrl{name}.isFocused(),
			TextCtrl{email}
				.isFocused(emailFocused)
				.isInvalid(emailInvalid)
				.onFocus([&] { log.push_back("focus"); })
				.onBlur([&] { log.push_back("blur"); })
		}
	}.show();
	pump();
	QWidget* w = windowTitled("Focus");
	CHECK(w != nullptr);
	if (w == nullptr)
		return;
	w->activateWindow();
	pump();
	const auto edits = w->findChildren<QLineEdit*>();
	CHECK_EQ(edits.size(), 2);
	if (edits.size() != 2)
		return;
	QLineEdit* first = edits[0]->text().isEmpty() && edits[0]->y() < edits[1]->y() ? edits[0] : edits[1];
	QLineEdit* second = first == edits[0] ? edits[1] : edits[0];
	CHECK(QApplication::focusWidget() == first);

	emailFocused = true;
	pump(200);
	CHECK(QApplication::focusWidget() == second);
	CHECK(!log.empty() && log[0] == "focus");
	CHECK(emailFocused);

	first->setFocus();
	pump(200);
	CHECK(log.size() == 2 && log[1] == "blur");
	CHECK(!emailFocused);

	const QColor base = second->palette().color(QPalette::Base);
	emailInvalid = true;
	pump(200);
	CHECK(second->palette().color(QPalette::Base) == QColor(253, 228, 228));
	emailInvalid = false;
	pump(200);
	CHECK(second->palette().color(QPalette::Base) == base);
	w->close();
	pump();
}

int main(int argc, char** argv)
{
	// No display on CI, and none needed: every check reads widget state.
	if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
		qputenv("QT_QPA_PLATFORM", "offscreen");
	QApplication app(argc, argv);
	return testfw::runAll();
}
