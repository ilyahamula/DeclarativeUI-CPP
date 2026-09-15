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

// Spacer and vertical Separator under a bound isDisabled(). Neither carries a
// value of its own, so
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
                // A vertical Separator between them: like the Spacer it carries
                // no value of its own, and like every leaf it inherits the group
                // box's bound disable flag rather than naming it.
                Separator{Orientation::Vertical}
                    .withSize({1, -1})
                    .withFlags(LayoutFlags().Expand()),
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

// A Grid of TextCtrls over one string. Every field in column 1 is bound to the
// same std::string, so typing in any one of them moves all the others -- the
// same shared-ref sync as drawTextMirror, but arranged as a form so the labels
// and fields line up in two bands rather than three independent rows.
//
// The Grid itself is bound to isDisabled(), which cascades to all six cells;
// none of them names the flag. Note the CheckBox lives OUTSIDE the grid, or
// locking the form would lock the control that unlocks it.
inline auto drawGridMirror(std::string& shared, bool& disabled)
{
    constexpr int kLabelH = 22;
    constexpr int kFieldH = 26;

    return Dialog {
        "Grid of fields (shared string)",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 12),
            StaticText{"All three fields are bound to one std::string:"}
                .withSize({-1, kLabelH})
                .withFlags(LayoutFlags().Border(Side::Bottom, 8)),
            Grid { 2, LayoutFlags().Expand().MinSize({420, -1}),
                StaticText{"First:"}
                    .withSize({-1, kLabelH})
                    .withFlags(LayoutFlags().CenterVertical()),
                TextCtrl{shared}
                    .withSize({-1, kFieldH})
                    .withFlags(LayoutFlags().Proportion(1).Expand()),

                StaticText{"Second:"}
                    .withSize({-1, kLabelH})
                    .withFlags(LayoutFlags().CenterVertical()),
                TextCtrl{shared}
                    .withSize({-1, kFieldH})
                    .withFlags(LayoutFlags().Proportion(1).Expand()),

                StaticText{"Third:"}
                    .withSize({-1, kLabelH})
                    .withFlags(LayoutFlags().CenterVertical()),
                TextCtrl{shared}
                    .withSize({-1, kFieldH})
                    .withFlags(LayoutFlags().Proportion(1).Expand())
            }
            .isDisabled(disabled),
            CheckBox{disabled, "Lock the grid"}
                .withSize({-1, kFieldH})
                .withFlags(LayoutFlags().Border(Side::Top, 12))
        }
    };
}

// Ten CheckBoxes inside a ScrollPanel, all bound to ONE bool that lives outside
// it -- and a CheckBox outside the panel bound to the same one. Tick any of
// them and every other follows, including the ones currently scrolled out of
// sight: a bound value is polled, not captured at build time, so being off
// screen changes nothing.
//
// That is also the point of the second flag. It disables the panel, and the
// cascade reaches the whole scrolled subtree -- none of the rows names it.
inline auto drawScrollPanelBinding(bool& shared, bool& disabled)
{
    constexpr int kRowH = 28;

    return Dialog {
        "Scrolled checkboxes (shared bool)",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 12).MinSize({360, -1}),
            CheckBox{shared, "Outside the panel -- bound to the same bool"}
                .withSize({-1, kRowH})
                .withFlags(LayoutFlags().Border(Side::Bottom, 8)),
            ScrollPanel { LayoutFlags().Expand(),
                VStack {
                    LayoutFlags().Expand(),
                    CheckBox{shared, "Bound copy 1"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand()),
                    CheckBox{shared, "Bound copy 2"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand()),
                    CheckBox{shared, "Bound copy 3"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand()),
                    CheckBox{shared, "Bound copy 4"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand()),
                    CheckBox{shared, "Bound copy 5"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand()),
                    CheckBox{shared, "Bound copy 6"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand()),
                    CheckBox{shared, "Bound copy 7"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand()),
                    CheckBox{shared, "Bound copy 8"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand()),
                    CheckBox{shared, "Bound copy 9"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand()),
                    CheckBox{shared, "Bound copy 10"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand())
                }
            }
            .isDisabled(disabled),
            CheckBox{disabled, "Lock the scrolled list"}
                .withSize({-1, kRowH})
                .withFlags(LayoutFlags().Border(Side::Top, 10))
        }
    };
}

// Two splitters and a spin box on ONE int: drag either sash and the other one
// follows, and typing a number into the spin box moves them both.
//
// This is the same shared-ref idiom the dialogs above use, applied to a value
// the user drives by dragging rather than by typing -- and it is the honest test
// of a Splitter's binding, because it exercises both directions at once. The
// sash writes the first pane's new width through to `divider`; anything else
// writing `divider` moves the sash. Neither splitter knows the other exists.
//
// Note that a bound position is CLAMPED by each splitter against its own floors
// but written back unclamped by neither: the engine only ever reads the int, so
// the spin box shows exactly what the user last committed. What each splitter
// then draws is that value pinned into its own legal range -- which is why the
// two sashes can briefly disagree if you type a number one of them cannot honour.
inline auto drawSplitterBinding(int& divider, bool& disabled)
{
    constexpr int kRowH = 28;
    constexpr int kLabelH = 20;
    constexpr int kPaneH = 110;

    return Dialog {
        "Two splitters (shared int)",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 12).MinSize({520, -1}),
            StaticText{"Both sashes and the spin box are bound to the same int:"}
                .withSize({-1, kLabelH}),
            HSplitter { LayoutFlags().Expand().Border(Side::Top, 8).MinSize({-1, kPaneH}),
                divider,
                VGroupBox { "Left",
                    LayoutFlags().Expand(),
                    // T is the binding type and cannot be deduced from items
                    // alone, so an unbound list spells it out.
                    ListBox<std::string> { { "Alpha", "Beta", "Gamma" } }
                        .withVisibleRows(4)
                        .withFlags(LayoutFlags().Proportion(1).Expand())
                        .isDisabled(disabled)
                },
                VGroupBox { "Right",
                    LayoutFlags().Expand(),
                    MultiLineTextCtrl{"Drag the sash above or below."}
                        .withFlags(LayoutFlags().Proportion(1).Expand())
                        .isDisabled(disabled)
                }
            }
            .isDisabled(disabled),
            HSplitter { LayoutFlags().Expand().Border(Side::Top, 10).MinSize({-1, kPaneH}),
                divider,
                // Read-only fields rather than StaticText: a pane's width is
                // whatever the user last dragged it to, and wrapping text pinned
                // to one line's height would overflow its own frame the moment
                // the pane narrowed. A single-line field clips instead.
                VGroupBox { "Same int, second splitter",
                    LayoutFlags().Expand(),
                    ReadonlyTextCtrl{"This sash mirrors the one above."}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand())
                },
                VGroupBox { "Remainder",
                    LayoutFlags().Expand(),
                    ReadonlyTextCtrl{"...and this pane takes what is left."}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand())
                }
            }
            .isDisabled(disabled),
            HStack {
                LayoutFlags().Border(Side::Top, 10),
                StaticText{"Sash position:"}
                    .withSize({100, kRowH})
                    .withFlags(LayoutFlags().CenterVertical()),
                // Writes the same int the two sashes do: type into it and both
                // move, drag either sash and it follows.
                SpinBox { Range<int>{ .min = 40, .max = 460 }, divider }
                    .withSize({90, kRowH})
                    .withFlags(LayoutFlags().Border(Side::Left, 6))
                    .isDisabled(disabled),
                Spacer{},
                CheckBox{disabled, "Lock both splitters"}
                    .withSize({-1, kRowH})
                    .withFlags(LayoutFlags().CenterVertical())
            }
        }
    };
}


// A CheckBox, a ToggleButton and an Expander on ONE bool: tick the box and the
// section opens, click the header and the box ticks itself.
//
// This is the same shared-ref idiom the dialogs above use, applied to a value
// the user drives by clicking a container's chrome rather than a control. The
// header writes the new state through to `open`; anything else writing `open`
// folds or unfolds the section, and the dialog re-fits around it -- a section
// closing is a re-measure, not just a move, which is why the window follows.
//
// The second expander is bound to the SAME bool, so the two open and close
// together and neither knows the other exists.
inline auto drawExpanderBinding(bool& open, bool& disabled)
{
    constexpr int kRowH = 28;
    constexpr int kLabelH = 20;

    return Dialog {
        "Two expanders (shared bool)",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 12).MinSize({420, -1}),
            StaticText{"Both headers, the check box and the toggle share one bool:"}
                .withSize({-1, kLabelH}),
            HStack {
                LayoutFlags().Expand().Border(Side::Top, 8),
                CheckBox{open, "Show details"}
                    .withSize({-1, kRowH})
                    .withFlags(LayoutFlags().Proportion(1).CenterVertical())
                    .isDisabled(disabled),
                ToggleButton{open, "Details"}
                    .withSize({110, kRowH})
                    .withFlags(LayoutFlags().CenterVertical())
                    .isDisabled(disabled)
            },
            Expander { "Details",
                LayoutFlags().Expand().Border(Side::Top, 10),
                open,
                VGroupBox { "Bound to the same bool",
                    LayoutFlags().Expand(),
                    ReadonlyTextCtrl{"Fold me from the check box, the toggle or my own header."}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand())
                }
            }
            .isDisabled(disabled),
            Expander { "Details, again",
                LayoutFlags().Expand().Border(Side::Top, 8),
                open,
                VGroupBox { "Same bool, second expander",
                    LayoutFlags().Expand(),
                    ReadonlyTextCtrl{"...and this one follows the first."}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand())
                }
            }
            .isDisabled(disabled),
            CheckBox{disabled, "Lock both sections"}
                .withSize({-1, kRowH})
                .withFlags(LayoutFlags().Border(Side::Top, 12))
        }
    };
}

// The Window control panel: one bool shared between a Window's show(bool&), a
// CheckBox and a ToggleButton, plus onClose()'s report read back live -- and a
// second bool shared with a CHECKABLE MENU ITEM in the shell's View menu.
//
// `shellOpen` is the same bool the application shell in app_shell.hpp is shown
// against, which makes it a three-way binding rather than the usual two:
//
//   * untick the check box (or the toggle) and the shell WINDOW closes;
//   * close the shell from its title bar or its Close button and both controls
//     clear themselves.
//
// `shellStatus` is written by the shell's onClose() and mirrored here through an
// ordinary bound TextCtrl -- the same shape drawTextMirror() uses, and the only
// one that updates live on the retained backends (ReadonlyTextCtrl takes a
// snapshot, so an external write would never reach it). So the callback firing
// exactly once is visible from a window that is still up. Both refs belong to
// the caller and outlive every window here.
//
// Backend divergence worth knowing: TICKING THE BOX BACK ON DOES NOT REOPEN the
// shell on wx or Qt. Closing destroys the frame and frees its engine session
// (R6.4), and nothing re-runs show() for a retained backend. On ImGui the main
// calls show() every frame, so there the shell comes straight back. Re-showing a
// closed window is a lifecycle question T3.6 settles, not part of T2.1.
//
// `wordWrap` is the menu-item binding: it is the shell's View > Word wrap check
// item and the check box at the bottom of this panel, on one bool, across two
// windows. Tick either and the other follows -- a menu item is an ordinary
// bound value, polled on wx and Qt exactly as every other control's is, and
// read live on ImGui. Ctrl+Shift+W does the same thing from the keyboard while
// the shell is the active window.
//
// This panel is Fixed() at an explicit Size -- the one place the Window default
// is turned off, next to the shell which keeps it and auto-fits. Drag the
// shell's edge and it grows; drag this one's and nothing happens. The Size is
// the TOTAL window size on every backend (client area plus frame on wx, the
// widget itself on Qt, padding plus title bar on ImGui), which is the same
// contract Dialog's Size overload has.
inline auto drawWindowBinding(bool& shellOpen, std::string& shellStatus, bool& disabled,
    bool& wordWrap)
{
    constexpr int kRowH = 28;
    constexpr int kLabelH = 20;

    return Window {
        "Window controls (shared bool)",
        Size{ 460, 420 },
        VStack {
            LayoutFlags().Expand().Border(Side::All, 12).MinSize({420, -1}),
            StaticText{"The shell's open flag, a check box and a toggle: one bool."}
                .withSize({-1, kLabelH}),
            HStack {
                LayoutFlags().Expand().Border(Side::Top, 8),
                CheckBox{shellOpen, "Application Shell is open"}
                    .withSize({-1, kRowH})
                    .withFlags(LayoutFlags().Proportion(1).CenterVertical())
                    .isDisabled(disabled),
                ToggleButton{shellOpen, "Shell"}
                    .withSize({110, kRowH})
                    .withFlags(LayoutFlags().CenterVertical())
                    .isDisabled(disabled)
                    .withTooltip("Untick to close the shell window")
            },
            Separator{}
                .withSize({-1, 1})
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 10)),
            StaticText{"What the shell's onClose() last reported:"}
                .withSize({-1, kLabelH})
                .withFlags(LayoutFlags().Border(Side::Top, 10)),
            // Bound, not readonly: an external write only reaches a control
            // through a bound ref, and this one is written from a callback in
            // another window.
            TextCtrl{shellStatus}
                .withSize({-1, kRowH})
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 4)),
            CheckBox{disabled, "Lock both controls"}
                .withSize({-1, kRowH})
                .withFlags(LayoutFlags().Border(Side::Top, 12)),
            Separator{}
                .withSize({-1, 1})
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 10)),
            StaticText{"The shell's View > Word wrap check item, and this box: one bool."}
                .withSize({-1, kLabelH})
                .withFlags(LayoutFlags().Border(Side::Top, 10)),
            // The menu-item binding. The check mark and this box are the same
            // bool: tick either one (or press Ctrl+Shift+W in the shell) and
            // both follow, across two separate windows.
            CheckBox{wordWrap, "Word wrap"}
                .withSize({-1, kRowH})
                .withFlags(LayoutFlags().Border(Side::Top, 4))
                .withTooltip("Also on the shell's View menu, as Ctrl+Shift+W")
        }
    }
    // The inverse of a Dialog: a Window resizes unless it is told not to.
    .Fixed();
}
