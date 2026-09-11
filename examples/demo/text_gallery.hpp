#pragma once

// Text display: the gallery dialog for ReadonlyTextCtrl.
//
// controls_gallery.hpp is long past the ~20-element mark rule 4 sets, so
// ReadonlyTextCtrl lands here instead, alongside the other text widgets for
// contrast (StaticText, TextCtrl, ClickableText, LinkText).
//
// The three read-only spellings are the point of the dialog:
//
//   StaticText        a label -- no frame, no selection, no caret
//   ReadonlyTextCtrl  a real field the user can focus and copy out of, but
//                     never type into
//   TextCtrl          editable, for comparison
//
// ReadonlyTextCtrl takes its text by value: the first one below is built from
// a string LITERAL, which is the case that used to dangle -- the widget is a
// temporary that dies with this expression, so a reference into its argument
// was gone by the time the backend realized the control.
//
// Snapshot, not binding: unlike TextCtrl, ReadonlyTextCtrl has no bound form
// today, so it shows the text as it stood when the tree was built. Rebuild the
// tree (or use a TextCtrl) to show a value that changes.

#include "declarative_ui.hpp"

#include <string>

inline auto drawTextUI(
    std::string& editableText,
    bool& fieldsDisabled)
{
    // Same pinning discipline as the other galleries: explicit sizes on the
    // leaves and MinSize on the group boxes, so the three backends compute the
    // same frames despite their different native metrics.
    constexpr int kBoxW = 380;
    constexpr int kRowH = 26;
    constexpr int kLabelH = 20;
    constexpr int kLabelW = 90;
    constexpr int kFieldW = 260;  // pinned so all three backends measure alike
    constexpr Size kButtonSize { 110, 28 };

    // A caller-owned const string, to show the other way a read-only field is
    // usually fed: a value computed once and displayed.
    static const std::string buildInfo = "DeclarativeUI-CPP 0.1.0";

    return Dialog {
        "Text Controls",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 10),
            VGroupBox { "Read-only text",
                LayoutFlags().MinSize({kBoxW, 150}),
                HStack {
                    StaticText{"Label:"}
                        .withSize({kLabelW, kLabelH})
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8)),
                    StaticText{"Plain StaticText -- not selectable"}
                        .withSize({kFieldW, kLabelH})
                        .withFlags(LayoutFlags().Proportion(1).CenterVertical())
                },
                HStack {
                    LayoutFlags().Border(Side::Top, 6),
                    StaticText{"From literal:"}
                        .withSize({kLabelW, kLabelH})
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8)),
                    // The dangling case: the text is owned, so the literal it
                    // was built from need not outlive this expression.
                    ReadonlyTextCtrl{"Select and copy me."}
                        .withSize({kFieldW, kRowH})
                        .withFlags(LayoutFlags().Proportion(1))
                        .withTooltip("Read-only: focusable and copyable, never editable.")
                        .isDisabled(fieldsDisabled)
                },
                HStack {
                    LayoutFlags().Border(Side::Top, 6),
                    StaticText{"From value:"}
                        .withSize({kLabelW, kLabelH})
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8)),
                    ReadonlyTextCtrl{buildInfo}
                        .withSize({kFieldW, kRowH})
                        .withFlags(LayoutFlags().Proportion(1))
                        .isDisabled(fieldsDisabled)
                }
            },
            VGroupBox { "Editable, for contrast",
                LayoutFlags().MinSize({kBoxW, 90}).Border(Side::Top, 8),
                HStack {
                    StaticText{"Editable:"}
                        .withSize({kLabelW, kLabelH})
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8)),
                    TextCtrl{editableText}
                        .withSize({kFieldW, kRowH})
                        .withFlags(LayoutFlags().Proportion(1))
                        .withTooltip("Bound to the caller's string -- edits write through.")
                        .isDisabled(fieldsDisabled)
                },
                HStack {
                    LayoutFlags().Border(Side::Top, 6),
                    ClickableText{"Clickable text"}
                        .withSize({130, kLabelH})
                        .withFlags(LayoutFlags().CenterVertical())
                        .onClick([]() {}),
                    LinkText{"Visit documentation"}
                        .withSize({160, kLabelH})
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Left, 16))
                        .onClick([]() {})
                }
            },
            HStack {
                LayoutFlags().Border(Side::Top, 8),
                CheckBox{fieldsDisabled, "Disable the fields"}
                    .withSize({-1, kRowH})
                    .withFlags(LayoutFlags().CenterVertical()),
                Spacer{},
                Button{"Close"}
                    .withSize(kButtonSize)
                    .withFlags(LayoutFlags().CenterVertical())
                    .onClick([]() {})
            }
        }
    };
}
