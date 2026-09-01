#pragma once

// Container-level disabling: isDisabled() on a stack, a group box, a tab panel
// or a single tab, instead of on each widget inside it.
//
// The rule is the one every backend already uses natively: a container's
// disabled state is OR'd into everything below it, so a child can never opt
// back out. `.isDisabled(false)` on a control inside a disabled group is
// therefore a no-op -- the dialogs below show that explicitly.
//
// Bound to a caller-owned bool, the whole subtree follows the flag live: ImGui
// re-reads it every frame, wx/Qt poll it on idle exactly like a shared value.

#include "declarative_ui.hpp"

#include <string>

// One group box owns the disabled state of the five controls inside it -- none
// of them carries isDisabled() itself. The nested box demonstrates the
// cascade: it never sets the flag, it just inherits it.
inline auto drawGroupBoxDisabling(std::string& name,
    std::string& email,
    int& copies,
    bool& express,
    bool& disabled)
{
    constexpr int kLabelW = 70;
    constexpr int kRowH = 26;
    constexpr int kFieldW = 170;

    return Dialog {
        "Disable a whole group box",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 12),
            VGroupBox { "Delivery details",
                LayoutFlags().Expand().MinSize({320, -1}),
                HStack {
                    StaticText{"Name:"}
                        .withSize({kLabelW, 20})
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 6)),
                    TextCtrl{name}
                        .withSize({kFieldW, kRowH})
                        .withFlags(LayoutFlags().Proportion(1))
                },
                HStack {
                    LayoutFlags().Border(Side::Top, 6),
                    StaticText{"Email:"}
                        .withSize({kLabelW, 20})
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 6)),
                    TextCtrl{email}
                        .withSize({kFieldW, kRowH})
                        .withFlags(LayoutFlags().Proportion(1))
                },
                // inherits the outer box's state without naming the flag
                HGroupBox { "Options",
                    LayoutFlags().Expand().Border(Side::Top, 8),
                    SpinBox { { .min = 1, .max = 99 }, copies }
                        .withSize({80, kRowH}),
                    CheckBox{express, "Express"}
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Left, 10)),
                    // an explicit false cannot escape a disabled ancestor
                    Button{"Send"}
                        .withSize({80, 28})
                        .withFlags(LayoutFlags().Border(Side::Left, 10))
                        .isDisabled(false)
                }
            }
            .isDisabled(disabled),
            // outside the box, so it stays clickable and can switch it back on
            CheckBox{disabled, "Disable the whole group"}
                .withFlags(LayoutFlags().Border(Side::Top, 12))
        }
    };
}

// Stacks carry the same modifier. A TabPanel can disable one page -- its content
// greys out while the tab stays selectable, the only behavior wx, Qt and ImGui
// share -- or the whole panel, which locks tab switching on all three.
inline auto drawStackAndTabDisabling(std::string& note,
    int& level,
    bool& logging,
    bool& pageDisabled,
    bool& panelDisabled)
{
    return Dialog {
        "Disable a stack / a tab",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 12),
            TabPanel {
                LayoutFlags().Expand().MinSize({320, 190}),
                Tab { "Always on",
                    VStack {
                        LayoutFlags().Expand().Border(Side::All, 8),
                        StaticText{"This page is never disabled."}
                            .withSize({-1, 20}),
                        // a row disabled on its own, inside an enabled page
                        HStack {
                            LayoutFlags().Border(Side::Top, 10),
                            StaticText{"Level:"}
                                .withSize({50, 20})
                                .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 6)),
                            Slider { Range<int>{ .min = 0, .max = 10 }, level }
                                .withSize({160, 24})
                                .withFlags(LayoutFlags().Proportion(1))
                        }
                        .isDisabled(pageDisabled)
                    }
                },
                Tab { "Advanced",
                    VStack {
                        LayoutFlags().Expand().Border(Side::All, 8),
                        MultiLineTextCtrl{note}
                            .withSize({-1, 80})
                            .withFlags(LayoutFlags().Expand()),
                        CheckBox{logging, "Verbose logging"}
                            .withFlags(LayoutFlags().Border(Side::Top, 8))
                    }
                }
                .isDisabled(pageDisabled)
            }
            .isDisabled(panelDisabled),
            CheckBox{pageDisabled, "Disable the 'Advanced' page and the Level row"}
                .withFlags(LayoutFlags().Border(Side::Top, 12)),
            CheckBox{panelDisabled, "Disable the whole tab panel"}
                .withFlags(LayoutFlags().Border(Side::Top, 6))
        }
    };
}
