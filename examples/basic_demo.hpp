#pragma once
#include "declarative_ui.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#include <functional>
#include <string>

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
                    .withFlags(LayoutFlags().Proportion(1).Expand().CenterVertical()),
                VGroupBox {
                    LayoutFlags().CenterVertical().Border(Side::Left),
                    RadioButton{radioChoice1, "On"},
                    RadioButton{radioChoice1, "Off"}
                }
            }
        }
    };
}
