#pragma once

// Layout primitives: the gallery dialog for Spacer and Separator.
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
