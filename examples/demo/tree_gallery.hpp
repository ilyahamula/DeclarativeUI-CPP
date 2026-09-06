#pragma once

// Hierarchical selection: the gallery dialog for TreeView.
//
// controls_gallery.hpp is long past the ~20-element mark rule 4 sets, so
// TreeView lands here instead, alongside a few already-implemented controls for
// context (TextCtrl, CheckBox, StaticText, Button).
//
// The two trees show both halves of the TreeView binding. Selection is always a
// PATH -- the item's labels from the root joined by '/' -- because an index says
// nothing about where an item sits in a tree. A std::string binding holds the
// one selected path; a std::vector<std::string> binding holds several, but only
// once isMultiSelect() widens the control: unlike ListBox, the bound type does
// not pick the mode, so a vector binding on its own is still single-select.
//
// Multi-select gesture note: wx and Qt inherit the platform's ctrl/shift-click
// range selection. ImGui has no native multi-select, so it gets plain-click
// replace + ctrl/cmd-click toggle -- no shift-range.
//
// Expansion note: `expanded` on a TreeItem is the INITIAL state only. After that
// the user owns it, and the two trees below expand independently even though
// they list the same items.

#include "declarative_ui.hpp"

#include <string>
#include <vector>

inline auto drawTreeUI(
    std::string& selectedFile,
    std::vector<std::string>& selectedModules,
    bool& treesDisabled)
{
    // Same pinning discipline as the other galleries: explicit sizes on the
    // leaves and MinSize on the group boxes, so the three backends compute the
    // same frames despite their different native metrics.
    constexpr int kBoxW = 300;
    constexpr int kRowH = 26;
    constexpr int kLabelH = 20;
    constexpr int kTreeH = 190;
    constexpr Size kButtonSize { 110, 28 };

    const std::vector<TreeItem> projectTree {
        { "src", {
            { "main.cpp" },
            { "engine", { { "layout.cpp" }, { "measure.cpp" } } },
        }, true },
        { "include", { { "widgets.hpp" }, { "stacks.hpp" } } },
        { "tests", { { "layout_tests.cpp" } } },
    };

    return Dialog {
        "Tree Controls",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 10),
            HStack {
                VGroupBox { "Single select",
                    LayoutFlags().MinSize({kBoxW, 280}),
                    StaticText{"Pick one file:"}
                        .withSize({-1, kLabelH}),
                    // Bound to a std::string: the selection is one item's path,
                    // e.g. "src/engine/layout.cpp".
                    TreeView { projectTree, selectedFile }
                        .withVisibleRows(8)
                        .withSize({-1, kTreeH})
                        .withFlags(LayoutFlags().Expand().Border(Side::Top, 5))
                        .isDisabled(treesDisabled),
                    // Shows the bound path live -- the tree writes through to
                    // the same string this field is bound to.
                    TextCtrl{selectedFile}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand().Border(Side::Top, 8))
                        .isDisabled(treesDisabled)
                },
                VGroupBox { "Multi select",
                    LayoutFlags().MinSize({kBoxW, 280}).Border(Side::Left, 12),
                    StaticText{"Modules (ctrl-click):"}
                        .withSize({-1, kLabelH}),
                    // Same items, a vector binding, and isMultiSelect() -- the
                    // vector alone would still select one path at a time.
                    TreeView { projectTree, selectedModules }
                        .isMultiSelect()
                        .withVisibleRows(8)
                        .withSize({-1, kTreeH})
                        .withFlags(LayoutFlags().Expand().Border(Side::Top, 5))
                        .isDisabled(treesDisabled),
                    CheckBox{treesDisabled, "Disable both trees"}
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
