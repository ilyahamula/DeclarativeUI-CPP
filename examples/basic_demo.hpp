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
    std::function<void()> onCheckClick = []() {})
{
    return Dialog {
        "Controls Demo",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 10),
            VGroupBox { "Text Input",
                MultiLineTextCtrl{multilineText}
                    .withFlags(LayoutFlags().Expand()),
                HStack {
                    StaticText{"Password:"}
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 5)),
                    PasswordInput{password}
                        .withFlags(LayoutFlags().Proportion(1).Expand())
                },
                LinkText{"Visit documentation"}
                    .withFlags(LayoutFlags().Border(Side::Top, 5))
                    .onClick([]() {})
            },
            HGroupBox { "Numeric Values",
                LayoutFlags().Border(Side::All, 5),
                VStack {
                    StaticText{"Integer"},
                    SpinBox { { .min = 0, .max = 100 }, spinInt }
                },
                VStack {
                    StaticText{"Float"},
                    SpinBox { { .min = 0.0f, .max = 10.0f, .step = 0.1f }, spinFloat }
                }
            },
            HGroupBox { "Date & Time",
                LayoutFlags().Border(Side::All, 5),
                VStack {
                    StaticText{"Date"},
                    DatePicker{date}
                },
                VStack {
                    StaticText{"Time"},
                    TimePicker{time}
                }
            },
            HStack {
                StaticText{""}
                    .withFlags(LayoutFlags().Proportion(1)),
                Button{"Check"}
                    .withFlags(LayoutFlags().CenterVertical())
                    .onClick(std::move(onCheckClick)),
                ToggleButton{toggle, "Toggle me!"}
                    .withFlags(LayoutFlags().Border(Side::Top, 5))
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
