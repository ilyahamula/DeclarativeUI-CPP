#pragma once

// Layout primitives: the gallery dialogs for Spacer, Separator, Grid,
// ScrollPanel and Expander.
//
// controls_gallery.hpp is long past the ~20-element mark rule 4 sets, so the
// layout primitives land here instead, alongside a few already-implemented
// controls for context (StaticText, Button, CheckBox).
//
// A Spacer is the only leaf in the catalogue that is pure geometry: it creates
// no native window on wx or Qt and draws nothing on ImGui, so what you see is
// entirely the gap it opens between its siblings. Three shapes are on show:
//
//   * Spacer{}          flexible -- Proportion(1) by default, so it takes all
//                       the leftover on the parent's main axis and pushes its
//                       siblings to the two ends of the row.
//   * Spacer{px}        a fixed gap, on whichever axis the parent runs.
//   * Spacer{}.withFlags(LayoutFlags().Proportion(n))
//                       withFlags() replaces the default outright, which is how
//                       several spacers split the leftover in a chosen ratio --
//                       and equally how a fixed spacer opts back into flexing.
//
// This replaces the StaticText{""} hack the other galleries used to open their
// button rows with; every one of them now says Spacer{}.
//
// A Separator takes an Orientation. It measures a hairline on its own axis and
// nothing on the other, so the cross-axis default (Start, for a leaf) would draw
// a line of zero length -- which is why every divider below adds Expand(). That
// is what makes a horizontal one run the width of its column and a vertical one
// the height of its row.
//
// The hairline's thickness is the native one, and the three disagree: 1 px on
// ImGui, 2 on wx, 3 on Qt. The dividers below therefore pin it with withSize(),
// the same discipline the other galleries use on every leaf, so the frames come
// out identical on all three backends.

#include "declarative_ui.hpp"

#include <string>

inline auto drawLayoutPrimitivesUI(bool& rowsDisabled)
{
    // Same pinning discipline as the other galleries: explicit sizes on the
    // leaves and MinSize on the group boxes, so the three backends compute the
    // same frames despite their different native metrics.
    constexpr int kRowH = 28;
    constexpr int kLabelH = 20;
    constexpr int kBoxW = 460;
    constexpr Size kButtonSize { 96, 28 };
    constexpr Size kCloseSize { 110, 28 };

    return Dialog {
        "Layout Primitives",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 10),
            VGroupBox { "Flexible spacer",
                LayoutFlags().Expand().MinSize({kBoxW, -1}),
                StaticText{"One Spacer{} between the buttons pushes them to the two ends:"}
                    .withSize({-1, kLabelH}),
                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 6),
                    Button{"Left"}
                        .withSize(kButtonSize)
                        .isDisabled(rowsDisabled),
                    Spacer{},
                    Button{"Right"}
                        .withSize(kButtonSize)
                        .isDisabled(rowsDisabled)
                },
                StaticText{"Two spacers, weights 1 and 2, split the same leftover 1:2:"}
                    .withSize({-1, kLabelH})
                    .withFlags(LayoutFlags().Border(Side::Top, 10)),
                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 6),
                    Button{"A"}
                        .withSize(kButtonSize)
                        .isDisabled(rowsDisabled),
                    Spacer{},
                    Button{"B"}
                        .withSize(kButtonSize)
                        .isDisabled(rowsDisabled),
                    Spacer{}
                        .withFlags(LayoutFlags().Proportion(2)),
                    Button{"C"}
                        .withSize(kButtonSize)
                        .isDisabled(rowsDisabled)
                }
            },
            // The native hairline differs per backend (1 px on ImGui, 2 on wx,
            // 3 on Qt), so it is pinned here the way the galleries pin every
            // other leaf -- otherwise the rows below it would sit 1-2 px apart
            // across the three builds.
            Separator{}
                .withSize({-1, 1})
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 10)),
            VGroupBox { "Fixed spacer",
                LayoutFlags().Expand().MinSize({kBoxW, -1}).Border(Side::Top, 10),
                StaticText{"A fixed gap does not flex; the row stays left-packed:"}
                    .withSize({-1, kLabelH}),
                HStack {
                    LayoutFlags().Border(Side::Top, 6),
                    Button{"Cut"}
                        .withSize(kButtonSize)
                        .isDisabled(rowsDisabled),
                    // Spacer{px} is px on BOTH axes -- the widget cannot know
                    // which way its parent runs. The parent spends the main
                    // one; the other still asks the row for that much cross
                    // band, so Spacer{Size{px, 0}} is the spelling to reach for
                    // when the row must stay as tall as its controls.
                    Spacer{Size{40, 0}},
                    Button{"Copy"}
                        .withSize(kButtonSize)
                        .isDisabled(rowsDisabled),
                    Spacer{Size{40, 0}},
                    Button{"Paste"}
                        .withSize(kButtonSize)
                        .isDisabled(rowsDisabled)
                },
                // The same 24 on both axes costs nothing in a column this wide,
                // so the plain int form reads as a 24 px vertical gap here.
                Spacer{24},
                CheckBox{rowsDisabled, "Disable the buttons above"}
                    .withSize({-1, kRowH})
            },
            VGroupBox { "Vertical divider",
                LayoutFlags().Expand().MinSize({kBoxW, -1}).Border(Side::Top, 10),
                HStack {
                    LayoutFlags().Expand(),
                    VStack {
                        LayoutFlags().Proportion(1).Expand(),
                        StaticText{"Left column"}
                            .withSize({-1, kLabelH}),
                        Button{"One"}
                            .withSize(kButtonSize)
                            .withFlags(LayoutFlags().Border(Side::Top, 6))
                            .isDisabled(rowsDisabled)
                    },
                    // Expand() is what gives the line its length: without it the
                    // leaf's Start cross-alignment would leave it 0 px tall.
                    Separator{Orientation::Vertical}
                        .withSize({1, -1})
                        .withFlags(LayoutFlags().Expand().Border(Side::Left, 14).Border(Side::Right, 14)),
                    VStack {
                        LayoutFlags().Proportion(1).Expand(),
                        StaticText{"Right column"}
                            .withSize({-1, kLabelH}),
                        Button{"Two"}
                            .withSize(kButtonSize)
                            .withFlags(LayoutFlags().Border(Side::Top, 6))
                            .isDisabled(rowsDisabled)
                    }
                }
            },
            HStack {
                LayoutFlags().Border(Side::Top, 10),
                Spacer{},
                Button{"Close"}
                    .withSize(kCloseSize)
                    .withFlags(LayoutFlags().CenterVertical())
                    .onClick([]() {})
            }
        }
    };
}

// The Account form: the gallery dialog for Grid.
//
// Three columns, three rows, nine cells in declaration order -- no HStack per
// row and no SizeGroup per column. The labels line up because column 0 IS one
// band sized to the widest of them, and the fields line up for the same reason;
// change "E-mail:" to something longer and every field moves together.
//
// Column 1 carries Proportion(1) on its fields, so it takes all the spare width
// when the dialog is wider than the form needs. Note that a Proportion weights
// both bands its cell sits in -- the column and the row -- so the fields here
// deliberately do not add Expand() on rows that should keep their height.
//
// The labels are CenterVertical() so they sit against the middle of the taller
// field beside them; a cell's alignment resolves per axis, exactly as it does
// on a Stack's cross axis.
inline auto drawAccountFormUI(
    std::string& name,
    std::string& email,
    std::string& password,
    bool& formDisabled)
{
    constexpr int kLabelH = 22;
    constexpr int kFieldH = 26;
    constexpr Size kSideButton { 90, 26 };
    constexpr Size kFootButton { 100, 28 };

    return Dialog {
        "Account",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 12),
            Grid { 3, LayoutFlags().Expand().MinSize({520, -1}),
                StaticText{"Name:"}
                    .withSize({-1, kLabelH})
                    .withFlags(LayoutFlags().CenterVertical()),
                TextCtrl{name}
                    .withSize({-1, kFieldH})
                    .withFlags(LayoutFlags().Proportion(1).Expand())
                    .isDisabled(formDisabled),
                Button{"Check"}
                    .withSize(kSideButton)
                    .withFlags(LayoutFlags().CenterVertical())
                    .isDisabled(formDisabled),

                StaticText{"E-mail:"}
                    .withSize({-1, kLabelH})
                    .withFlags(LayoutFlags().CenterVertical()),
                TextCtrl{email}
                    .withSize({-1, kFieldH})
                    .withFlags(LayoutFlags().Proportion(1).Expand())
                    .isDisabled(formDisabled),
                Button{"Verify"}
                    .withSize(kSideButton)
                    .withFlags(LayoutFlags().CenterVertical())
                    .isDisabled(formDisabled),

                StaticText{"Password:"}
                    .withSize({-1, kLabelH})
                    .withFlags(LayoutFlags().CenterVertical()),
                PasswordInput{password}
                    .withSize({-1, kFieldH})
                    .withFlags(LayoutFlags().Proportion(1).Expand())
                    .isDisabled(formDisabled),
                Button{"Show"}
                    .withSize(kSideButton)
                    .withFlags(LayoutFlags().CenterVertical())
                    .isDisabled(formDisabled)
            },
            CheckBox{formDisabled, "Lock the form"}
                .withSize({-1, kFieldH})
                .withFlags(LayoutFlags().Border(Side::Top, 10)),
            Separator{}
                .withSize({-1, 1})
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 10)),
            HStack {
                LayoutFlags().Border(Side::Top, 10),
                Spacer{},
                Button{"OK"}
                    .withSize(kFootButton)
                    .onClick([]() {}),
                Button{"Cancel"}
                    .withSize(kFootButton)
                    .withFlags(LayoutFlags().Border(Side::Left, 8))
                    .onClick([]() {})
            }
        }
    };
}

// The scrollable option list: the gallery dialog for ScrollPanel.
//
// 600 px of options inside a 240 px viewport. The panel's own height is
// min(content, MaxSize, LayoutEngine::kDefaultScrollViewport), which is the
// whole point: the dialog auto-fits to everything EXCEPT the list, and the list
// scrolls inside whatever is left.
//
// The scrollbar gutter is reserved on every backend and reserved always, so the
// three compute the same content frames and nothing is ever drawn underneath a
// scrollbar. The second panel scrolls the other way to show that the rules are
// per axis: a horizontal panel caps its width and leaves its height alone.
inline auto drawScrollPanelUI(bool& optionsDisabled)
{
    constexpr int kRowH = 30;
    constexpr int kPanelW = 340;

    return Dialog {
        "Scrollable Options",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 12),
            StaticText{"600 px of options in a 240 px viewport (scroll it):"}
                .withSize({-1, 20}),
            ScrollPanel { LayoutFlags().Expand().Border(Side::Top, 6).MinSize({kPanelW, -1}),
                VStack {
                    // MinSize pins the content extent, so the demo really is
                    // 600 px of content on all three backends however tall each
                    // one draws a check box.
                    LayoutFlags().Expand().MinSize({-1, 600}),
                    CheckBox{false, "Option 1"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand()),
                    CheckBox{false, "Option 2"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand()),
                    CheckBox{true, "Option 3"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand()),
                    CheckBox{false, "Option 4"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand()),
                    CheckBox{false, "Option 5"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand()),
                    CheckBox{true, "Option 6"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand()),
                    CheckBox{false, "Option 7"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand()),
                    CheckBox{false, "Option 8"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand()),
                    CheckBox{true, "Option 9"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand()),
                    CheckBox{false, "Option 10"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand()),
                    CheckBox{false, "Option 11"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand()),
                    CheckBox{true, "Option 12"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand())
                }
                .isDisabled(optionsDisabled)
            },
            CheckBox{optionsDisabled, "Disable the whole list"}
                .withSize({-1, kRowH})
                .withFlags(LayoutFlags().Border(Side::Top, 10)),
            Separator{}
                .withSize({-1, 1})
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 10)),
            StaticText{"A wide row in a horizontal panel: the width caps, the height does not."}
                .withSize({-1, 20})
                .withFlags(LayoutFlags().Border(Side::Top, 10)),
            ScrollPanel { LayoutFlags().Expand().Border(Side::Top, 6),
                HStack {
                    LayoutFlags().Expand(),
                    Button{"One"}.withSize({120, 28}),
                    Button{"Two"}.withSize({120, 28}),
                    Button{"Three"}.withSize({120, 28}),
                    Button{"Four"}.withSize({120, 28}),
                    Button{"Five"}.withSize({120, 28}),
                    Button{"Six"}.withSize({120, 28})
                }
            }
            .withScroll(ScrollAxis::Horizontal),
            HStack {
                LayoutFlags().Border(Side::Top, 12),
                Spacer{},
                Button{"Close"}
                    .withSize({110, 28})
                    .withFlags(LayoutFlags().CenterVertical())
                    .onClick([]() {})
            }
        }
    };
}


// Expander: the "Settings" gallery dialog.
//
// Three collapsible sections over ordinary controls. Collapsed, a section costs
// the dialog EXACTLY its header -- no content, no margins, and not even the gap
// the content would have sat below -- so an auto-fit dialog shrinks as sections
// close and grows as they open. Watch the window height as you click.
//
// The header is native where a native one exists (wxCollapsibleHeaderCtrl, a
// checkable QToolButton) and drawn on the draw list on ImGui, whose own
// CollapsingHeader always spans the whole window and would overrun the engine's
// rectangle in a narrow column -- the same reason Separator draws its own line.
//
// Every value inside is bound to a caller-owned variable rather than declared
// from a literal. That is deliberate: on ImGui a folded section stops consuming
// widget ids, so the controls after it renumber, and an unbound snapshot is
// keyed by exactly that numbering. Bound values are the caller's variables and
// do not care.
//
// The dialog pins its width with MinSize so opening a section changes the
// height and nothing else; without it the widest open section would decide how
// wide the whole dialog is from one click to the next.
inline auto drawExpanderUI(
    bool& basicOpen,
    bool& advancedOpen,
    bool& networkOpen,
    bool& logging,
    int& level,
    int& retries,
    std::string& proxy,
    bool& sectionsDisabled)
{
    constexpr int kRowH = 28;
    constexpr int kLabelH = 20;
    constexpr int kDialogW = 420;
    constexpr int kLabelW = 90;

    return Dialog {
        "Settings",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 12).MinSize({kDialogW, -1}),
            StaticText{"Click a header: the dialog grows and shrinks with it."}
                .withSize({-1, kLabelH}),

            Expander { "Basic",
                LayoutFlags().Expand().Border(Side::Top, 10),
                basicOpen,
                VStack {
                    LayoutFlags().Expand().Border(Side::Left, 16),
                    CheckBox{logging, "Enable logging"}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Expand()),
                    HStack {
                        LayoutFlags().Expand().Border(Side::Top, 6),
                        StaticText{"Level:"}
                            .withSize({kLabelW, kRowH})
                            .withFlags(LayoutFlags().CenterVertical()),
                        ComboBox{ {"Error", "Warning", "Info", "Debug"}, level }
                            .withSize({-1, kRowH})
                            .withFlags(LayoutFlags().Proportion(1).Expand())
                    }
                }
            }
            .isDisabled(sectionsDisabled),

            Expander { "Advanced",
                LayoutFlags().Expand().Border(Side::Top, 8),
                advancedOpen,
                VStack {
                    LayoutFlags().Expand().Border(Side::Left, 16),
                    HStack {
                        LayoutFlags().Expand(),
                        StaticText{"Retries:"}
                            .withSize({kLabelW, kRowH})
                            .withFlags(LayoutFlags().CenterVertical()),
                        SpinBox { Range<int>{ .min = 0, .max = 10 }, retries }
                            .withSize({90, kRowH})
                    },
                    StaticText{"This section starts collapsed; nothing below it moved."}
                        .withSize({-1, kLabelH})
                        .withFlags(LayoutFlags().Border(Side::Top, 6))
                }
            }
            .isDisabled(sectionsDisabled),

            Expander { "Network",
                LayoutFlags().Expand().Border(Side::Top, 8),
                networkOpen,
                HStack {
                    LayoutFlags().Expand().Border(Side::Left, 16),
                    StaticText{"Proxy:"}
                        .withSize({kLabelW, kRowH})
                        .withFlags(LayoutFlags().CenterVertical()),
                    TextCtrl{proxy}
                        .withSize({-1, kRowH})
                        .withFlags(LayoutFlags().Proportion(1).Expand()),
                    Button{"Test"}
                        .withSize({80, kRowH})
                        .withFlags(LayoutFlags().Border(Side::Left, 6))
                        .onClick([]() {})
                }
            }
            .isDisabled(sectionsDisabled),

            CheckBox{sectionsDisabled, "Lock every section (headers included)"}
                .withSize({-1, kRowH})
                .withFlags(LayoutFlags().Border(Side::Top, 12)),
            Separator{}
                .withSize({-1, 1})
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 10)),
            HStack {
                LayoutFlags().Border(Side::Top, 10),
                Spacer{},
                Button{"Close"}
                    .withSize({110, 28})
                    .withFlags(LayoutFlags().CenterVertical())
                    .onClick([]() {})
            }
        }
    };
}
