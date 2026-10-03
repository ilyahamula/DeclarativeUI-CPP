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

// RadioGroup: one exclusive choice among labelled options, bound to the index
// of the chosen one. It is a stack of RadioButtons, so it looks exactly like
// the hand-written radios on every backend; what it saves is the bookkeeping.
// Shown here as a column, as a row (withOrientation), and live-hidden
// (isHidden), next to a ComboBox and a label reading the same indices.
inline auto drawRadioGroupUI(int& shipping, int& size, int& wrap, bool& hideWrap, bool& groupsDisabled)
{
    constexpr int kBoxW = 260;
    constexpr int kRowH = 26;
    constexpr int kLabelH = 20;

    return Dialog {
        "Radio Groups",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 10),
            HStack {
                VGroupBox { "Shipping (column)",
                    LayoutFlags().MinSize({kBoxW, 150}),
                    RadioGroup{shipping, {"Standard", "Express", "Overnight", "Pick up in store"}}
                        .isDisabled(groupsDisabled)
                },
                VGroupBox { "Size (row)",
                    LayoutFlags().MinSize({kBoxW, 150}).Border(Side::Left, 12),
                    RadioGroup{size, {"S", "M", "L", "XL"}}
                        .withOrientation(Orientation::Horizontal)
                        .isDisabled(groupsDisabled),
                    // the same index, from a combo: either one moves the other
                    ComboBox{ {"S", "M", "L", "XL"}, size }
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand().Border(Side::Top, 10))
                        .isDisabled(groupsDisabled),
                    CheckBox{hideWrap, "Hide the wrapping options"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Border(Side::Top, 8)),
                    // bound isHidden: the group box shrinks around it live
                    RadioGroup{LayoutFlags().Border(Side::Top, 4), wrap, {"Plain paper", "Ribbon"}}
                        .withOrientation(Orientation::Horizontal)
                        .isHidden(hideWrap)
                        .isDisabled(groupsDisabled)
                }
            },
            StaticText{"Pick a group, then disable them all:"}
                .withSize({-1, kLabelH})
                .withFlags(LayoutFlags().Border(Side::Top, 10)),
            CheckBox{groupsDisabled, "Disable the groups"}
                .withSize({-1, kRowH})
        }
    };
}

// EditableCombo beside the read-only ComboBox it complements: the ComboBox can
// only pick from its list, the EditableCombo takes any text and offers the
// list as suggestions. A placeholder shows while the field is empty, and the
// status line names what each one reported.
inline auto drawEditableComboGalleryUI(std::string& font, std::string& city, std::string& size,
    std::string& status)
{
    constexpr int kLabelW = 120;
    constexpr int kFieldW = 200;
    return Dialog {
        "Editable combos",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 12),
            HStack {
                StaticText{"Font"}.withSize({kLabelW, 20}).withFlags(LayoutFlags().CenterVertical()),
                EditableCombo{font, {"Arial", "Courier New", "Georgia", "Helvetica", "Verdana"}}
                    .withSize({kFieldW, -1})
                    .withTooltip("Pick one, or type any font name")
                    .onChange([&status](const std::string& f) { status = "Font: " + f; })
            },
            HStack {
                LayoutFlags().Border(Side::Top, 6),
                StaticText{"City"}.withSize({kLabelW, 20}).withFlags(LayoutFlags().CenterVertical()),
                EditableCombo{city, {"Kyiv", "Lviv", "Odesa", "Kharkiv"}}
                    .withPlaceholder("Type or pick a city")
                    .withSize({kFieldW, -1})
                    .onChange([&status](const std::string& c) { status = "City: " + c; })
            },
            HStack {
                LayoutFlags().Border(Side::Top, 6),
                StaticText{"Size (list only)"}.withSize({kLabelW, 20}).withFlags(LayoutFlags().CenterVertical()),
                ComboBox{ {"Small", "Medium", "Large"}, size }
                    .withSize({kFieldW, -1})
                    .onChange([&status](const std::string& s) { status = "Size: " + s; })
            },
            StaticText{status}
                .withSize({kLabelW + kFieldW + 8, 20})
                .withFlags(LayoutFlags().Border(Side::Top, 10))
        }
    };
}

// VirtualList beside the ListBox it scales past: the ListBox holds its six
// items, the VirtualList a million log lines it never stores -- each row is
// produced by the function when it scrolls into view. Its selection is
// reported into the label underneath.
inline auto drawVirtualListGalleryUI(int& logCount, int& logRow, std::string& languagePick,
    std::string& status)
{
    return Dialog {
        "Large lists",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 12),
            HStack {
                VStack {
                    StaticText{"ListBox (6 items)"}.withSize({160, 20}),
                    ListBox<std::string>{ {"C++", "Rust", "Python", "Go", "Zig", "Ada"}, languagePick }
                        .withVisibleRows(10)
                        .withSize({160, -1})
                },
                VStack {
                    LayoutFlags().Border(Side::Left, 16),
                    StaticText{"VirtualList (1,000,000 rows)"}.withSize({300, 20}),
                    VirtualList{ logCount,
                        [](int i) { return "#" + std::to_string(i) + "  request served in " + std::to_string(i % 97 + 3) + " ms"; },
                        logRow }
                        .withVisibleRows(10)
                        .withSize({300, -1})
                        .withTooltip("Rows are produced on demand; nothing is stored")
                        .onChange([&status](int row) { status = "Selected log line " + std::to_string(row); })
                }
            },
            StaticText{status}
                .withSize({476, 20})
                .withFlags(LayoutFlags().Border(Side::Top, 10))
        }
    };
}
