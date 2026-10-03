#pragma once

#include <QAbstractListModel>
#include <QListView>

#include <functional>
#include <string>

// The native half of a VirtualList on Qt: a QListView over a model that owns
// no rows -- data() asks the caller's function for the one row being drawn.
// uniformItemSizes keeps the view from asking every row for its size, which
// is what would otherwise make a large list slow. No Q_OBJECT: the model only
// overrides virtuals and emits the signals QAbstractItemModel already has.
class VirtualListModel : public QAbstractListModel
{
public:
	VirtualListModel(std::function<std::string(int)> rowText, int count, QObject* parent)
		: QAbstractListModel(parent)
		, m_rowText(std::move(rowText))
		, m_count(count)
	{
	}

	int rowCount(const QModelIndex& parent = QModelIndex()) const override
	{
		return parent.isValid() ? 0 : m_count;
	}

	QVariant data(const QModelIndex& index, int role) const override
	{
		if (role != Qt::DisplayRole || !index.isValid() || !m_rowText)
			return {};
		return QString::fromStdString(m_rowText(index.row()));
	}

	int count() const { return m_count; }

	// A new count: the view is reset (the caller re-applies the selection).
	void setCount(int count)
	{
		beginResetModel();
		m_count = count;
		endResetModel();
	}

	// Rows changed in place: every row is asked again when next drawn.
	void refresh()
	{
		if (m_count > 0)
			emit dataChanged(index(0), index(m_count - 1), { Qt::DisplayRole });
	}

private:
	std::function<std::string(int)> m_rowText;
	int m_count;
};
