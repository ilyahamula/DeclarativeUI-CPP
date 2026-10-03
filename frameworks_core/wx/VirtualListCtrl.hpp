#pragma once

#include <wx/listctrl.h>
#include <wx/wx.h>

#include <functional>
#include <string>

// The native half of a VirtualList on wx: a single-column, header-less report
// wxListCtrl in wxLC_VIRTUAL mode, which owns no items -- it asks
// OnGetItemText for the rows it is about to draw. The one column always spans
// the client width, so there is never a horizontal scroll bar for a column
// nobody can resize.
class VirtualListCtrl : public wxListCtrl
{
public:
	VirtualListCtrl(wxWindow* parent, std::function<std::string(int)> rowText, const wxPoint& pos,
		const wxSize& size, long style)
		: wxListCtrl(parent, wxID_ANY, pos, size,
			style | wxLC_REPORT | wxLC_VIRTUAL | wxLC_NO_HEADER | wxLC_SINGLE_SEL)
		, m_rowText(std::move(rowText))
	{
		InsertColumn(0, wxString());
		Bind(wxEVT_SIZE, [this](wxSizeEvent& event) {
			SetColumnWidth(0, std::max(1, GetClientSize().x));
			event.Skip();
		});
	}

	// The selected row, -1 for none.
	int selectedRow() const { return static_cast<int>(GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)); }

	// Select one row (or none) and bring it into view. Callers that also
	// listen to the selection events hold their own guard: the generic
	// control reports programmatic selection like a click.
	void selectRow(int row)
	{
		const long current = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (current >= 0 && current != row)
			SetItemState(current, 0, wxLIST_STATE_SELECTED);
		if (row >= 0 && row < GetItemCount())
		{
			SetItemState(row, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED,
				wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
			EnsureVisible(row);
		}
	}

protected:
	wxString OnGetItemText(long item, long) const override
	{
		return m_rowText ? wxString::FromUTF8(m_rowText(static_cast<int>(item))) : wxString();
	}

private:
	std::function<std::string(int)> m_rowText;
};
