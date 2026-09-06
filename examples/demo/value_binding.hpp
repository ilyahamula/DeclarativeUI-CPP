#pragma once

// Value binding: dialogs where controls share a caller-owned value by reference,
// plus a checkbox bound to isDisabled().
//
// Two things are on show here, both of which only work because a bound ref is
// polled rather than read once at build time:
//
//   * Two controls on the SAME value stay in step. Move either one and the other
//     follows -- no callback wiring, no manual copy-back.
//   * A bool bound to isDisabled() enables/disables live, without rebuilding the
//     tree.
//
// Retained backends (wx/Qt) poll on idle; ImGui rebuilds every frame and so reads
// the live value for free.
//
// macOS caveat: a disabled wxGauge/QProgressBar looks identical to an enabled one
// -- the platform draws no disabled state for progress indicators. The state IS
// applied; it just isn't visible. Prefer the slider/spin/text dialogs below when
// eyeballing whether disabling works.

#include "declarative_ui.hpp"

#include <string>
#include <vector>

inline auto drawProgressBarBindedToSlider(float& value, bool& checked)
{
    return Dialog {
        "Slider and Checkbox Dialog",
        VStack {
            ProgressBar{ value }
                .withFlags(LayoutFlags().CenterHorizontal().CenterVertical().Expand())
                .isDisabled(checked),
            HStack {
                Slider { { .min = 0.0f, .max = 100.0f }, value }
                    .isDisabled(checked),
                CheckBox{checked, "Disable"}
            }
        }
    };
}

// SpinBox + Slider over one int. The clearest demonstration of two-way sync:
// type in the spin box and the slider jumps, drag the slider and the number
// follows.
inline auto drawSpinAndSliderPair(int& value, bool& disabled)
{
    return Dialog {
        "Spin + Slider (shared int)",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 12),
            HStack {
                StaticText{"Value:"}
                    .withSize({60, 22})
                    .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8)),
                SpinBox{ Range<int>{ .min = 0, .max = 100 }, value }
                    .withFlags(LayoutFlags().CenterVertical())
                    .isDisabled(disabled)
            },
            Slider { Range<int>{ .min = 0, .max = 100 }, value }
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 10))
                .isDisabled(disabled),
            CheckBox{disabled, "Disable both"}
                .withFlags(LayoutFlags().Border(Side::Top, 12))
        }
    };
}

// Two text fields over one string. Typing in either updates the other, and the
// field you are typing in keeps its caret -- the sync only writes a control whose
// displayed text already disagrees with the value.
inline auto drawTextMirror(std::string& text, bool& disabled)
{
    return Dialog {
        "Mirrored text fields (shared string)",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 12),
            StaticText{"Type in either field:"}
                .withFlags(LayoutFlags().Border(Side::Bottom, 8)),
            TextCtrl{text}
                .withFlags(LayoutFlags().Expand())
                .isDisabled(disabled),
            TextCtrl{text}
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 8))
                .isDisabled(disabled),
            CheckBox{disabled, "Disable both"}
                .withFlags(LayoutFlags().Border(Side::Top, 12))
        }
    };
}

// ComboBox + RadioButtons over one int. Radio indices are assigned in
// declaration order, so they line up with the combo's item indices.
inline auto drawChoiceMirror(int& choice, bool& disabled)
{
    return Dialog {
        "Combo + Radios (shared index)",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 12),
            ComboBox{ {"Red", "Green", "Blue"}, choice }
                .withFlags(LayoutFlags().Expand())
                .isDisabled(disabled),
            VGroupBox { "Same value as radios",
                LayoutFlags().Expand().Border(Side::Top, 10),
                RadioButton{choice, "Red"}
                    .isDisabled(disabled),
                RadioButton{choice, "Green"}
                    .isDisabled(disabled),
                RadioButton{choice, "Blue"}
                    .isDisabled(disabled)
            },
            CheckBox{disabled, "Disable both"}
                .withFlags(LayoutFlags().Border(Side::Top, 12))
        }
    };
}

// CheckBox + ToggleButton over one bool, with a second bool disabling them.
// Two independent bindings in one dialog: the shared value and the disable flag.
inline auto drawToggleMirror(bool& flag, bool& disabled)
{
    return Dialog {
        "Checkbox + Toggle (shared bool)",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 12),
            CheckBox{flag, "Enabled feature"}
                .withFlags(LayoutFlags().Expand())
                .isDisabled(disabled),
            ToggleButton{flag, "Same bool"}
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 8))
                .isDisabled(disabled),
            CheckBox{disabled, "Disable both"}
                .withFlags(LayoutFlags().Border(Side::Top, 12))
        }
    };
}

// Two ListBoxes over one std::string. Picking an item in either box moves the
// selection in the other -- the same shared-ref sync as drawTextMirror/
// drawChoiceMirror, just with ListBox's single-select binding instead.
inline auto drawListBoxMirror(std::string& choice, bool& disabled)
{
    return Dialog {
        "Two ListBoxes (shared string)",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 12),
            HStack {
                ListBox<std::string> { {"C++", "Rust", "Python", "Go", "Zig"}, choice }
                    .withVisibleRows(5)
                    .withSize({140, 110})
                    .withFlags(LayoutFlags().Expand())
                    .isDisabled(disabled),
                ListBox<std::string> { {"C++", "Rust", "Python", "Go", "Zig"}, choice }
                    .withVisibleRows(5)
                    .withSize({140, 110})
                    .withFlags(LayoutFlags().Expand().Border(Side::Left, 8))
                    .isDisabled(disabled)
            },
            CheckBox{disabled, "Disable both"}
                .withFlags(LayoutFlags().Border(Side::Top, 12))
        }
    };
}

// Two TreeViews and a TextCtrl over one std::string. The trees write the
// selected item's PATH into the shared string, so picking in either tree moves
// the selection in the other and the field spells out what was picked.
//
// It also runs the other way: type a valid path into the field -- "src/engine"
// -- and both trees select it. That is the ref sync doing its job in the
// direction no control initiated, which is exactly what a bound value buys.
// A path that matches nothing simply selects nothing; a stale path is "not in
// this tree", and quietly selecting a neighbour would be worse.
//
// Note the trees expand independently: `expanded` on a TreeItem seeds the
// initial state, it is not a bound value.
inline auto drawTreeViewMirror(std::string& path, bool& disabled)
{
    const std::vector<TreeItem> tree {
        { "src", { { "main.cpp" }, { "engine", { { "layout.cpp" } } } }, true },
        { "docs", { { "readme.md" } } },
    };

    return Dialog {
        "Two TreeViews (shared path)",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 12),
            StaticText{"Pick in either tree, or type a path:"}
                .withFlags(LayoutFlags().Border(Side::Bottom, 8)),
            HStack {
                TreeView { tree, path }
                    .withVisibleRows(6)
                    .withSize({170, 140})
                    .withFlags(LayoutFlags().Expand())
                    .isDisabled(disabled),
                TreeView { tree, path }
                    .withVisibleRows(6)
                    .withSize({170, 140})
                    .withFlags(LayoutFlags().Expand().Border(Side::Left, 8))
                    .isDisabled(disabled)
            },
            TextCtrl{path}
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 8))
                .isDisabled(disabled),
            CheckBox{disabled, "Disable both"}
                .withFlags(LayoutFlags().Border(Side::Top, 12))
        }
    };
}

// Two Tables over ONE rows vector and ONE selected-key value, both by reference.
// Click a row in either and the other follows; sort one and the other keeps its
// own view order, because sorting is a property of the view and the binding is a
// property of the data.
//
// The key binding is on show here rather than the index one: `selected` holds
// the text of the row's FIRST column, so the TextCtrl below can select a row by
// typing its name. That readability is the whole point of a key binding -- at
// the cost that two rows sharing a first column are indistinguishable through
// it, which is why the File column is left uneditable here while Notes is not.
//
// `rows` is bound too, not just the selection, which is what makes the editable
// Notes column write back: edit a note in the left table and the right one shows
// it, because both are reading the caller's vector.
inline auto drawTableMirror(TableRows& rows, std::string& selected, bool& disabled)
{
    const std::vector<TableColumn> columns {
        { "File",  -1, /*sortable*/ true },
        { "Size",  70, /*sortable*/ true },
        { "Notes", -1, /*sortable*/ false, /*editable*/ true },
    };

    return Dialog {
        "Two Tables (shared rows + key)",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 12),
            StaticText{"Pick in either table, or type a file name:"}
                .withFlags(LayoutFlags().Border(Side::Bottom, 8)),
            HStack {
                Table { columns, rows, selected }
                    .withVisibleRows(5)
                    .withSize({320, 150})
                    .withFlags(LayoutFlags().Expand())
                    .isDisabled(disabled),
                Table { columns, rows, selected }
                    .withVisibleRows(5)
                    .withSize({320, 150})
                    .withFlags(LayoutFlags().Expand().Border(Side::Left, 8))
                    .isDisabled(disabled)
            },
            TextCtrl{selected}
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 8))
                .isDisabled(disabled),
            CheckBox{disabled, "Disable both"}
                .withFlags(LayoutFlags().Border(Side::Top, 12))
        }
    };
}

// One string, three jobs: it is the TextCtrl's value AND the live-bound tooltip
// of the two controls below it. Type in the field and hover either one -- the
// hover text is already the new string, with nothing rebuilt and no callback
// wiring. The Button alongside carries a snapshot instead, so it keeps saying
// the same thing however the field is edited: that is the whole difference
// between withTooltip(std::string&) and withTooltip(const std::string&).
//
// Tooltips are leaf-only -- a Stack is pure geometry with no native window to
// hang one on -- so the group box below has no tooltip of its own, only its
// children do. Disabling suppresses the tooltip everywhere: wx and Qt deliver
// no tooltip event to a disabled window, and the ImGui backend matches them.
inline auto drawTooltipBinding(std::string& hint, bool& disabled)
{
    return Dialog {
        "Live tooltips (shared string)",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 12),
            StaticText{"Tooltip text (hover the controls below):"}
                .withFlags(LayoutFlags().Border(Side::Bottom, 8)),
            TextCtrl{hint}
                .withFlags(LayoutFlags().Expand())
                .withTooltip("Whatever you type here becomes the hover text below."),
            HGroupBox { "Both bound to the same string",
                LayoutFlags().Expand().Border(Side::Top, 10),
                Button{"Hover me"}
                    .withSize({110, 28})
                    .withFlags(LayoutFlags().CenterVertical())
                    .withTooltip(hint)
                    .isDisabled(disabled),
                CheckBox{disabled, "...and me"}
                    .withFlags(LayoutFlags().CenterVertical().Border(Side::Left, 12))
                    .withTooltip(hint)
            },
            Button{"Snapshot tooltip"}
                .withSize({160, 28})
                .withFlags(LayoutFlags().Border(Side::Top, 12))
                .withTooltip("Taken once, at build time -- editing the field above never changes this.")
        }
    };
}

// One bool, three jobs: it is the CheckBox's own value, it disables the group
// box holding the two mirrored spin boxes, and it disables the reset Button --
// all by reference, so ticking the box updates every one of them at once. The
// spin boxes never mention the flag; they inherit it from their container.
inline auto drawGroupDisabledByCheckBox(int& value, bool& disabled)
{
    return Dialog {
        "Group box bound to isDisabled()",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 12),
            HGroupBox { "Mirrored count",
                LayoutFlags().Expand().MinSize({280, -1}),
                SpinBox { Range<int>{ .min = 0, .max = 100 }, value }
                    .withSize({100, 26})
                    .withFlags(LayoutFlags().CenterVertical()),
                Slider { Range<int>{ .min = 0, .max = 100 }, value }
                    .withSize({150, 24})
                    .withFlags(LayoutFlags().Proportion(1).CenterVertical().Border(Side::Left, 10))
            }
            .isDisabled(disabled),
            HStack {
                LayoutFlags().Border(Side::Top, 12),
                CheckBox{disabled, "Lock the group"},
                Button{"Reset"}
                    .withSize({90, 28})
                    .withFlags(LayoutFlags().Border(Side::Left, 12))
                    .onClick([&value]() { value = 0; })
            }
        }
    };
}

// Spacer under a bound isDisabled(). A Spacer carries no value of its own, so
// there is nothing to share by reference the way the dialogs above do -- the
// bindable half of it is the disable flag, and even that it only ever inherits:
// a windowless leaf has nothing to grey out, and never asks for the flag.
//
// So the binding on show is one bool doing three jobs around the spacers: it is
// the CheckBox's own value, it disables the group box the flexible Spacer{}
// pushes apart, and it disables the Button in the fixed row. Tick the box and
// every one of them follows at once, while the gaps stay exactly where the
// engine put them -- which is the point: geometry is not state.
inline auto drawSpacerDisableBinding(std::string& caption, bool& disabled)
{
    return Dialog {
        "Spacer in a bound row",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 12),
            HGroupBox { "Pushed apart by one Spacer{}",
                LayoutFlags().Expand().MinSize({380, -1}),
                // Both fields are bound to the same string, so the spacer sits
                // between two controls that already mirror each other: editing
                // either one leaves the gap untouched.
                TextCtrl{caption}
                    .withSize({150, 26})
                    .withFlags(LayoutFlags().CenterVertical()),
                Spacer{},
                TextCtrl{caption}
                    .withSize({150, 26})
                    .withFlags(LayoutFlags().CenterVertical())
            }
            .isDisabled(disabled),
            HStack {
                LayoutFlags().Border(Side::Top, 12),
                CheckBox{disabled, "Lock the row"}
                    .withFlags(LayoutFlags().CenterVertical()),
                Spacer{},
                Button{"Reset"}
                    .withSize({90, 28})
                    .isDisabled(disabled)
                    .onClick([&caption]() { caption = "shared"; })
            }
        }
    };
}
