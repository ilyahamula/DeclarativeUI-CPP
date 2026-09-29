// Backend behaviour tests for Qt, run on the offscreen platform plugin so no
// display is needed. Windows are real Qt widgets; the event loop is pumped by
// hand with wall-clock time passing, because RefSync polls on a QTimer.

#include "../test_framework.hpp"

#include "declarative_ui.hpp"

#include <QApplication>

#include <thread>
#include <QElapsedTimer>
#include <QCheckBox>
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
#include <QTimer>

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

int main(int argc, char** argv)
{
	// No display on CI, and none needed: every check reads widget state.
	if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
		qputenv("QT_QPA_PLATFORM", "offscreen");
	QApplication app(argc, argv);
	return testfw::runAll();
}
