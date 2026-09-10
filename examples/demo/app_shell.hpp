#pragma once

// The application shell: the gallery dialog for Splitter.
//
// It is a Dialog for now and becomes a real Window when T2.1 lands, at which
// point the menu bar, tool bar and status bar join it here. The layout is the
// shape they will all hang off: a browser pane on the left, the work area on
// the right, and a draggable sash between them.
//
// controls_gallery.hpp is long past the ~20-element mark rule 4 sets, so the
// splitter lands here instead, alongside a few already-implemented controls for
// context (TreeView, Table, StaticText, CheckBox, Button).
//
// Two things about a Splitter are worth watching:
//
//   * There is no native splitter behind it. wxSplitterWindow and QSplitter own
//     their children's geometry, which is exactly what the layout engine exists
//     to take back, so all three backends get engine-arranged panes and one 6 px
//     leaf that knows how to be dragged. Drag the sash on each build and the
//     panes land on the same pixel.
//   * The position is the FIRST pane's extent along the splitter's own axis, and
//     it is the panes that re-measure when it changes -- not just the rectangles
//     that move. Narrow the tree pane far enough and the table's columns fold.
//
// Splitters nest: the right-hand pane here is itself a VSplitter, so the shell
// has the three-region layout an editor usually wants. Neither sash can be
// dragged past its pane's floor, which the outer splitter sets explicitly with
// withMinPaneSize() and the inner one leaves at the 40 px default.

#include "declarative_ui.hpp"

#include <string>
#include <vector>

inline auto drawAppShellUI(
    std::string& selectedFile,
    TableRows& rows,
    int& selectedRow,
    std::string& notes,
    bool& shellDisabled)
{
    // Same pinning discipline as the other galleries: explicit sizes on the
    // leaves and MinSize on the containers, so the three backends compute the
    // same frames despite their different native metrics.
    constexpr int kRowH = 26;
    constexpr int kLabelH = 20;
    constexpr int kShellH = 360;
    constexpr Size kButtonSize { 110, 28 };

    const std::vector<TreeItem> projectTree {
        { "src", {
            { "main.cpp" },
            { "engine", { { "layout.cpp" }, { "measure.cpp" } } },
        }, true },
        { "include", { { "widgets.hpp" }, { "splitter.hpp" } } },
        { "tests", { { "layout_tests.cpp" } } },
    };

    const std::vector<TableColumn> columns {
        { "File",  -1, /*sortable*/ true },
        { "Size",  70, /*sortable*/ true },
        { "Notes", -1, /*sortable*/ false, /*editable*/ true },
    };

    return Dialog {
        "Application Shell",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 10),
            StaticText{"Drag either sash; the panes re-measure, they do not just move."}
                .withSize({-1, kLabelH}),
            // 240 is the first pane's width in pixels, snapshotted from a
            // literal -- the splitter keeps its own copy of it. The binding
            // demo in value_binding.hpp is where an int& goes instead.
            HSplitter { LayoutFlags().Expand().Border(Side::Top, 8).MinSize({720, kShellH}),
                240,
                VStack {
                    LayoutFlags().Expand(),
                    StaticText{"Project"}
                        .withSize({-1, kLabelH}),
                    TreeView { projectTree, selectedFile }
                        .withVisibleRows(10)
                        .withFlags(LayoutFlags().Proportion(1).Expand().Border(Side::Top, 4))
                        .isDisabled(shellDisabled)
                },
                VSplitter { LayoutFlags().Expand(),
                    220,
                    VStack {
                        LayoutFlags().Expand(),
                        StaticText{"Files"}
                            .withSize({-1, kLabelH}),
                        Table { columns, rows, selectedRow }
                            .withVisibleRows(6)
                            .withFlags(LayoutFlags().Proportion(1).Expand().Border(Side::Top, 4))
                            .isDisabled(shellDisabled)
                    },
                    VStack {
                        LayoutFlags().Expand(),
                        StaticText{"Notes"}
                            .withSize({-1, kLabelH}),
                        MultiLineTextCtrl{notes}
                            .withFlags(LayoutFlags().Proportion(1).Expand().Border(Side::Top, 4))
                            .isDisabled(shellDisabled)
                    }
                }
                // The inner sash keeps the 40 px default on both panes.
            }
            // Neither outer pane may be dragged below this, whatever the user
            // does with the sash -- a pane's own MinSize would say the same
            // thing, and the larger of the two wins.
            .withMinPaneSize(160, 300),
            CheckBox{shellDisabled, "Lock the whole shell (sashes included)"}
                .withSize({-1, kRowH})
                .withFlags(LayoutFlags().Border(Side::Top, 10)),
            Separator{}
                .withSize({-1, 1})
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 10)),
            HStack {
                LayoutFlags().Border(Side::Top, 10),
                Spacer{},
                Button{"Close"}
                    .withSize(kButtonSize)
                    .withFlags(LayoutFlags().CenterVertical())
                    .onClick([]() {})
            }
        }
    };
}
