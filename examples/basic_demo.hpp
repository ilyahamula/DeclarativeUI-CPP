#pragma once
#include "declarative_ui.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

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
                            .withFlags(LayoutFlags().Expand()),
                        HStack {
                            StaticText{"Password:"}
                                .withSize({kLabelW, kLabelH})
                                .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 5)),
                            PasswordInput{password}
                                .withSize({-1, kRowH})
                                .withFlags(LayoutFlags().Proportion(1))
                        },
                        LinkText{"Visit documentation"}
                            .withSize({-1, kLabelH})
                            .withFlags(LayoutFlags().Border(Side::Top, 5))
                            .onClick([]() {})
                    },
                    HGroupBox { "Numeric Values",
                        LayoutFlags().MinSize({kBoxW, 90}).Border(Side::Top, 8),
                        VStack {
                            StaticText{"Integer"}
                                .withSize({kFieldW, kLabelH}),
                            SpinBox { { .min = 0, .max = 100 }, spinInt }
                                .withSize({kFieldW, kRowH})
                        },
                        VStack {
                            StaticText{"Float"}
                                .withSize({kFieldW, kLabelH}),
                            SpinBox { { .min = 0.0f, .max = 10.0f, .step = 0.1f }, spinFloat }
                                .withSize({kFieldW, kRowH})
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
                        Image{"images/Cat03.jpg"}
                            .withSize({300, 160})
                            .withFlags(LayoutFlags().CenterHorizontal())
                            .onClick([]() {})
                            .onHover([]() {})
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
                                    .withSize({-1, kRowH}),
                                Separator{}.withFlags(LayoutFlags().Expand()),
                                HStack {
                                    StaticText{"Theme color:"}
                                        .withSize({kLabelW, kLabelH})
                                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8)),
                                    ColorPicker{themeColor}
                                        .withSize({-1, kRowH})
                                        .withFlags(LayoutFlags().Proportion(1))
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
                StaticText{""}
                    .withFlags(LayoutFlags().Proportion(1)),
                Button{"Check"}
                    .withSize(kButtonSize)
                    .withFlags(LayoutFlags().CenterVertical())
                    .onClick(std::move(onCheckClick)),
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

inline auto drawUI(std::string& text,
    float& sliderValue,
    int& sliderValue1,
    int& radioChoice,
    int& radioChoice1,
    std::string& selectedCombo,
    bool& checked,
    std::function<void()> onButtonClick = []() {})
{
    return Dialog {
        "Declarative UI Dialog",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 10),
            HStack {
                Button{"Browse..."}
                    .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 15))
                    .onClick(std::move(onButtonClick)),
                TextCtrl{text}
                    .withFlags(LayoutFlags().Proportion(1).Expand())
            },
            HGroupBox {
                Slider { { .min = 0.0f, .max = 65.0f }, sliderValue }
                    .withFlags(LayoutFlags().Proportion(1).Expand().CenterVertical()),
                VGroupBox {
                    LayoutFlags().CenterVertical().Border(Side::Left),
                    RadioButton{radioChoice, "On"},
                    RadioButton{radioChoice, "Off"}
                }
            },
            HStack {
                StaticText{"Ready"}
                    .withFlags(LayoutFlags().Proportion(1).CenterVertical()),
                Button{"OK"}
                    .withFlags(LayoutFlags().CenterVertical())
            },
            HGroupBox { "Combo Group",
                ComboBox{ {"Hello", "Goodbye", "Nihao" }, selectedCombo }
                    .withFlags(LayoutFlags().Proportion(1).CenterVertical().Border(Side::Right)),
                CheckBox{checked, "Check me!"}
                    .withFlags(LayoutFlags().CenterVertical())
            },
            HGroupBox {
                Slider { { .min = 0, .max = 200 }, sliderValue1 }
                    .withFlags(LayoutFlags().Proportion(1).Expand().CenterVertical())
                    .onChange([](int newValue) {

                    }),
                VGroupBox {
                    LayoutFlags().CenterVertical().Border(Side::Left),
                    RadioButton{radioChoice1, "On"},
                    RadioButton{radioChoice1, "Off"}
                }
            }
        }
    };
}
