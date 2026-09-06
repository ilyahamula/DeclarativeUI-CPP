#pragma once

// Selection controls: the gallery dialog for list-style pickers.
//
// controls_gallery.hpp is already well past the ~20-element mark rule 4 sets,
// so ListBox lands here instead, alongside a few already-implemented controls
// for context (ComboBox, StaticText, Button).
//
// The two list boxes show both halves of the ListBox binding: the bound type is
// what picks the mode. A std::string binding selects one item by its text; a
// std::vector<int> binding selects many by index. Nothing else changes.
//
// Multi-select gesture note: wx and Qt inherit the platform's ctrl/shift-click
// range selection. ImGui has no native multi-select, so it gets plain-click
// replace + ctrl/cmd-click toggle -- no shift-range.

#include "declarative_ui.hpp"

#include <string>
#include <vector>

inline auto drawSelectionUI(
    std::string& favouriteLanguage,
    std::vector<int>& selectedTags,
    std::string& comboChoice,
    bool& listsDisabled)
{
    // Same pinning discipline as the controls gallery: explicit sizes on the
    // leaves and MinSize on the group boxes, so the three backends compute the
    // same frames despite their different native metrics.
    constexpr int kBoxW = 300;
    constexpr int kRowH = 26;
    constexpr int kLabelH = 20;
    constexpr int kListH = 130;
    constexpr Size kButtonSize { 110, 28 };

    return Dialog {
        "Selection Controls",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 10),
            HStack {
                VGroupBox { "Single select",
                    LayoutFlags().MinSize({kBoxW, 210}),
                    StaticText{"Favourite language:"}
                        .withSize({-1, kLabelH}),
                    // Bound to a std::string: the selection is the item's text.
                    ListBox<std::string> { {"C++", "Rust", "Python", "Go", "Zig", "Ada"}, favouriteLanguage }
                        .withVisibleRows(5)
                        .withSize({-1, kListH})
                        .withFlags(LayoutFlags().Expand().Border(Side::Top, 5))
                        .isDisabled(listsDisabled),
                    ComboBox<std::string> { {"C++", "Rust", "Python", "Go", "Zig", "Ada"}, comboChoice }
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand().Border(Side::Top, 8))
                        .isDisabled(listsDisabled)
                },
                VGroupBox { "Multi select",
                    LayoutFlags().MinSize({kBoxW, 210}).Border(Side::Left, 12),
                    StaticText{"Tags (ctrl-click):"}
                        .withSize({-1, kLabelH}),
                    // Bound to a std::vector<int>: the selection is item indices,
                    // and the vector binding is the whole of what makes it multi.
                    ListBox<std::vector<int>> { {"urgent", "bug", "feature", "docs", "wontfix", "good first issue"}, selectedTags }
                        .withVisibleRows(5)
                        .withSize({-1, kListH})
                        .withFlags(LayoutFlags().Expand().Border(Side::Top, 5))
                        .isDisabled(listsDisabled),
                    CheckBox{listsDisabled, "Disable selection controls"}
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
