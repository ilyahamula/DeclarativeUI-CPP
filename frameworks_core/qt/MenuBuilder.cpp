#include "frameworks_core/qt/MenuBuilder.hpp"

#include "frameworks_core/qt/RefSync.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#include <QAction>
#include <QKeyCombination>
#include <QKeySequence>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QObject>
#include <QString>

#include <string>
#include <utility>

namespace
{

// The copied model. Lives until the window is destroyed, because every slot and
// RefSync poll below holds a MenuItem* into it and nothing mutates the vectors
// after the bar is built.
struct MenuHost
{
	MenuBarModel model;
};

// Qt treats '&' as the mnemonic marker in menu text exactly as wx does, so
// caller text escapes it the same way qt/LayoutBackend.cpp escapes container
// labels.
QString escaped(const std::string& text)
{
	return QString::fromStdString(text).replace(QLatin1String("&"), QLatin1String("&&"));
}

// Qt's own key table. Built from the parsed fields rather than from
// QKeySequence::fromString, whose accepted spellings are Qt's translated key
// names and agree with neither wx's nor ImGui's.
//
// Qt::ControlModifier is Cmd on macOS unless the application asks otherwise,
// which is the same remap wx and ImGui do -- so "Ctrl+S" reads native on every
// platform without a special case here. `super` is deliberately absent: it is
// display-only on every backend (CoreTypes/Shortcut.hpp).
bool qtKeyFor(const std::string& key, Qt::Key& out)
{
	if (key.size() == 1)
	{
		const char c = key.front();
		if (c >= 'A' && c <= 'Z')
		{
			out = (Qt::Key)(Qt::Key_A + (c - 'A'));
			return true;
		}
		if (c >= '0' && c <= '9')
		{
			out = (Qt::Key)(Qt::Key_0 + (c - '0'));
			return true;
		}
		return false;
	}

	if (key.size() >= 2 && key[0] == 'F')
	{
		const int number = std::stoi(key.substr(1));
		if (number >= 1 && number <= 12)
		{
			out = (Qt::Key)(Qt::Key_F1 + (number - 1));
			return true;
		}
		return false;
	}

	if (key == "Enter")          { out = Qt::Key_Return;    return true; }
	if (key == "Escape")         { out = Qt::Key_Escape;    return true; }
	if (key == "Delete")         { out = Qt::Key_Delete;    return true; }
	if (key == "Insert")         { out = Qt::Key_Insert;    return true; }
	if (key == "Home")           { out = Qt::Key_Home;      return true; }
	if (key == "End")            { out = Qt::Key_End;       return true; }
	if (key == "PageUp")         { out = Qt::Key_PageUp;    return true; }
	if (key == "PageDown")       { out = Qt::Key_PageDown;  return true; }
	if (key == "Tab")            { out = Qt::Key_Tab;       return true; }
	if (key == "Space")          { out = Qt::Key_Space;     return true; }
	if (key == "Backspace")      { out = Qt::Key_Backspace; return true; }
	if (key == "Left")           { out = Qt::Key_Left;      return true; }
	if (key == "Right")          { out = Qt::Key_Right;     return true; }
	if (key == "Up")             { out = Qt::Key_Up;        return true; }
	if (key == "Down")           { out = Qt::Key_Down;      return true; }
	if (key == "Plus")           { out = Qt::Key_Plus;      return true; }
	if (key == "Minus")          { out = Qt::Key_Minus;     return true; }
	if (key == "Comma")          { out = Qt::Key_Comma;     return true; }
	if (key == "Period")         { out = Qt::Key_Period;    return true; }
	if (key == "Slash")          { out = Qt::Key_Slash;     return true; }
	return false;
}

void applyShortcut(QAction* action, const MenuItem& item)
{
	Qt::Key key = Qt::Key_unknown;
	if (!item.shortcut || !qtKeyFor(item.shortcut->key, key))
	{
#ifdef USE_LOGGER
		if (!item.shortcutText.empty())
			Logger::instance().log("attachMenuBar()\t-> unparseable shortcut \""
				+ item.shortcutText + "\" on \"" + item.label + "\": no accelerator registered\n");
#endif
		return;
	}

	Qt::KeyboardModifiers mods = Qt::NoModifier;
	if (item.shortcut->ctrl)
		mods |= Qt::ControlModifier;
	if (item.shortcut->alt)
		mods |= Qt::AltModifier;
	if (item.shortcut->shift)
		mods |= Qt::ShiftModifier;
	action->setShortcut(QKeySequence(QKeyCombination(mods, key)));
}

void buildMenu(QMenu* menu, std::vector<MenuItem>& items, QMainWindow* window);

void appendItem(QMenu* menu, MenuItem& item, QMainWindow* window)
{
	if (item.isSeparator)
	{
		menu->addSeparator();
		return;
	}

	if (!item.submenu.empty())
	{
		// A submenu's own item is a container: it opens, it never selects.
		QMenu* sub = menu->addMenu(escaped(item.label));
		buildMenu(sub, item.submenu, window);
		return;
	}

	QAction* action = menu->addAction(escaped(item.label));
	applyShortcut(action, item);

	MenuItem* model = &item;
	if (model->checkedFlag)
	{
		action->setCheckable(true);
		action->setChecked(model->checkedFlag->get());
	}
	action->setEnabled(!model->disabledFlag.get());

	QObject::connect(action, &QAction::triggered, window, [model, action](bool) {
		// Value first, then the callback, so a handler reading the bound bool
		// sees the state the user just selected.
		if (model->checkedFlag)
			model->checkedFlag->set(action->isChecked());
		if (model->selectHandler)
			model->selectHandler();
	});

	// Bound flags are polled, never pushed on disagreement -- the shape every
	// externally-written value uses here. RefSync's QSignalBlocker guards the
	// window rather than the action, but nothing needs guarding: setChecked()
	// emits toggled(), and the handler above listens to triggered().
	if (model->checkedFlag && model->checkedFlag->isBound())
	{
		const bool* flag = model->checkedFlag->boundValue();
		bindExternalRefSync(window,
			[action] { return action->isChecked(); },
			[flag] { return *flag; },
			[action](bool value) { action->setChecked(value); });
	}
	if (model->disabledFlag.isBound())
	{
		const bool* flag = model->disabledFlag.boundValue();
		bindExternalRefSync(window,
			[action] { return action->isEnabled(); },
			[flag] { return !*flag; },
			[action](bool enabled) { action->setEnabled(enabled); });
	}
}

void buildMenu(QMenu* menu, std::vector<MenuItem>& items, QMainWindow* window)
{
	for (auto& item : items)
		appendItem(menu, item, window);
}

} // unnamed namespace

int attachMenuBar(QMainWindow* window, const MenuBarModel& model)
{
	if (model.empty())
		return 0;

#ifdef USE_LOGGER
	Logger::instance().log("attachMenuBar()\t-> new QMenuBar()\n");
#endif
	auto* host = new MenuHost;
	host->model = model; // the caller's Window is a temporary; this copy is ours

	auto* bar = new QMenuBar(window);
	for (auto& menuModel : host->model.menus)
	{
		QMenu* menu = bar->addMenu(escaped(menuModel.label));
		buildMenu(menu, menuModel.items, window);
	}
	window->setMenuBar(bar);

	QObject::connect(window, &QObject::destroyed, [host] { delete host; });

	// A native (macOS) bar lives in the system menu and costs the window no
	// height at all; an in-window one costs exactly its own.
	return bar->isNativeMenuBar() ? 0 : bar->sizeHint().height();
}
