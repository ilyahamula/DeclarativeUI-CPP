#pragma once

#include <string>

#include <QLabel>
#include <QString>

// Caller text shown as a Qt control's LABEL. Qt reads '&' as the mnemonic
// marker on buttons, check boxes, radio buttons, tool buttons, group-box and
// tab titles and actions, so "Save & Exit" would lose its ampersand and bind
// Alt+Space. The framework offers no way to ask for a mnemonic, so every '&'
// the caller wrote is literal -- the Qt twin of wx/Labels.hpp.
inline QString qtLabelText(const std::string& text)
{
	return QString::fromStdString(text).replace(QLatin1String("&"), QLatin1String("&&"));
}

// A QLabel that shows caller text as TEXT. QLabel defaults to Qt::AutoText,
// which renders anything that looks like HTML ("<b>", "<img src=...>") as rich
// text -- on Qt only, and from strings that may be user data such as file
// names or messages. wx and ImGui always draw the characters, so Qt does too.
// A plain-text QLabel with no buddy shows '&' literally, so no escaping here.
inline void qtSetPlainText(QLabel* label, const std::string& text)
{
	label->setTextFormat(Qt::PlainText);
	label->setText(QString::fromStdString(text));
}
