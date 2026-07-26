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

inline auto drawStaticText()
{
    return Dialog {
        "Simple Dialog",
        VStack {
            LayoutFlags().Expand().Border(Side::Left, 40).Border(Side::Right, 100),
            StaticText{"This is a simple dialog."}
                .withFlags(LayoutFlags().CenterHorizontal().CenterVertical())
        }
    };
}

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

// ---------------------------------------------------------------------------
// Layout probes
//
// Small, state-free dialogs that each isolate one engine behaviour. They are
// Resizable() on purpose: the interesting part is what happens to the extra
// space once the window grows past the auto-fit size. Call them directly,
// e.g. `drawFixedLeftStretchRight().show();`
// ---------------------------------------------------------------------------

// Two columns in a row: left keeps its 200px, right eats all the slack.
inline auto drawFixedLeftStretchRight()
{
    return Dialog {
        "Split: fixed left / stretch right",
        HStack {
            LayoutFlags().Expand().Border(Side::All, 10),
            VGroupBox { "Fixed 200",
                LayoutFlags().MinSize({200, 220}).Expand(),
                StaticText{"Width never changes"},
                StaticText{"Proportion: none"},
                Button{"Left"}.withSize({120, 28})
            },
            VGroupBox { "Stretch",
                LayoutFlags().Proportion(1).Expand().Border(Side::Left, 10),
                StaticText{"Takes all remaining width"},
                StaticText{"Proportion: 1"},
                ProgressBar{0.5f}
                    .withSize({-1, 18})
                    .withFlags(LayoutFlags().Expand())
            }
        }
    }.Resizable();
}

// The mirror image: the slack goes to the left column instead.
inline auto drawStretchLeftFixedRight()
{
    return Dialog {
        "Split: stretch left / fixed right",
        HStack {
            LayoutFlags().Expand().Border(Side::All, 10),
            VGroupBox { "Stretch",
                LayoutFlags().Proportion(1).Expand(),
                StaticText{"Takes all remaining width"},
                StaticText{"Proportion: 1"},
                ProgressBar{0.5f}
                    .withSize({-1, 18})
                    .withFlags(LayoutFlags().Expand())
            },
            VGroupBox { "Fixed 200",
                LayoutFlags().MinSize({200, 220}).Expand().Border(Side::Left, 10),
                StaticText{"Width never changes"},
                StaticText{"Proportion: none"},
                Button{"Right"}.withSize({120, 28})
            }
        }
    }.Resizable();
}

// Three columns sharing the slack 1:2:1 — the middle one grows twice as fast.
inline auto drawProportionalColumns()
{
    return Dialog {
        "Proportions 1 : 2 : 1",
        HStack {
            LayoutFlags().Expand().Border(Side::All, 10),
            VGroupBox { "Weight 1",
                LayoutFlags().Proportion(1).Expand().MinSize({120, 180}),
                StaticText{"1x"}
            },
            VGroupBox { "Weight 2",
                LayoutFlags().Proportion(2).Expand().MinSize({120, 180}).Border(Side::Left, 8),
                StaticText{"2x"}
            },
            VGroupBox { "Weight 1",
                LayoutFlags().Proportion(1).Expand().MinSize({120, 180}).Border(Side::Left, 8),
                StaticText{"1x"}
            }
        }
    }.Resizable();
}

// Vertical counterpart: fixed header, stretching body, fixed footer.
inline auto drawHeaderBodyFooter()
{
    return Dialog {
        "Fixed header / stretch body / fixed footer",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 10),
            HGroupBox { "Header (fixed 60)",
                LayoutFlags().MinSize({360, 60}).Expand(),
                StaticText{"Height never changes"}
                    .withFlags(LayoutFlags().CenterVertical())
            },
            VGroupBox { "Body (stretch)",
                LayoutFlags().Proportion(1).Expand().Border(Side::Top, 8),
                StaticText{"Absorbs every extra pixel of height"},
                ProgressBar{0.3f}
                    .withSize({-1, 18})
                    .withFlags(LayoutFlags().Expand())
            },
            HStack {
                LayoutFlags().Border(Side::Top, 8),
                StaticText{""}
                    .withFlags(LayoutFlags().Proportion(1)),
                Button{"Cancel"}.withSize({100, 28}),
                Button{"OK"}
                    .withSize({100, 28})
                    .withFlags(LayoutFlags().Border(Side::Left, 8))
            }
        }
    }.Resizable();
}

// Nested split: a stretching column that itself splits into a fixed top row
// and a stretching bottom row. Checks that proportion composes through levels.
inline auto drawNestedSplit()
{
    return Dialog {
        "Nested split",
        HStack {
            LayoutFlags().Expand().Border(Side::All, 10),
            VGroupBox { "Sidebar (fixed 160)",
                LayoutFlags().MinSize({160, 260}).Expand(),
                Button{"Item 1"}.withSize({-1, 26}).withFlags(LayoutFlags().Expand()),
                Button{"Item 2"}.withSize({-1, 26}).withFlags(LayoutFlags().Expand().Border(Side::Top, 4)),
                Button{"Item 3"}.withSize({-1, 26}).withFlags(LayoutFlags().Expand().Border(Side::Top, 4))
            },
            VStack {
                LayoutFlags().Proportion(1).Expand().Border(Side::Left, 10),
                HGroupBox { "Toolbar (fixed)",
                    LayoutFlags().MinSize({240, 56}).Expand(),
                    Button{"A"}.withSize({48, 26}),
                    Button{"B"}.withSize({48, 26}).withFlags(LayoutFlags().Border(Side::Left, 6)),
                    StaticText{""}.withFlags(LayoutFlags().Proportion(1))
                },
                VGroupBox { "Canvas (stretch both ways)",
                    LayoutFlags().Proportion(1).Expand().Border(Side::Top, 8),
                    StaticText{"Grows horizontally and vertically"}
                }
            }
        }
    }.Resizable();
}

// Same content twice: raw labels (ragged fields) vs. SizeGroup labels
// (fields line up because every label is widened to the group's widest).
inline auto drawSizeGroupComparison(std::string& a, std::string& b, std::string& c)
{
    constexpr int kRowH = 26;
    return Dialog {
        "SizeGroup vs. natural label widths",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 10),
            VGroupBox { "Ragged (no SizeGroup)",
                LayoutFlags().MinSize({360, 100}).Expand(),
                HStack {
                    StaticText{"Name:"}.withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 6)),
                    TextCtrl{a}.withSize({-1, kRowH}).withFlags(LayoutFlags().Proportion(1))
                },
                HStack {
                    LayoutFlags().Border(Side::Top, 4),
                    StaticText{"Description:"}.withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 6)),
                    TextCtrl{b}.withSize({-1, kRowH}).withFlags(LayoutFlags().Proportion(1))
                }
            },
            VGroupBox { "Aligned (SizeGroup 1)",
                LayoutFlags().MinSize({360, 100}).Expand().Border(Side::Top, 8),
                HStack {
                    StaticText{"Name:"}
                        .withFlags(LayoutFlags().SizeGroup(1).CenterVertical().Border(Side::Right, 6)),
                    TextCtrl{a}.withSize({-1, kRowH}).withFlags(LayoutFlags().Proportion(1))
                },
                HStack {
                    LayoutFlags().Border(Side::Top, 4),
                    StaticText{"Description:"}
                        .withFlags(LayoutFlags().SizeGroup(1).CenterVertical().Border(Side::Right, 6)),
                    TextCtrl{c}.withSize({-1, kRowH}).withFlags(LayoutFlags().Proportion(1))
                }
            }
        }
    };
}

// MaxSize caps a stretching child: past ~400px the field stops growing and
// the leftover space stays empty inside the row.
inline auto drawMinMaxClamp(std::string& text)
{
    return Dialog {
        "MinSize / MaxSize clamping",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 10),
            VGroupBox { "Uncapped",
                LayoutFlags().MinSize({420, 80}).Expand(),
                StaticText{"Proportion(1), no MaxSize — grows without limit"},
                TextCtrl{text}
                    .withSize({-1, 26})
                    .withFlags(LayoutFlags().Proportion(1).Expand())
            },
            VGroupBox { "Capped at 400",
                LayoutFlags().MinSize({420, 80}).Expand().Border(Side::Top, 8),
                StaticText{"Proportion(1) + MaxSize({400, -1})"},
                TextCtrl{text}
                    .withSize({-1, 26})
                    .withFlags(LayoutFlags().Proportion(1).Expand().MaxSize({400, -1}))
            },
            VGroupBox { "Floored at 250",
                LayoutFlags().MinSize({420, 80}).Expand().Border(Side::Top, 8),
                StaticText{"MinSize({250, -1}), no proportion — never shrinks below 250"},
                TextCtrl{text}
                    .withSize({-1, 26})
                    .withFlags(LayoutFlags().MinSize({250, -1}))
            }
        }
    }.Resizable();
}

// Cross-axis alignment side by side inside a stretching row.
inline auto drawAlignmentProbe()
{
    return Dialog {
        "Cross-axis alignment",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 10),
            HGroupBox { "Row: vertical alignment of a short button",
                LayoutFlags().MinSize({420, 120}).Expand(),
                Button{"Top (default)"}.withSize({130, 28}),
                Button{"Center"}
                    .withSize({130, 28})
                    .withFlags(LayoutFlags().CenterVertical().Border(Side::Left, 8)),
                Button{"Expand"}
                    .withSize({130, -1})
                    .withFlags(LayoutFlags().Expand().Border(Side::Left, 8))
            },
            VGroupBox { "Column: horizontal alignment of a narrow button",
                LayoutFlags().MinSize({420, 140}).Expand().Border(Side::Top, 8),
                Button{"Left (default)"}.withSize({130, 28}),
                Button{"Center"}
                    .withSize({130, 28})
                    .withFlags(LayoutFlags().CenterHorizontal().Border(Side::Top, 6)),
                Button{"Expand"}
                    .withSize({-1, 28})
                    .withFlags(LayoutFlags().Expand().Border(Side::Top, 6))
            }
        }
    }.Resizable();
}

