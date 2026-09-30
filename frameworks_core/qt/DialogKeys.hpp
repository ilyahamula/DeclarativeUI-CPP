#pragma once

#include "frameworks_core/CoreTypes/DialogKeys.hpp"

#include <QEvent>
#include <QKeyEvent>
#include <QObject>
#include <QPushButton>
#include <QVariant>
#include <QWidget>

// Enter / Escape for Button::isDefault() / isCancel() on Qt.
//
// An event filter on the top-level window (EngineSession::watch), for both
// window kinds: QDialog's own Enter/Escape handling does not exist on a
// QMainWindow, and QDialog's Escape is reject(), which would close the dialog
// instead of pressing the cancel button. A key reaches the window only after
// the focused control IGNORED it -- QApplication propagates ignored key events
// up the parent chain -- which is exactly the rule wanted: a QTextEdit keeps
// its newline, a QLineEdit reports returnPressed (onEnter) and then lets the
// key through, and the default button is pressed after it.
//
// Buttons are built with autoDefault off (ButtonWrapper), so a focused plain
// button passes Enter on to here rather than clicking itself.
namespace qt_dialog_keys
{

inline constexpr const char* kRoleProperty = "declarativeUiDialogKeys";

inline void markButton(QPushButton* button, unsigned roles)
{
	if (roles != kNoDialogKey)
		button->setProperty(kRoleProperty, roles);
}

// First enabled, visible button of `window` (not of a nested top-level)
// carrying `role`, in tree order.
inline QPushButton* findButton(QWidget* window, DialogKeyRole role)
{
	for (QPushButton* button : window->findChildren<QPushButton*>())
	{
		if (button->window() != window || !button->isVisible() || !button->isEnabled())
			continue;
		const QVariant roles = button->property(kRoleProperty);
		if (roles.isValid() && (roles.toUInt() & role) != 0)
			return button;
	}
	return nullptr;
}

class Filter : public QObject
{
public:
	explicit Filter(QWidget* window)
		: QObject(window)
		, m_window(window)
	{
	}

protected:
	bool eventFilter(QObject* watched, QEvent* event) override
	{
		if (watched != m_window || event->type() != QEvent::KeyPress)
			return false;
		auto* key = static_cast<QKeyEvent*>(event);
		const Qt::KeyboardModifiers mods = key->modifiers() & ~Qt::KeypadModifier;
		if (mods != Qt::NoModifier)
			return false;
		DialogKeyRole role = kNoDialogKey;
		if (key->key() == Qt::Key_Escape)
			role = kCancelButton;
		else if (key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter)
			role = kDefaultButton;
		if (role == kNoDialogKey)
			return false;
		QPushButton* button = findButton(m_window, role);
		if (button == nullptr)
			return false; // the window's own behaviour: a QDialog rejects on Escape
		button->click();
		return true;
	}

private:
	QWidget* m_window;
};

// Owned by the window.
inline void install(QWidget* window)
{
	window->installEventFilter(new Filter(window));
}

} // namespace qt_dialog_keys
