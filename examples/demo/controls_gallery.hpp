#pragma once

// The widget gallery: one dialog exercising every implemented control.
// New UI elements belong here (or in a follow-on dialog once this one passes
// ~20 elements) -- see rule 4 in CLAUDE.md.

#include "declarative_ui.hpp"

#include <functional>
#include <string>

inline auto drawControlsUI(
    std::string& multilineText,
    std::string& password,
    int& spinInt,
    float& spinFloat,
    Date& date,
    Time& time,
    bool& toggle,
    float& progress,
    std::string& tabNote,
    bool& tabLogging,
    Color& themeColor,
    std::function<void()> onCheckClick = []() {},
    std::function<void()> onMessageBoxClick = []() {})
{
    // A spread of controls carries .withTooltip("..."), the leaf-only hover-text
    // modifier: snapshot text here, the live-bound form in value_binding.hpp.
    // It is pure decoration -- it never reaches the engine, so nothing below
    // moves because of it.
    //
    // Pinned geometry: every leaf carries an explicit size (withSize; -1
    // keeps that axis engine-driven) and every group box / tab panel a
    // MinSize outer box generous enough to absorb per-backend chrome
    // (group-box caption, tab bar). Native font/metric differences then
    // never reach the engine, so it computes the same frames relative to
    // the dialog on ImGui, wx and Qt. Non-resizable: the auto-fit result
    // is the final window size.
    constexpr int kBoxW = 340;   // outer width of every group box / tab panel
    constexpr int kRowH = 26;    // single-line input height
    constexpr int kLabelH = 20;  // static/link text height
    constexpr int kLabelW = 80;  // row-label width in labeled rows
    constexpr int kFieldW = 140;  // paired field width (spin boxes)
    constexpr int kPickerW = 230; // date/time picker width (>= imgui intrinsic)
    constexpr Size kButtonSize { 110, 28 };

    return Dialog {
        "Controls Demo",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 10),
            HStack {
                // Left column
                VStack {
                    VGroupBox { "Text Input",
                        LayoutFlags().MinSize({kBoxW, 200}),
                        MultiLineTextCtrl{multilineText}
                            .withSize({-1, 90})
                            .withFlags(LayoutFlags().Expand())
                            .withTooltip("Free-form notes. Scrolls once it fills up."),
                        HStack {
                            StaticText{"Password:"}
                                .withSize({kLabelW, kLabelH})
                                .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 5)),
                            PasswordInput{password}
                                .withSize({-1, kRowH})
                                .withFlags(LayoutFlags().Proportion(1))
                                .withTooltip("At least 8 characters, one of them a digit.")
                        },
                        LinkText{"Visit documentation"}
                            .withSize({-1, kLabelH})
                            .withFlags(LayoutFlags().Border(Side::Top, 5))
                            .onClick([]() {})
                            .withTooltip("Opens the online reference in your browser.")
                    },
                    HGroupBox { "Numeric Values",
                        LayoutFlags().MinSize({kBoxW, 90}).Border(Side::Top, 8),
                        VStack {
                            StaticText{"Integer"}
                                .withSize({kFieldW, kLabelH}),
                            SpinBox { { .min = 0, .max = 100 }, spinInt }
                                .withSize({kFieldW, kRowH})
                                .withTooltip("Whole numbers, 0 to 100.")
                        },
                        VStack {
                            StaticText{"Float"}
                                .withSize({kFieldW, kLabelH}),
                            SpinBox { { .min = 0.0f, .max = 10.0f, .step = 0.1f }, spinFloat }
                                .withSize({kFieldW, kRowH})
                                .withTooltip("0.0 to 10.0, stepping by 0.1.")
                        }
                    },
                    VGroupBox { "Date & Time",
                        LayoutFlags().MinSize({kBoxW, 90}).Border(Side::Top, 8),
                        HStack {
                            StaticText{"Date"}
                                .withSize({kLabelW, kLabelH})
                                .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8)),
                            DatePicker{date}
                                .withSize({kPickerW, kRowH})
                                .withTooltip("Composite control -- the tooltip covers all three fields.")
                        },
                        HStack {
                            StaticText{"Time"}
                                .withSize({kLabelW, kLabelH})
                                .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8)),
                            TimePicker{time}
                                .withSize({kPickerW, kRowH})
                        }
                    }
                },
                // Right column
                VStack {
                    LayoutFlags().Border(Side::Left, 12),
                    VGroupBox { "Preview",
                        LayoutFlags().MinSize({kBoxW, 200}),
                        // Fit: the 960x959 cat letterboxes into the 300x160
                        // band rather than being squashed into it.
                        Image{"images/Cat03.jpg"}
                            .withSize({300, 160})
                            .withScaleMode(ScaleMode::Fit)
                            .withFlags(LayoutFlags().CenterHorizontal())
                            .onClick([]() {})
                            .onHover([]() {})
                            .withTooltip("Cat03.jpg -- click to open full size.")
                    },
                    VGroupBox { "Progress",
                        LayoutFlags().MinSize({kBoxW, 90}).Border(Side::Top, 8),
                        HStack {
                            StaticText{"Loading:"}
                                .withSize({kLabelW, kLabelH})
                                .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8)),
                            ProgressBar{progress}
                                .withSize({-1, 18})
                                .withFlags(LayoutFlags().Proportion(1).CenterVertical())
                                .withTooltip("Driven by the slider in the binding demos.")
                        },
                        HStack {
                            StaticText{"Fixed 60%:"}
                                .withSize({kLabelW, kLabelH})
                                .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8)),
                            ProgressBar{0.6f}
                                .withSize({-1, 18})
                                .withFlags(LayoutFlags().Proportion(1).CenterVertical())
                        }
                    },
                    TabPanel {
                        LayoutFlags().MinSize({kBoxW, 170}).Border(Side::Top, 8),
                        Tab { "Notes",
                            VStack {
                                LayoutFlags().Expand().Border(Side::All, 5),
                                MultiLineTextCtrl{tabNote}
                                    .withSize({-1, 90})
                                    .withFlags(LayoutFlags().Expand())
                            }
                        },
                        Tab { "Settings",
                            VStack {
                                LayoutFlags().Border(Side::All, 5),
                                CheckBox{tabLogging, "Enable logging"}
                                    .withSize({-1, kRowH})
                                    .withTooltip("Writes a verbose trace to disk."),
                                Separator{}.withFlags(LayoutFlags().Expand()),
                                HStack {
                                    StaticText{"Theme color:"}
                                        .withSize({kLabelW, kLabelH})
                                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8)),
                                    ColorPicker{themeColor}
                                        .withSize({-1, kRowH})
                                        .withFlags(LayoutFlags().Proportion(1))
                                        .withTooltip("Accent color applied across the app.")
                                }
                            }
                        },
                        Tab { "About",
                            VStack {
                                LayoutFlags().Border(Side::All, 5),
                                StaticText{"DeclarativeUI-CPP"}
                                    .withSize({-1, kLabelH}),
                                StaticText{"A backend-agnostic declarative UI framework"}
                                    .withSize({-1, kLabelH})
                            }
                        }
                    }
                }
            },
            HStack {
                LayoutFlags().Border(Side::Top, 8),
                Spacer{},
                Button{"Check"}
                    .withSize(kButtonSize)
                    .withFlags(LayoutFlags().CenterVertical())
                    .onClick(std::move(onCheckClick))
                    .withTooltip("Validates every field in this dialog."),
                Button{"MessageBox"}
                    .withSize(kButtonSize)
                    .withFlags(LayoutFlags().Border(Side::Left, 8).CenterVertical())
                    .onClick(std::move(onMessageBoxClick)),
                ToggleButton{toggle, "Toggle me!"}
                    .withSize(kButtonSize)
                    .withFlags(LayoutFlags().Border(Side::Left, 8).CenterVertical())
                    .onChange([](bool newValue) {

                })
            }
        }
    };
}

// Scale modes and text alignment: a dialog of its own, next to the gallery
// above rather than inside it. The gallery already fills the demo's viewport
// on ImGui, and four pictures plus an alignment ruler is more than the ~20
// element mark rule 4 puts on one dialog.
//
// Every picture is the SAME file in the SAME 180x100 frame, so the only thing
// that differs between the four is the mode. Cat03.jpg is 960x959 -- near
// enough square that a non-square frame is what makes the four read apart:
//
//   Fit      100x100 centred, transparent bars left and right
//   Fill     180x180 centred, cropped top and bottom
//   Stretch  180x100 exactly, squashed -- the framework's long-standing default
//   Center   960x959 centred and cropped hard, so only the middle survives
//
// The alignment box is three labels in one band. The band is what matters:
// each label carries Expand(), so it stretches across the whole box and the
// slack inside its frame is what withAlign() moves the text around in. Without
// that a leaf sits at its desired width and all three would look identical.
//
// `displaysDisabled` binds the whole dialog's leaves to one caller-owned flag,
// which is the value-binding shape a read-only widget can offer: neither a
// picture nor a label has a value to share, so isDisabled() is the binding.
inline auto drawScaleAndAlignUI(bool& displaysDisabled)
{
    constexpr int kBoxW = 420;
    constexpr Size kPicture { 180, 100 };
    constexpr int kCaptionH = 20;
    constexpr int kLabelH = 22;

    auto picture = [&](const char* caption, ScaleMode mode) {
        return VStack {
            LayoutFlags().Border(Side::Right, 10),
            StaticText{caption}
                .withAlign(TextAlign::Center)
                .withSize({kPicture.width, kCaptionH})
                .withFlags(LayoutFlags().Expand()),
            Image{"images/Cat03.jpg"}
                .withSize(kPicture)
                .withScaleMode(mode)
                .withFlags(LayoutFlags().Border(Side::Top, 4))
                .isDisabled(displaysDisabled)
                .withTooltip("The same 960x959 file in the same 180x100 frame.")
        };
    };

    return Dialog {
        "Scale modes & text alignment",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 12),
            VGroupBox { "Image::withScaleMode -- one file, one frame, four modes",
                LayoutFlags().Expand(),
                HStack {
                    LayoutFlags().Border(Side::All, 6),
                    picture("Fit", ScaleMode::Fit),
                    picture("Fill", ScaleMode::Fill),
                    picture("Stretch", ScaleMode::Stretch),
                    picture("Center", ScaleMode::Center)
                }
            },
            VGroupBox { "StaticText::withAlign -- one band, three alignments",
                LayoutFlags().Expand().MinSize({kBoxW, -1}).Border(Side::Top, 10),
                StaticText{"Left -- the default, and where every label starts"}
                    .withAlign(TextAlign::Left)
                    .withSize({-1, kLabelH})
                    .withFlags(LayoutFlags().Expand().Border(Side::All, 6))
                    .isDisabled(displaysDisabled),
                Separator{}
                    .withSize({-1, 1})
                    .withFlags(LayoutFlags().Expand()),
                StaticText{"Centre"}
                    .withAlign(TextAlign::Center)
                    .withSize({-1, kLabelH})
                    .withFlags(LayoutFlags().Expand().Border(Side::All, 6))
                    .isDisabled(displaysDisabled),
                Separator{}
                    .withSize({-1, 1})
                    .withFlags(LayoutFlags().Expand()),
                StaticText{"Right"}
                    .withAlign(TextAlign::Right)
                    .withSize({-1, kLabelH})
                    .withFlags(LayoutFlags().Expand().Border(Side::All, 6))
                    .isDisabled(displaysDisabled)
            },
            CheckBox{displaysDisabled, "Disable the pictures and the labels"}
                .withSize({-1, 26})
                .withFlags(LayoutFlags().Border(Side::Top, 10))
        }
    };
}
