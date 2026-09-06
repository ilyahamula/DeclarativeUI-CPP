#pragma once

// Tabular data: the gallery dialog for Table.
//
// controls_gallery.hpp is long past the ~20-element mark rule 4 sets, so Table
// lands here instead, alongside a few already-implemented controls for context
// (StaticText, TextCtrl, CheckBox, Button).
//
// The two tables show both halves of the Table binding. A row is named either by
// its ORIGINAL index (int / std::vector<int>) or by the text of its first column
// (std::string / std::vector<std::string>), and -- as with ListBox -- the bound
// type is what picks single- vs multi-select.
//
// "Original" is the load-bearing word. Clicking a sortable header reorders what
// is on screen but not the data, so row 2 stays row 2 whatever the view is
// sorted by. That is why the index binding is the robust one: a key binding
// cannot tell two rows with the same first column apart, and editing that
// column changes which row the binding names.
//
// Sorting note: all three backends sort lexicographically on the cell TEXT, so
// "10" sorts before "9" everywhere -- a table holds text, and agreeing on one
// order matters more than guessing at numbers in some columns.
//
// Editing note: the Notes column is editable, and `rows` is passed as a non-const
// lvalue, which BINDS it -- that is what lets an edit write back to the caller's
// data. Double-click a Notes cell on any backend to edit it. Pass rows as a
// const or temporary instead and edits reach you through onCellChange() only;
// on ImGui they would not even survive to the next frame, since the tree is
// rebuilt from the caller's data every time.

#include "declarative_ui.hpp"

#include <string>
#include <vector>

inline auto drawTableUI(
    TableRows& rows,
    int& selectedRow,
    std::vector<int>& checkedRows,
    bool& tablesDisabled)
{
    // Same pinning discipline as the other galleries: explicit sizes on the
    // leaves and MinSize on the group boxes, so the three backends compute the
    // same frames despite their different native metrics.
    constexpr int kBoxW = 360;
    constexpr int kRowH = 26;
    constexpr int kLabelH = 20;
    constexpr int kTableH = 200;
    constexpr Size kButtonSize { 110, 28 };

    // Sortable where an order means something, editable only where the caller
    // owns the text. "Size" is pinned to 70px; the rest are measured from their
    // widest cell, which is what -1 asks for.
    //
    // The multi-select table below declares these same three columns with
    // addColumn() instead, which is the other half of the API: a vector is worth
    // it when a column set is shared, as it is here, and addColumn() reads
    // better when the columns belong to one table and most fields are defaults.
    const std::vector<TableColumn> columns {
        { "File",  -1, /*sortable*/ true },
        { "Size",  70, /*sortable*/ true },
        { "Notes", -1, /*sortable*/ false, /*editable*/ true },
    };

    return Dialog {
        "Table Controls",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 10),
            HStack {
                VGroupBox { "Single select (row index)",
                    LayoutFlags().MinSize({kBoxW, 300}),
                    StaticText{"Click a row, sort by File or Size:"}
                        .withSize({-1, kLabelH}),
                    // Bound to an int: the selection is one ORIGINAL row index,
                    // unchanged by sorting. -1 means nothing is selected.
                    Table { columns, rows, selectedRow }
                        .withVisibleRows(6)
                        .withSize({-1, kTableH})
                        .withFlags(LayoutFlags().Expand().Border(Side::Top, 5))
                        .isDisabled(tablesDisabled),
                    // Shows the bound index live -- the table writes through to
                    // the same int this spin box is bound to, so it also drives
                    // the selection when you type into it.
                    HStack {
                        LayoutFlags().Border(Side::Top, 8),
                        StaticText{"Selected row:"}
                            .withSize({95, kRowH})
                            .withFlags(LayoutFlags().CenterVertical()),
                        SpinBox { Range<int>{ .min = -1, .max = 99 }, selectedRow }
                            .withSize({80, kRowH})
                            .withFlags(LayoutFlags().Border(Side::Left, 6))
                            .isDisabled(tablesDisabled)
                    }
                },
                VGroupBox { "Multi select (row indices)",
                    LayoutFlags().MinSize({kBoxW, 300}).Border(Side::Left, 12),
                    StaticText{"Ctrl-click rows; double-click Notes to edit:"}
                        .withSize({-1, kLabelH}),
                    // Same columns and the same bound rows, a vector binding:
                    // unlike TreeView there is no isMultiSelect() to call, the
                    // bound type is the mode. Edits made here show up in the
                    // table on the left, because both read the caller's rows.
                    //
                    // Columns declared fluently rather than from `columns` above
                    // -- the two spellings build the same table. Either overload
                    // works: a braced TableColumn, or the fields unpacked with
                    // everything after the label defaulted.
                    Table { rows, checkedRows }
                        .addColumn("File", -1, /*sortable*/ true)
                        .addColumn({ "Size", 70, /*sortable*/ true })
                        .addColumn("Notes", -1, /*sortable*/ false, /*editable*/ true)
                        .withVisibleRows(6)
                        .withSize({-1, kTableH})
                        .withFlags(LayoutFlags().Expand().Border(Side::Top, 5))
                        .isDisabled(tablesDisabled),
                    CheckBox{tablesDisabled, "Disable both tables"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Border(Side::Top, 8))
                }
            },
            HStack {
                LayoutFlags().Border(Side::Top, 8),
                Spacer{},
                Button{"Close"}
                    .withSize(kButtonSize)
                    .withFlags(LayoutFlags().CenterVertical())
                    .onClick([]() {})
            }
        }
    };
}
