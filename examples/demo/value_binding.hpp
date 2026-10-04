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

#include <algorithm>
#include <cctype>
#include <chrono>
#include <functional>
#include <string>
#include <thread>
#include <vector>

inline auto drawProgressBarBindedToSlider(float& value, bool& checked)
{
    return
        Dialog {
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
    return
        Dialog {
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
    return
        Dialog {
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

// ComboBox + RadioButtons over one int. Each radio names the index it stands
// for, so it lines up with the combo's item of the same index -- and the three
// radios are one group because they share `choice`, not because of where they
// are declared.
//
// The combo's CHOICES are bound too: `colours` is the caller's vector, and
// "Add a colour" appends to it -- the combo repopulates on every backend and
// keeps its selection. The radios name the first three indices only.
inline auto drawChoiceMirror(int& choice, bool& disabled, ItemList& colours)
{
    return
        Dialog {
            "Combo + Radios (shared index)",
            VStack {
                LayoutFlags().Expand().Border(Side::All, 12),
                ComboBox{ colours, choice }
                    .withFlags(LayoutFlags().Expand())
                    .isDisabled(disabled),
                Button{"Add a colour"}
                    .withFlags(LayoutFlags().Border(Side::Top, 8))
                    .isDisabled(disabled)
                    .onClick([&colours] { colours.push_back("Colour " + std::to_string(colours.size() + 1)); }),
                VGroupBox { "Same value as radios",
                    LayoutFlags().Expand().Border(Side::Top, 10),
                    RadioButton{choice, 0, "Red"}
                        .isDisabled(disabled),
                    RadioButton{choice, 1, "Green"}
                        .isDisabled(disabled),
                    RadioButton{choice, 2, "Blue"}
                        .isDisabled(disabled)
                },
                CheckBox{disabled, "Disable both"}
                    .withFlags(LayoutFlags().Border(Side::Top, 12))
            }
        };
}

// RadioGroup + Slider + SpinBox over one int, with a bool disabling all three.
// The group writes the index of the option picked; the slider and the spin box
// write the same number -- so dragging the slider moves the radio, and picking
// a radio moves the slider. onChange reports after the int has been written.
inline auto drawRadioGroupMirror(int& level, bool& disabled, std::string& lastPick)
{
    return
        Dialog {
            "RadioGroup + Slider (shared index)",
            VStack {
                LayoutFlags().Expand().Border(Side::All, 12),
                RadioGroup{level, {"Off", "Low", "Medium", "High"}}
                    .withOrientation(Orientation::Horizontal)
                    .isDisabled(disabled)
                    .onChange([&lastPick](int i) { lastPick = "Picked option " + std::to_string(i); }),
                Slider { Range<int>{ .min = 0, .max = 3 }, level }
                    .withFlags(LayoutFlags().Expand().Border(Side::Top, 10))
                    .isDisabled(disabled),
                SpinBox { Range<int>{ .min = 0, .max = 3 }, level }
                    .withSize({120, -1})
                    .withFlags(LayoutFlags().Border(Side::Top, 8))
                    .isDisabled(disabled),
                StaticText{lastPick}
                    .withSize({260, 20})
                    .withFlags(LayoutFlags().Border(Side::Top, 8)),
                CheckBox{disabled, "Disable all three"}
                    .withFlags(LayoutFlags().Border(Side::Top, 12))
            }
        };
}

// CheckBox + ToggleButton over one bool, with a second bool disabling them.
// Two independent bindings in one dialog: the shared value and the disable flag.
inline auto drawToggleMirror(bool& flag, bool& disabled)
{
    return
        Dialog {
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
    return
        Dialog {
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

    return
        Dialog {
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

    return
        Dialog {
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
    return
        Dialog {
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
    return
        Dialog {
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
    return
        Dialog {
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

    return
        Dialog {
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

    return
        Dialog {
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

    return
        Dialog {
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

    return
        Dialog {
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
// `shellStatus` is bound THREE ways here: the shell's onClose() writes it, the
// TextCtrl below edits it, and the StatusBar field under that shows it. Type in
// the box and the bar follows.
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
// calls show() every frame, so there the shell comes straight back. The dialog
// panel further down shows what a caller does about that: re-showing works on
// every backend, but on a retained one it is a second show() call rather than a
// write to the flag.
//
// `showGrid` is the context-menu binding: a check item inside the toggle's own
// right-click menu and the `ToggleButton` itself, on one bool. Unlike the menu
// bar, a popup is rebuilt from the model every time it opens, so its check mark
// is read at that moment and nothing polls it.
//
// `wordWrap` is the menu-item AND tool-bar binding: the shell's View > Word wrap
// check item, the shell's "Wrap" tool, the check box below and the tool bar
// below are four holders of one bool, across two windows. A check tool is an
// ordinary bound value, polled on wx and Qt exactly as a CheckBox's is.
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
    bool& wordWrap, bool& showGrid)
{
    constexpr int kRowH = 28;
    constexpr int kLabelH = 20;

    return
        Window {
            "Window controls (shared bool)",
            Size{ 460, 500 },
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
                // The same string again, in a status bar field: type in the box
                // above and the bar follows, because both are bound to `shellStatus`
                // and neither knows about the other. A StatusBar needs no flags to
                // span -- Expand() is its default.
                StatusBar{ shellStatus }
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
                    .withTooltip("Also on the shell's View menu, as Ctrl+Shift+W"),
                // The third and fourth holders of that same bool: a toolbar CHECK
                // tool here, and the shell's own toolbar and View menu over there.
                // Press any one of the four and the other three follow.
                ToolBar {
                    ToolItem{"Wrap"}.withTooltip("The same bool as the box above").toggled(wordWrap),
                    ToolItem::Separator(),
                    ToolItem{"On"}.onClick([&wordWrap]() { wordWrap = true; }),
                    ToolItem{"Off"}.onClick([&wordWrap]() { wordWrap = false; }),
                }
                    .withFlags(LayoutFlags().Border(Side::Top, 4)),
                Separator{}
                    .withSize({-1, 1})
                    .withFlags(LayoutFlags().Expand().Border(Side::Top, 10)),
                StaticText{"A context-menu check item and this toggle: one bool."}
                    .withSize({-1, kLabelH})
                    .withFlags(LayoutFlags().Border(Side::Top, 10)),
                // The context-menu binding. Right-click the toggle: the check item
                // in its own menu and the toggle's pressed state are the same bool,
                // so either one moves the other. The menu is rebuilt from the model
                // every time it opens, which is why the check mark is always current
                // without anything polling it.
                ToggleButton{showGrid, "Show grid"}
                    .withSize({140, kRowH})
                    .withFlags(LayoutFlags().Border(Side::Top, 4))
                    .withTooltip("Right-click me")
                    .withContextMenu({
                        MenuItem{"Show grid"}.checkable(showGrid),
                        MenuItem::Separator(),
                        MenuItem{"Reset"}.onSelect([&showGrid]() { showGrid = false; }),
                    })
            }
        }
        // The inverse of a Dialog: a Window resizes unless it is told not to.
        .Fixed();
}

// FilePicker + TextCtrl over one path string, plus a second picker on the same
// value. Three controls, one std::string: pick a file in either picker, or type
// into the plain field, and the other two follow.
//
// This is the binding demo T3.1 owes (rules.md G2), and it shows the one thing
// a FilePicker does that a plain field cannot: both ways of setting the path --
// the dialog and the keyboard -- commit through the same value, so nothing
// downstream can tell them apart.
//
// The disabled pair is bound too: tick the box and both pickers grey out,
// leaving the plain field as the only way to change the value. That is the
// clearest way to see the three controls really are one value.
inline auto drawFilePickerBinding(std::string& path, bool& pickersDisabled)
{
    constexpr int kRowH = 28;
    constexpr int kLabelH = 20;
    constexpr int kLabelW = 96;
    constexpr Size kFieldSize { 360, 28 };

    return
        Dialog {
            "FilePicker + TextCtrl (shared path)",
            VStack {
                LayoutFlags().Expand().Border(Side::All, 12),
                StaticText{"One std::string behind all three controls:"}
                    .withSize({-1, kLabelH}),

                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 8),
                    StaticText{"Picker:"}
                        .withSize({kLabelW, kLabelH})
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8)),
                    FilePicker{path}
                        .withMode(FileMode::Open)
                        .withFilter("Sources (*.cpp;*.hpp)|*.cpp;*.hpp|All files|*")
                        .withDialogTitle("Pick the shared path")
                        .withSize(kFieldSize)
                        .isDisabled(pickersDisabled)
                },

                // A second picker on the SAME value, in Save mode. Pick in one and
                // the other's field follows -- no callback wiring, no copy-back.
                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 6),
                    StaticText{"Save as:"}
                        .withSize({kLabelW, kLabelH})
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8)),
                    FilePicker{path}
                        .withMode(FileMode::Save)
                        .withDialogTitle("Save the shared path as")
                        .withSize(kFieldSize)
                        .isDisabled(pickersDisabled)
                },

                // The plain field: the same string again, and the one control that
                // stays live when the box below is ticked.
                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 6),
                    StaticText{"As text:"}
                        .withSize({kLabelW, kLabelH})
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8)),
                    TextCtrl{path}
                        .withSize(kFieldSize)
                },

                Separator{}
                    .withSize({-1, 1})
                    .withFlags(LayoutFlags().Expand().Border(Side::Top, 10)),

                CheckBox{pickersDisabled, "Disable both pickers (the text field stays live)"}
                    .withSize({-1, kRowH})
                    .withFlags(LayoutFlags().Border(Side::Top, 10))
            }
        };
}

// Two CheckListBoxes over ONE std::vector<std::string>: tick a box in either and
// the other follows, because both are bound to the same caller-owned value and
// the checked set IS that value. A third control, a plain ListBox on the same
// vector, shows the set read as a multi-selection -- the two widgets decode the
// bound type through the same helpers, so "ticked" and "selected" are the same
// list of items spelled two ways.
//
// The lists deliberately carry the SAME items in a different order: the checked
// set names items by text, not by position, so the second list ticks the right
// rows anyway. A std::vector<int> binding would name them by position and the
// two lists would disagree -- which is the whole reason both spellings exist.
inline auto drawCheckListBinding(std::vector<std::string>& modules, bool& listsDisabled)
{
    constexpr int kRowH = 28;
    constexpr int kLabelH = 20;
    constexpr int kListW = 190;

    return
        Dialog {
            "CheckListBox x2 (shared checked set)",
            VStack {
                LayoutFlags().Expand().Border(Side::All, 12),
                StaticText{"One std::vector<std::string> behind all three lists:"}
                    .withSize({-1, kLabelH}),

                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 8),
                    VStack {
                        LayoutFlags().Expand(),
                        StaticText{"Declared order:"}.withSize({-1, kLabelH}),
                        CheckListBox{
                            {"Core", "Network", "Storage", "Rendering", "Audio"},
                            modules}
                            .withVisibleRows(5)
                            .withSize({kListW, -1})
                            .withFlags(LayoutFlags().Border(Side::Top, 4))
                            .isDisabled(listsDisabled)
                    },
                    Spacer{Size{12, 0}},
                    VStack {
                        LayoutFlags().Expand(),
                        StaticText{"Reversed:"}.withSize({-1, kLabelH}),
                        CheckListBox{
                            {"Audio", "Rendering", "Storage", "Network", "Core"},
                            modules}
                            .withVisibleRows(5)
                            .withSize({kListW, -1})
                            .withFlags(LayoutFlags().Border(Side::Top, 4))
                            .isDisabled(listsDisabled)
                    },
                    Spacer{Size{12, 0}},
                    // The same vector as a multi-select ListBox: what the check
                    // lists call "ticked" this one calls "selected".
                    VStack {
                        LayoutFlags().Expand(),
                        StaticText{"As a ListBox:"}.withSize({-1, kLabelH}),
                        ListBox{
                            {"Core", "Network", "Storage", "Rendering", "Audio"},
                            modules}
                            .withVisibleRows(5)
                            .withSize({kListW, -1})
                            .withFlags(LayoutFlags().Border(Side::Top, 4))
                    }
                },

                Separator{}
                    .withSize({-1, 1})
                    .withFlags(LayoutFlags().Expand().Border(Side::Top, 10)),

                CheckBox{listsDisabled, "Disable both check lists (the ListBox stays live)"}
                    .withSize({-1, kRowH})
                    .withFlags(LayoutFlags().Border(Side::Top, 10))
            }
        };
}

// T3.4: the two ProgressBar spellings over one caller-owned float and one
// caller-owned bool.
//
// `progress` is shared by the Slider and the determinate bar, so dragging the
// slider fills the bar with no wiring at all. The indeterminate bar beside it
// is bound to nothing: there is no number, and that is the point.
//
// `busy` is what picks which of the two is live. isDisabled() binds a bool, not
// an expression, so the complement is a second caller-owned bool the checkbox
// keeps in step -- writing `!busy` into the call would snapshot the value at
// build time and never change again.
//
// macOS caveat, the same one this file opens with: a disabled wxGauge or
// QProgressBar is drawn exactly like an enabled one, so watch the SLIDER to see
// the toggle land. The animation itself is visible everywhere.
inline auto drawIndeterminateProgressBinding(float& progress, bool& busy, bool& idle)
{
    constexpr int kRowH = 28;
    constexpr int kLabelH = 20;
    constexpr int kLabelW = 96;
    constexpr Size kBarSize { 320, 20 };

    return
        Dialog {
            "ProgressBar: value vs Indeterminate()",
            VStack {
                LayoutFlags().Expand().Border(Side::All, 12),
                StaticText{"One float behind the slider below and the top bar:"}
                    .withSize({-1, kLabelH}),

                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 8),
                    StaticText{"Progress:"}
                        .withSize({kLabelW, kLabelH})
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8)),
                    ProgressBar{progress}
                        .withSize(kBarSize)
                        .withFlags(LayoutFlags().CenterVertical())
                        .isDisabled(busy)
                },

                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 6),
                    StaticText{"Indexing…"}
                        .withSize({kLabelW, kLabelH})
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8)),
                    // No value, so nothing to share: the mode replaces the number.
                    ProgressBar{}
                        .Indeterminate()
                        .withSize(kBarSize)
                        .withFlags(LayoutFlags().CenterVertical())
                        .isDisabled(idle)
                },

                Separator{}
                    .withSize({-1, 1})
                    .withFlags(LayoutFlags().Expand().Border(Side::Top, 10)),

                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 10),
                    StaticText{"Drag:"}
                        .withSize({kLabelW, kLabelH})
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8)),
                    Slider { Range<float>{ .min = 0.0f, .max = 100.0f }, progress }
                        .withFlags(LayoutFlags().Expand().CenterVertical())
                        .isDisabled(busy)
                },

                // The one write that keeps the complement honest. `busy` is bound,
                // so the checkbox has already stored the new state by the time this
                // runs -- value first, then the callback, as everywhere else.
                CheckBox{busy, "Busy (work of unknown length) -- disables the determinate half"}
                    .withSize({-1, kRowH})
                    .withFlags(LayoutFlags().Border(Side::Top, 12))
                    .onChange([&idle](bool nowBusy) { idle = !nowBusy; })
            }
        };
}

// T3.6: a Dialog's OPEN FLAG as an ordinary bound value.
//
// `detailsOpen` has three holders here -- the check box, the toggle button, and
// the dialog's own lifecycle -- and they are one bool. Untick either control and
// the dialog closes; close the dialog from its title bar and both controls come
// back up, because the flag is cleared from the same place onClose() fires from.
// Nothing in this file wires the two together.
//
// The one asymmetry is OPENING, and it is why both controls carry an onChange
// that calls `openDetails`. Clearing the flag is enough to close a dialog on
// every backend, but setting it back is only enough to REOPEN one on ImGui,
// where the main calls show() every frame. wx and Qt destroyed the native dialog
// and freed its engine session with it, so there a second show() call is what
// brings one back -- which is exactly what the main's version of `openDetails`
// does. That settles the question T2.1's shell left open: re-showing works
// everywhere, but on a retained backend it is a call, not a flag.
//
// Ticking can only happen while the dialog is down, so the two controls cannot
// stack up two dialogs between them: RefSync mirrors the other control's state
// with wx's non-notifying setters and Qt's QSignalBlocker, so a mirrored write
// never re-enters onChange.
//
// `detailsReport` is written by the dialog's onClose() and mirrored here through
// a bound TextCtrl -- the only shape that updates live on the retained backends.
// The count beside it is what makes "exactly once" readable.
inline auto drawDialogBinding(bool& detailsOpen, std::string& note,
    std::string& detailsReport, bool& disabled, std::function<void()> openDetails)
{
    constexpr int kRowH = 28;
    constexpr int kLabelH = 20;

    return
        Dialog {
            "Dialog controls (shared bool)",
            VStack {
                LayoutFlags().Expand().Border(Side::All, 12).MinSize({440, -1}),
                StaticText{"A dialog's open flag, a check box and a toggle: one bool."}
                    .withSize({-1, kLabelH}),

                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 8),
                    CheckBox{detailsOpen, "Details dialog is open"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Proportion(1).CenterVertical())
                        .isDisabled(disabled)
                        .onChange([openDetails](bool nowOpen) {
                            if (nowOpen)
                                openDetails();
                        }),
                    ToggleButton{detailsOpen, "Details"}
                        .withSize({110, kRowH})
                        .withFlags(LayoutFlags().CenterVertical())
                        .isDisabled(disabled)
                        .withTooltip("Untick to close the dialog; tick to open it again")
                        .onChange([openDetails](bool nowOpen) {
                            if (nowOpen)
                                openDetails();
                        })
                },

                Separator{}
                    .withSize({-1, 1})
                    .withFlags(LayoutFlags().Expand().Border(Side::Top, 10)),

                StaticText{"What the dialog's onClose() last reported:"}
                    .withSize({-1, kLabelH})
                    .withFlags(LayoutFlags().Border(Side::Top, 10)),
                TextCtrl{detailsReport}
                    .withSize({-1, kRowH})
                    .withFlags(LayoutFlags().Expand().Border(Side::Top, 4)),

                StaticText{"A note the dialog and this panel share:"}
                    .withSize({-1, kLabelH})
                    .withFlags(LayoutFlags().Border(Side::Top, 10)),
                // The ordinary two-control binding, across a window boundary: type
                // here and the field inside the dialog follows, and the other way
                // round. It survives a close and a reopen because the value belongs
                // to the caller, not to either dialog.
                TextCtrl{note}
                    .withSize({-1, kRowH})
                    .withFlags(LayoutFlags().Expand().Border(Side::Top, 4)),

                CheckBox{disabled, "Lock both controls"}
                    .withSize({-1, kRowH})
                    .withFlags(LayoutFlags().Border(Side::Top, 12))
            }
        };
}

// The dialog the panel above opens and closes. Deliberately NOT Modal(): a
// modal one could not be closed from the panel, because the panel would not be
// accepting input. Resizable(), so the other Dialog default is on show too.
inline auto drawDetailsUI(bool& open, std::string& note, std::string& detailsReport,
    int& closeCount)
{
    constexpr int kRowH = 28;
    constexpr int kLabelH = 20;
    constexpr Size kButtonSize { 110, 28 };

    return
        Dialog {
            "Details",
            VStack {
                LayoutFlags().Expand().Border(Side::All, 12).MinSize({340, -1}),
                StaticText{"Shown against the panel's bool. Close me from my title"}
                    .withSize({-1, kLabelH}),
                StaticText{"bar, from the button below, or from the panel."}
                    .withSize({-1, kLabelH}),

                StaticText{"The shared note:"}
                    .withSize({-1, kLabelH})
                    .withFlags(LayoutFlags().Border(Side::Top, 10)),
                TextCtrl{note}
                    .withSize({-1, kRowH})
                    .withFlags(LayoutFlags().Expand().Border(Side::Top, 4)),

                Separator{}
                    .withSize({-1, 1})
                    .withFlags(LayoutFlags().Expand().Border(Side::Top, 12)),

                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 12),
                    Spacer{},
                    // Closing from inside is the same act as closing from the title
                    // bar: clear the bool and the dialog follows.
                    Button{"Close"}
                        .withSize(kButtonSize)
                        .withFlags(LayoutFlags().CenterVertical())
                        .onClick([&open]() { open = false; })
                }
            }
        }
        .Resizable()
        .onClose([&detailsReport, &closeCount]() {
            detailsReport = "onClose() fired " + std::to_string(++closeCount)
                + "x -- once per close, never twice.";
        });
}

// T4.1: a Toast built from caller-owned values.
//
// A Toast holds no binding of its own -- it is shown and forgotten, and its
// message is COPIED at show() -- so what binds here is everything that feeds
// it. The message field, the style combo and the duration are each a
// caller-owned value, and the duration has two holders: drag the slider and the
// spin box follows, type in the spin box and the slider moves. Whatever the
// three say at the moment of the click is what the toast shows; editing the
// message afterwards does not rewrite a toast that is already up.
//
// "Show three" raises three at once to make the STACKING visible: they sit one
// above another in the bottom-right corner, oldest lowest, and as each expires
// the rest slide down. `locked` is bound to both buttons' isDisabled().
inline auto drawToastBinding(std::string& message, int& style, int& durationMs, bool& locked)
{
    constexpr int kRowH = 28;
    constexpr int kLabelH = 20;
    constexpr int kLabelW = 80;
    constexpr Size kButtonSize { 130, 28 };

    // Built fresh in each handler from whatever the caller's values say NOW --
    // not captured by value at build time, which on wx and Qt would freeze the
    // settings the dialog opened with.
    auto showToast = [&message, &style, &durationMs](const std::string& suffix) {
        Toast{message + suffix}
            .withStyle(static_cast<MessageBoxStyle>(style))
            .withDuration(durationMs)
            .show();
    };

    return
        Dialog {
            "Toast controls (shared values)",
            VStack {
                LayoutFlags().Expand().Border(Side::All, 12).MinSize({440, -1}),
                StaticText{"The toast is built from these values at the moment you click."}
                    .withSize({-1, kLabelH}),

                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 10),
                    StaticText{"Message:"}
                        .withSize({kLabelW, kLabelH})
                        .withFlags(LayoutFlags().CenterVertical()),
                    TextCtrl{message}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Proportion(1).CenterVertical())
                },
                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 8),
                    StaticText{"Style:"}
                        .withSize({kLabelW, kLabelH})
                        .withFlags(LayoutFlags().CenterVertical()),
                    // Indices in MessageBoxStyle's declaration order.
                    ComboBox{ {"Info", "Warning", "Error", "Question"}, style }
                        .withSize({160, kRowH})
                        .withFlags(LayoutFlags().CenterVertical())
                },
                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 8),
                    StaticText{"Duration:"}
                        .withSize({kLabelW, kLabelH})
                        .withFlags(LayoutFlags().CenterVertical()),
                    Slider { Range<int>{ .min = 500, .max = 8000, .step = 100 }, durationMs }
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Proportion(1).CenterVertical()),
                    SpinBox { Range<int>{ .min = 500, .max = 8000, .step = 100 }, durationMs }
                        .withSize({100, kRowH})
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Left, 8)),
                    StaticText{"ms"}
                        .withSize({24, kLabelH})
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Left, 6))
                },

                Separator{}
                    .withSize({-1, 1})
                    .withFlags(LayoutFlags().Expand().Border(Side::Top, 10)),

                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 10),
                    Spacer{},
                    Button{"Show three"}
                        .withSize(kButtonSize)
                        .withFlags(LayoutFlags().Border(Side::Right, 8))
                        .withTooltip("Three at once, to see them stack")
                        .isDisabled(locked)
                        .onClick([showToast]() {
                            showToast(" (1 of 3)");
                            showToast(" (2 of 3)");
                            showToast(" (3 of 3)");
                        }),
                    Button{"Show toast"}
                        .withSize(kButtonSize)
                        .isDisabled(locked)
                        .onClick([showToast]() { showToast(""); })
                },

                CheckBox{locked, "Lock both buttons"}
                    .withSize({-1, kRowH})
                    .withFlags(LayoutFlags().Border(Side::Top, 12))
            }
        };
}

// T4.2: RichText is read-only, so there is no value of its own to bind. What
// binds is what surrounds it:
//
//   * `locked` is shared by the check box and BOTH RichTexts' isDisabled():
//     tick it and the text greys out and its links go inert on every backend
//     (ImGui dims it the way it dims every control in a disabled scope).
//   * `lastLink` and `linkClicks` are written by the links -- one RichText uses
//     onLink(url), the other the onLink(url, void*) overload that also hands
//     over the native handle -- and read back by the two fields below, the
//     count through a SpinBox that can reset it by hand.
inline auto drawRichTextBinding(std::string& lastLink, int& linkClicks, bool& locked)
{
    constexpr int kRowH = 28;
    constexpr int kLabelH = 20;
    constexpr int kLabelW = 90;
    constexpr int kTextW = 400;

    return
        Dialog {
            "RichText (shared values)",
            VStack {
                LayoutFlags().Expand().Border(Side::All, 12),
                RichText{"Pick a colour: [**red**](red), [*green*](green) or "
                         "[{#3060ff}blue{/}](blue). Each link writes its url into the "
                         "field below."}
                    .withSize({kTextW, -1})
                    .isDisabled(locked)
                    .onLink([&lastLink, &linkClicks](const std::string& url) {
                        lastLink = url;
                        ++linkClicks;
                    }),
                RichText{"This one uses the *native-handle* overload: "
                         "[count me](count) -- the handle is non-null on wx and Qt."}
                    .withSize({kTextW, -1})
                    .withFlags(LayoutFlags().Border(Side::Top, 8))
                    .isDisabled(locked)
                    .onLink([&lastLink, &linkClicks](const std::string& url, void* native) {
                        lastLink = url + (native != nullptr ? " (native handle)" : " (no handle)");
                        ++linkClicks;
                    }),

                Separator{}
                    .withSize({-1, 1})
                    .withFlags(LayoutFlags().Expand().Border(Side::Top, 10)),

                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 10),
                    StaticText{"Last link:"}
                        .withSize({kLabelW, kLabelH})
                        .withFlags(LayoutFlags().CenterVertical()),
                    TextCtrl{lastLink}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Proportion(1).CenterVertical())
                },
                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 8),
                    StaticText{"Clicks:"}
                        .withSize({kLabelW, kLabelH})
                        .withFlags(LayoutFlags().CenterVertical()),
                    SpinBox { Range<int>{ .min = 0, .max = 1000, .step = 1 }, linkClicks }
                        .withSize({100, kRowH})
                        .withFlags(LayoutFlags().CenterVertical())
                },

                CheckBox{locked, "Disable both texts"}
                    .withSize({-1, kRowH})
                    .withFlags(LayoutFlags().Border(Side::Top, 12))
            }
        };
}

// Two ways a value can outlive the code that set it.
//
// postToUi(): "Start work" runs a worker thread that must not touch the bound
// progress or status itself -- the UI thread reads them. Each step is posted
// to the UI thread instead, and the last one clears `busy`, which re-enables
// the button and raises a toast. The same code runs on all three backends.
//
// withId(): two UNBOUND check boxes below a foldable section. On ImGui an
// unbound value is kept by the control's position in the tree, and opening the
// section above shifts that position -- so the unnamed box loses its tick when
// the section folds or unfolds, while the one named with withId() keeps it. On
// wx and Qt both keep their state natively; there the id is the native window
// or object name, which is what tests find the control by.
inline auto drawWorkerAndIdentityUI(float& progress, std::string& status, bool& busy, bool& detailsOpen,
    bool& hideStatusBar)
{
    return
        Dialog {
            "Background work & stable ids",
            VStack {
                LayoutFlags().Expand().Border(Side::All, 12),
                VGroupBox { "postToUi -- a worker thread updates bound values",
                    LayoutFlags().Expand(),
                    ProgressBar{progress}
                        .withSize({300, 18})
                        .withFlags(LayoutFlags().Expand()),
                    HStack {
                        LayoutFlags().Border(Side::Top, 8),
                        Button{"Start work"}
                            .isDisabled(busy)
                            .onClick([&progress, &status, &busy] {
                                busy = true;
                                progress = 0.0f;
                                status = "Working...";
                                std::thread([&progress, &status, &busy] {
                                    for (int step = 1; step <= 20; ++step)
                                    {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                                        postToUi([&progress, step] { progress = step * 5.0f; });
                                    }
                                    postToUi([&status, &busy] {
                                        status = "Done";
                                        busy = false;
                                        Toast{"Background work finished"}.show();
                                    });
                                }).detach();
                            })
                            .withId("worker-start"),
                        Spacer{}
                    },
                    // isHidden(): the bar leaves the layout entirely -- the dialog
                    // shrinks by its height and its gap -- and comes back when the
                    // box is unticked. Bound, so no rebuild is involved.
                    CheckBox{hideStatusBar, "Hide the status bar"}
                        .withFlags(LayoutFlags().Border(Side::Top, 8)),
                    StatusBar{status}
                        .isHidden(hideStatusBar)
                        .withFlags(LayoutFlags().Expand().Border(Side::Top, 8)),
                    // The same bound string in a label and a read-only field: both
                    // follow it live, and both keep the size they had for "Idle",
                    // so the dialog does not grow when "Working..." arrives. The
                    // label is pinned wide enough for the longest message.
                    HStack {
                        LayoutFlags().Border(Side::Top, 8),
                        StaticText{"Label:"}
                            .withSize({50, -1})
                            .withFlags(LayoutFlags().CenterVertical()),
                        StaticText{status}
                            .withSize({110, -1})
                            .withFlags(LayoutFlags().CenterVertical()),
                        StaticText{"Field:"}
                            .withSize({50, -1})
                            .withFlags(LayoutFlags().CenterVertical().Border(Side::Left, 8)),
                        ReadonlyTextCtrl{status}
                            .withFlags(LayoutFlags().Proportion(1))
                    }
                },
                VGroupBox { "withId -- unbound values that survive a fold",
                    LayoutFlags().Expand().Border(Side::Top, 10),
                    Expander { "Details above the boxes", detailsOpen,
                        VStack {
                            StaticText{"Opening or folding this section shifts every control below it."},
                            TextCtrl{std::string("filler")}
                        }
                    },
                    CheckBox{false, "Named with withId(): keeps its tick"}
                        .withId("worker-demo-named")
                        .withFlags(LayoutFlags().Border(Side::Top, 8)),
                    CheckBox{false, "Unnamed: may lose it on ImGui"}
                }
            }
        };
}

// VForEach: a list whose ROWS come from a vector. Each row is a function of
// its item and reports changes through callbacks by index; the vector is
// bound, so adding, ticking and removing all show up at once -- on ImGui by
// the per-frame rebuild, on wx and Qt by rebuilding just the rows that
// changed (every row when the count did). The dialog grows and shrinks with
// the list.
//
// The new-task field keeps its text OUTSIDE the vector on purpose: a field
// that wrote its own row's item on every keystroke would be rebuilt as you
// type on wx and Qt.
struct DemoTodo
{
    std::string title;
    bool done = false;

    bool operator==(const DemoTodo&) const = default;
};

inline auto drawTodoListUI(std::vector<DemoTodo>& todos, std::string& newTitle, std::string& keyStatus)
{
    // Both buttons carry an icon (withIcon, 16x16 left of the label).
    // Keyboard: "Add" is the default button (Enter anywhere adds) and "Clear"
    // the cancel button (Escape clears the field). The field's onEnter runs
    // FIRST, on all three -- here it only reports, into a bound label, so
    // the order is visible: the label names the text, then the row appears.
    return
        Dialog {
            "Todo list (VForEach)",
            VStack {
                LayoutFlags().Expand().Border(Side::All, 12),
                HStack {
                    TextCtrl{newTitle}
                        .withPlaceholder("New task, then Enter")
                        .withFlags(LayoutFlags().Proportion(1))
                        .onEnter([&keyStatus](const std::string& text) {
                            keyStatus = text.empty() ? "Enter: nothing to add" : "Enter: adding \"" + text + "\"";
                        }),
                    Button{"Add"}
                        .withIcon("images/icon_add.png")
                        .isDefault()
                        .withFlags(LayoutFlags().Border(Side::Left, 8))
                        .onClick([&todos, &newTitle] {
                            if (newTitle.empty())
                                return;
                            todos.push_back({ newTitle, false });
                            newTitle.clear();
                        }),
                    Button{"Clear"}
                        .withIcon("images/icon_clear.png")
                        .isCancel()
                        .withFlags(LayoutFlags().Border(Side::Left, 4))
                        .onClick([&newTitle, &keyStatus] {
                            newTitle.clear();
                            keyStatus = "Escape / Clear: field cleared";
                        })
                },
                StaticText{keyStatus}
                    .withSize({300, 20})
                    .withFlags(LayoutFlags().Border(Side::Top, 6)),
                VForEach {
                    LayoutFlags().Expand().Border(Side::Top, 10),
                    todos,
                    [&todos](const DemoTodo& todo, std::size_t i) {
                        return HStack {
                            LayoutFlags().Expand(),
                            CheckBox{todo.done, todo.title}
                                .onChange([&todos, i](bool on) { todos[i].done = on; }),
                            Spacer{},
                            Button{"Remove"}
                                .onClick([&todos, i] { todos.erase(todos.begin() + static_cast<long>(i)); })
                        };
                    }
                }
            }
        };
}

// TreeView items + Table rows, both BOUND: the buttons change the caller's
// vectors and nothing else, and both controls follow on every backend. The
// selection is kept by path / key across a refill, a folder the user opened
// stays open, and neither control resizes the window -- they keep the size of
// their first content, as a bound ListBox does. `filePick` is shown by a
// bound label, so the key binding is visible as it follows the rows.
//
// The rows are an Observable<TableRows>: bound exactly like a TableRows&, but
// changed through edit(), which counts the change -- so the retained backends
// poll one integer instead of comparing every row (review finding 7). The
// folders stay a plain vector, which is polled by comparison.
inline auto drawTreeTableBindingUI(std::vector<TreeItem>& folders, Observable<TableRows>& files,
    std::string& folderPick, std::string& filePick)
{
    return
        Dialog {
            "Bound tree + table",
            VStack {
                LayoutFlags().Expand().Border(Side::All, 12),
                HStack {
                    TreeView{ folders, folderPick }
                        .withVisibleRows(8)
                        .withSize({180, -1}),
                    Table{ { { "File", 180, true }, { "Size", 60, true } }, files, filePick }
                        .withVisibleRows(7)
                        .withFlags(LayoutFlags().Border(Side::Left, 10))
                },
                HStack {
                    LayoutFlags().Border(Side::Top, 10),
                    Button{"Add folder"}
                        .onClick([&folders] {
                            folders.push_back({ "Folder " + std::to_string(folders.size() + 1),
                                { { "notes" }, { "drafts" } }, true });
                        }),
                    Button{"Add file"}
                        .withFlags(LayoutFlags().Border(Side::Left, 6))
                        .onClick([&files] {
                            const std::size_t count = files.get().size();
                            files.edit().push_back({ "file-" + std::to_string(count + 1) + ".txt",
                                std::to_string(count * 7 + 3) });
                        }),
                    Button{"Grow selected"}
                        .withFlags(LayoutFlags().Border(Side::Left, 6))
                        .onClick([&files, &filePick] {
                            for (auto& row : files.edit())
                            {
                                if (!row.empty() && row[0] == filePick && row.size() > 1)
                                    row[1] = std::to_string(std::stoi(row[1]) * 2);
                            }
                        }),
                    Button{"Remove selected"}
                        .withFlags(LayoutFlags().Border(Side::Left, 6))
                        .onClick([&files, &filePick] {
                            std::erase_if(files.edit(), [&](const TableRow& row) { return !row.empty() && row[0] == filePick; });
                        })
                },
                HStack {
                    LayoutFlags().Border(Side::Top, 8),
                    StaticText{"Selected file:"}
                        .withSize({110, 20}),
                    StaticText{filePick}
                        .withSize({260, 20})
                }
            }
        };
}

// Focus and validity on the three text fields: the form opens with focus in
// Name (isFocused()), Email and Password are checked as the user leaves them
// (onBlur) and marked with isInvalid(bool&) -- the same bools that disable
// "Sign up", the default button. "Check" moves focus to the first bad field
// through Email's bound focus flag, and the notes field reports its focus
// moves into the status line. A field's mark never changes its size.
struct DemoSignUp
{
    std::string name;
    std::string email;
    std::string password;
    std::string notes;
    bool emailFocused = false;
    bool emailInvalid = false;
    bool passwordInvalid = false;
    bool cannotSubmit = true;
    std::string status = "Fill in the form";
};

inline auto drawSignUpFormUI(DemoSignUp& form)
{
    const auto validate = [&form] {
        form.emailInvalid = !form.email.empty() && form.email.find('@') == std::string::npos;
        form.passwordInvalid = !form.password.empty() && form.password.size() < 8;
        form.cannotSubmit = form.name.empty() || form.email.empty() || form.password.empty()
            || form.emailInvalid || form.passwordInvalid;
    };
    constexpr int kLabelW = 80;
    constexpr int kFieldW = 220;
    return
        Dialog {
            "Sign up (focus + validation)",
            VStack {
                LayoutFlags().Expand().Border(Side::All, 12),
                HStack {
                    StaticText{"Name"}.withSize({kLabelW, 20}).withFlags(LayoutFlags().CenterVertical()),
                    TextCtrl{form.name}
                        .withSize({kFieldW, -1})
                        .isFocused()
                        .onChange([validate](const std::string&) { validate(); })
                },
                HStack {
                    LayoutFlags().Border(Side::Top, 6),
                    StaticText{"Email"}.withSize({kLabelW, 20}).withFlags(LayoutFlags().CenterVertical()),
                    TextCtrl{form.email}
                        .withSize({kFieldW, -1})
                        .withPlaceholder("name@example.com")
                        .isFocused(form.emailFocused)
                        .isInvalid(form.emailInvalid)
                        .onBlur(validate)
                },
                HStack {
                    LayoutFlags().Border(Side::Top, 6),
                    StaticText{"Password"}.withSize({kLabelW, 20}).withFlags(LayoutFlags().CenterVertical()),
                    PasswordInput{form.password}
                        .withSize({kFieldW, -1})
                        .withPlaceholder("8 characters or more")
                        .isInvalid(form.passwordInvalid)
                        .onBlur(validate)
                },
                MultiLineTextCtrl{form.notes}
                    .withSize({kLabelW + kFieldW + 8, 60})
                    .withFlags(LayoutFlags().Border(Side::Top, 8))
                    .onFocus([&form] { form.status = "Writing notes..."; })
                    .onBlur([&form] { form.status = "Notes: " + std::to_string(form.notes.size()) + " characters"; }),
                StaticText{form.status}
                    .withSize({kLabelW + kFieldW + 8, 20})
                    .withFlags(LayoutFlags().Border(Side::Top, 6)),
                HStack {
                    LayoutFlags().Border(Side::Top, 8),
                    Spacer{},
                    Button{"Check"}
                        .onClick([&form, validate] {
                            validate();
                            if (form.emailInvalid || form.email.empty())
                            {
                                form.status = "Fix the email first";
                                form.emailFocused = true; // moves focus there
                            }
                            else
                            {
                                form.status = form.cannotSubmit ? "Something is still missing" : "Looks good";
                            }
                        }),
                    Button{"Sign up"}
                        .isDefault()
                        .isDisabled(form.cannotSubmit)
                        .withFlags(LayoutFlags().Border(Side::Left, 6))
                        .onClick([&form] { form.status = "Signed up as " + form.name; })
                }
            }
        };
}

// SearchField over one std::string, shared with a label that echoes it and a
// list it filters live: every edit (typed, cleared with the field's own
// button, or written from outside by "Show fruit") re-filters the bound
// ItemList through onChange, and the list follows on every backend. A bool
// disables the field and the button together.
struct DemoSearchBinding
{
    std::string query;
    ItemList all { "Apple", "Apricot", "Banana", "Blueberry", "Carrot", "Cherry",
                   "Cucumber", "Grape", "Leek", "Lemon", "Mango", "Pear" };
    ItemList shown = all;
    std::string picked;
    std::string lastSearch = "Press Enter to search";
    bool disabled = false;
};

inline void filterDemoItems(DemoSearchBinding& s)
{
    std::string needle = s.query;
    for (auto& c : needle)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    s.shown.clear();
    for (const auto& item : s.all)
    {
        std::string lower = item;
        for (auto& c : lower)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (lower.find(needle) != std::string::npos)
            s.shown.push_back(item);
    }
}

inline auto drawSearchBindingUI(DemoSearchBinding& s)
{
    return
        Dialog {
            "SearchField + list (shared query)",
            VStack {
                LayoutFlags().Expand().Border(Side::All, 12),
                SearchField{s.query}
                    .withPlaceholder("Filter produce...")
                    .withSize({240, -1})
                    .isDisabled(s.disabled)
                    .onChange([&s](const std::string&) { filterDemoItems(s); })
                    .onSearch([&s](const std::string& q) {
                        s.lastSearch = "Searched for \"" + q + "\": " + std::to_string(s.shown.size()) + " hits";
                    }),
                StaticText{s.query}
                    .withSize({240, 20})
                    .withFlags(LayoutFlags().Border(Side::Top, 6)),
                ListBox{s.shown, s.picked}
                    .withVisibleRows(6)
                    .withSize({240, -1})
                    .withFlags(LayoutFlags().Border(Side::Top, 6)),
                StaticText{s.lastSearch}
                    .withSize({240, 20})
                    .withFlags(LayoutFlags().Border(Side::Top, 6)),
                HStack {
                    LayoutFlags().Border(Side::Top, 8),
                    Button{"Show fruit"}
                        .isDisabled(s.disabled)
                        .onClick([&s] {
                            s.query = "an"; // written from outside: the field follows
                            filterDemoItems(s);
                        }),
                    CheckBox{s.disabled, "Disable"}
                        .withFlags(LayoutFlags().Border(Side::Left, 10).CenterVertical())
                }
            }
        };
}

// EditableCombo over one std::string, shared with a TextCtrl and a label --
// typing in either field moves the other -- and over a BOUND item list:
// "Remember" adds the current text to the suggestions, and the drop-down
// follows on every backend while keeping the text. A bool disables both
// fields and the button.
struct DemoComboBinding
{
    std::string tag = "urgent";
    ItemList recent { "urgent", "later", "waiting" };
    bool disabled = false;
};

inline auto drawEditableComboBindingUI(DemoComboBinding& s)
{
    return
        Dialog {
            "EditableCombo + TextCtrl (shared text)",
            VStack {
                LayoutFlags().Expand().Border(Side::All, 12),
                EditableCombo{s.tag, s.recent}
                    .withSize({220, -1})
                    .isDisabled(s.disabled),
                TextCtrl{s.tag}
                    .withSize({220, -1})
                    .withFlags(LayoutFlags().Border(Side::Top, 6))
                    .isDisabled(s.disabled),
                StaticText{s.tag}
                    .withSize({220, 20})
                    .withFlags(LayoutFlags().Border(Side::Top, 6)),
                HStack {
                    LayoutFlags().Border(Side::Top, 8),
                    Button{"Remember"}
                        .isDisabled(s.disabled)
                        .onClick([&s] {
                            if (!s.tag.empty() && std::find(s.recent.begin(), s.recent.end(), s.tag) == s.recent.end())
                                s.recent.push_back(s.tag);
                        }),
                    CheckBox{s.disabled, "Disable"}
                        .withFlags(LayoutFlags().Border(Side::Left, 10).CenterVertical())
                }
            }
        };
}

// Slider orientation and ticks: three vertical channel faders (minimum at the
// bottom on every backend, a tick every 25) each bound to an int shared with
// the SpinBox under it, and a horizontal master on a float with a tick every
// 0.25 while the value still moves in steps of 0.05. "Mute" disables every
// fader through one bound bool. Ticks only mark positions: on wx the native
// slider draws them, on macOS through the NSSlider tick count.
struct DemoMixer
{
    int bass = 40;
    int mid = 60;
    int treble = 75;
    float master = 0.8f;
    bool muted = false;
};

inline auto drawMixerUI(DemoMixer& m)
{
    const auto channel = [&m](const char* name, int& value) {
        return
            VStack {
                Slider{ Range<int>{ .min = 0, .max = 100 }, value }
                    .withOrientation(Orientation::Vertical)
                    .withTicks(25)
                    .withSize({40, 160})
                    .withFlags(LayoutFlags().Center())
                    .isDisabled(m.muted),
                SpinBox{ Range<int>{ .min = 0, .max = 100 }, value }
                    .withSize({70, -1})
                    .withFlags(LayoutFlags().Border(Side::Top, 6))
                    .isDisabled(m.muted),
                StaticText{name}
                    .withAlign(TextAlign::Center)
                    .withSize({70, 20})
            };
    };
    return
        Dialog {
            "Mixer (vertical sliders + ticks)",
            VStack {
                LayoutFlags().Expand().Border(Side::All, 12),
                HStack {
                    channel("Bass", m.bass),
                    Spacer{Size{16, 0}},
                    channel("Mid", m.mid),
                    Spacer{Size{16, 0}},
                    channel("Treble", m.treble)
                },
                StaticText{"Master"}
                    .withSize({260, 20})
                    .withFlags(LayoutFlags().Border(Side::Top, 12)),
                Slider{ Range<float>{ .min = 0.0f, .max = 1.0f, .step = 0.05f }, m.master }
                    .withTicks(0.25f)
                    .withSize({260, -1})
                    .isDisabled(m.muted),
                CheckBox{m.muted, "Mute"}
                    .withFlags(LayoutFlags().Border(Side::Top, 10))
            }
        };
}

// Spinner running state shared by reference: one bool drives a default
// spinner and a logo spinner, is flipped by a CheckBox and a ToggleButton
// alike, and disables the "Start work" button while it is set -- so the
// three controls and both spinners always agree on every backend.
inline auto drawSpinnerBindingUI(bool& busy, std::string& status)
{
    return
        Dialog {
            "Spinner (shared running flag)",
            VStack {
                LayoutFlags().Expand().Border(Side::All, 12),
                HStack {
                    Spinner{}.isRunning(busy).withFlags(LayoutFlags().CenterVertical()),
                    Spinner{}.withImage("images/logo_spinner.png").isRunning(busy).withSize({48, 48})
                        .withFlags(LayoutFlags().Border(Side::Left, 16)),
                    StaticText{status}
                        .withSize({180, 20})
                        .withFlags(LayoutFlags().Border(Side::Left, 16).CenterVertical())
                },
                HStack {
                    LayoutFlags().Border(Side::Top, 12),
                    CheckBox{busy, "Busy"}
                        .withFlags(LayoutFlags().CenterVertical())
                        .onChange([&status](bool on) { status = on ? "Working..." : "Idle"; }),
                    ToggleButton{busy, "Busy"}
                        .withFlags(LayoutFlags().Border(Side::Left, 10))
                        .onChange([&status](bool on) { status = on ? "Working..." : "Idle"; }),
                    Button{"Start work"}
                        .isDisabled(busy)
                        .withFlags(LayoutFlags().Border(Side::Left, 10))
                        .onClick([&busy, &status] {
                            busy = true;
                            status = "Working...";
                        })
                }
            }
        };
}

// Calendar + DatePicker over one Date: picking a day in either moves the
// other, "Today" and "Next week" write it from outside (the calendar brings
// that month back into view), and a bool disables both.
inline void addDays(Date& date, int days)
{
    date.day += days;
    while (date.day > daysInMonth(date.year, date.month))
    {
        date.day -= daysInMonth(date.year, date.month);
        if (++date.month > 12) { date.month = 1; ++date.year; }
    }
}

inline auto drawCalendarBindingUI(Date& date, bool& disabled)
{
    return
        Dialog {
            "Calendar + DatePicker (shared date)",
            VStack {
                LayoutFlags().Expand().Border(Side::All, 12),
                Calendar{date}
                    .withFirstDayOfWeek(FirstDayOfWeek::Sunday)
                    .isDisabled(disabled),
                DatePicker{date}
                    .withSize({230, -1})
                    .withFlags(LayoutFlags().Border(Side::Top, 8))
                    .isDisabled(disabled),
                HStack {
                    LayoutFlags().Border(Side::Top, 8),
                    Button{"Today"}
                        .isDisabled(disabled)
                        .onClick([&date] { date = todayDate(); }),
                    Button{"Next week"}
                        .isDisabled(disabled)
                        .withFlags(LayoutFlags().Border(Side::Left, 6))
                        .onClick([&date] { addDays(date, 7); }),
                    CheckBox{disabled, "Disable"}
                        .withFlags(LayoutFlags().Border(Side::Left, 10).CenterVertical())
                }
            }
        };
}

// VirtualList over caller-owned state: the selected row is one int shared with
// a SpinBox (type a row number and the list scrolls to it), the count is bound
// ("Add 1000 rows" grows the list live), and "Shout" changes every row's text
// in place and bumps the revision so the visible rows are asked again. A bool
// disables the list and its controls.
struct DemoVirtualList
{
    int count = 50000;
    int selected = 0;
    int revision = 0;
    bool shout = false;
    bool disabled = false;
};

inline auto drawVirtualListBindingUI(DemoVirtualList& s)
{
    return
        Dialog {
            "VirtualList + SpinBox (shared row)",
            VStack {
                LayoutFlags().Expand().Border(Side::All, 12),
                VirtualList{ s.count,
                    [&s](int i) {
                        const std::string text = "Customer " + std::to_string(i);
                        return s.shout ? text + "!!!" : text;
                    },
                    s.selected }
                    .withVisibleRows(8)
                    .withRevision(s.revision)
                    .withSize({260, -1})
                    .isDisabled(s.disabled),
                HStack {
                    LayoutFlags().Border(Side::Top, 8),
                    StaticText{"Row"}.withSize({40, 20}).withFlags(LayoutFlags().CenterVertical()),
                    SpinBox{ Range<int>{ .min = -1, .max = 10000000 }, s.selected }
                        .withSize({120, -1})
                        .isDisabled(s.disabled)
                },
                HStack {
                    LayoutFlags().Border(Side::Top, 8),
                    Button{"Add 1000 rows"}
                        .isDisabled(s.disabled)
                        .onClick([&s] { s.count += 1000; }),
                    Button{"Shout"}
                        .isDisabled(s.disabled)
                        .withFlags(LayoutFlags().Border(Side::Left, 6))
                        .onClick([&s] {
                            s.shout = !s.shout;
                            ++s.revision; // same count, new text: ask the rows again
                        }),
                    CheckBox{s.disabled, "Disable"}
                        .withFlags(LayoutFlags().Border(Side::Left, 10).CenterVertical())
                }
            }
        };
}
