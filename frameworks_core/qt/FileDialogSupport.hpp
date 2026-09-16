#pragma once

#include "frameworks_core/CoreTypes/FileFilter.hpp"
#include "frameworks_core/CoreTypes/GeneralTypes.hpp"

#include <string>
#include <vector>

#include <QFileDialog>
#include <QFileInfo>
#include <QString>
#include <QStringList>

// The Qt half of T3.1's file support, shared by FileDialogWrapper (the one-shot
// FileDialog) and FilePickerWrapper (the Browse button), exactly as the wx
// header beside it is.

// Qt's name-filter spelling, built from the parsed fields.
//
// Qt disagrees with wx on both separators -- patterns inside a group are
// separated by SPACES and groups by ";;", and the description carries its own
// patterns in brackets -- which is the whole reason FileFilter is parsed once
// into fields rather than passed around as a string.
inline QString qtNameFilterFor(const std::vector<FileFilter>& filters)
{
	QStringList groups;
	for (const FileFilter& filter : filters)
	{
		QStringList patterns;
		for (const std::string& pattern : filter.patterns)
			patterns << QString::fromStdString(pattern);
		groups << QStringLiteral("%1 (%2)")
			.arg(QString::fromStdString(filter.description), patterns.join(QLatin1Char(' ')));
	}
	return groups.join(QStringLiteral(";;"));
}

// Run the modal dialog for `mode` and return the chosen path, or "" on cancel.
//
// The blocking static calls, deliberately, so the control flow matches wx's:
// onChange fires before Browse returns on both retained backends. They spin
// Qt's own nested event loop, which is safe here because a Browse click is an
// EVENT -- the engine's measure/arrange pass is long finished by then.
inline std::string qtRunFileDialog(QWidget* parent, const std::string& title,
	FileMode mode, const std::vector<FileFilter>& filters, const std::string& initialPath)
{
	const QString caption = QString::fromStdString(title);
	const QString initial = QString::fromStdString(initialPath);
	const QString filter = qtNameFilterFor(filters);

	QString chosen;
	switch (mode)
	{
	case FileMode::Directory:
		chosen = QFileDialog::getExistingDirectory(parent, caption, initial);
		break;
	case FileMode::Save:
		chosen = QFileDialog::getSaveFileName(parent, caption, initial, filter);
		break;
	case FileMode::Open:
		chosen = QFileDialog::getOpenFileName(parent, caption, initial, filter);
		break;
	}
	return chosen.isEmpty() ? std::string() : chosen.toStdString();
}
