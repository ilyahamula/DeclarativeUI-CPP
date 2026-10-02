#pragma once

#include "frameworks_core/CoreTypes/TextField.hpp"
#include "frameworks_core/qt/RefSync.hpp"

#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QWidget>

#include <memory>

// Focus and validity of a Qt text field (TextField.hpp).
namespace qt_text_field
{

// Whether focus is in `field` -- the field itself or, for a scroll-area based
// editor, anything inside it.
inline bool contains(const QWidget* field, const QWidget* widget)
{
	return widget != nullptr && (widget == field || field->isAncestorOf(widget));
}

inline void apply(QWidget* field, TextFieldOptions options)
{
	// Invalid: the palette's Base, never a style sheet -- a style sheet
	// changes a field's size hint on most styles, and marking a field must
	// not move anything. The untouched palette is kept to restore.
	const QPalette original = field->palette();
	auto showInvalid = [field, original](bool invalid) {
		QPalette palette = original;
		if (invalid)
			palette.setColor(QPalette::Base, QColor(253, 228, 228));
		field->setPalette(palette);
	};
	if (options.invalid.get())
		showInvalid(true);
	if (options.invalid.isBound())
	{
		auto shown = std::make_shared<bool>(options.invalid.get());
		const bool& invalid = options.invalid.get();
		bindExternalRefSync(field,
			[shown] { return *shown; },
			[&invalid] { return invalid; },
			[shown, showInvalid](bool next) {
				*shown = next;
				showInvalid(next);
			});
	}

	// Focus moves, from the application's one signal: it reports the widget
	// focus left and the one it reached, so "into" and "out of" the field
	// are both one comparison, composite editors included.
	bool* focusFlag = options.focused.isBound() ? &options.focused.get() : nullptr;
	if (options.onFocus || options.onBlur || focusFlag != nullptr)
	{
		QObject::connect(qApp, &QApplication::focusChanged, field,
			[field, focusFlag, onFocus = std::move(options.onFocus), onBlur = std::move(options.onBlur)](
				QWidget* from, QWidget* to) {
				const bool was = contains(field, from);
				const bool is = contains(field, to);
				if (was == is)
					return;
				if (focusFlag != nullptr)
				{
					*focusFlag = is;
					markChanged(focusFlag);
				}
				if (is && onFocus)
					onFocus();
				else if (!is && onBlur)
					onBlur();
			});
	}

	// Focus requests: the window's starting focus (a snapshot true), and a
	// bound flag the caller sets true. setFocus() on a window not yet shown
	// makes the field the one it opens with.
	if (options.focused.get())
		field->setFocus();
	if (focusFlag != nullptr)
	{
		bindExternalRefSync(field,
			[field] { return contains(field, QApplication::focusWidget()); },
			[focusFlag] { return *focusFlag; },
			[field](bool want) {
				if (want)
					field->setFocus();
			});
	}
}

} // namespace qt_text_field
