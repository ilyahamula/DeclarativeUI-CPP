#pragma once

// Layout primitives: the gallery dialog for Spacer.
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
            // No Separator between the two boxes yet: ImGui::Separator() spans
            // the window instead of the engine frame, so one here trips the
            // drift guard. T1.2 gives the separator an orientation and a
            // frame-bound draw, and brings its rows into this file.
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
