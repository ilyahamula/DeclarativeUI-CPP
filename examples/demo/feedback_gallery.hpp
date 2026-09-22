#pragma once

// Feedback: the gallery for a Dialog's LIFECYCLE -- Modal(), onClose() and
// show(bool&) -- as an OK/Cancel form, which is the shape those three exist for.
//
// A confirmation is the smallest thing you cannot build without all three of
// them. The parent asks; a modal box takes the answer and nothing else in the
// application can be touched meanwhile; the answer arrives through a shared
// value rather than a return code, because show() does NOT block on any
// backend; and onClose() is how the parent learns the box has gone whichever
// way it went.
//
// Four things are worth watching:
//
//   * Modal() is NON-BLOCKING everywhere. show() returns immediately on wx
//     (wxWindowDisabler), Qt (Qt::ApplicationModal + show(), never exec()) and
//     ImGui (BeginPopupModal instead of Begin) -- so there is no result to
//     return, and the two buttons write a caller-owned string instead. Watch
//     the parent's "Last answer" field fill in while the box is still up on
//     screen: that write happened before the dialog closed, which a blocking
//     dialog could not have shown you.
//   * The OPEN FLAG is the single truth. Neither button closes the dialog;
//     both clear `confirmOpen` and the dialog follows, exactly as the title
//     bar's close button clears it from the other side.
//   * onClose() fires EXACTLY ONCE per close, and the report counts them, so
//     "once" is something you can read rather than take on trust. Open and
//     close the box three times and it says three, never six -- both close
//     paths run through one place in the session.
//   * OPENING is the one half of the lifecycle that is not symmetric across
//     the backends, which is why the parent takes `openConfirm` as a callback
//     instead of just setting the flag. On ImGui show() IS the frame, so the
//     flag alone is enough and the main's frame loop does the rest. wx and Qt
//     destroy the native dialog when it closes, so there a second show() call
//     is what brings one back -- one show() per open on a retained backend,
//     one show() per frame on an immediate one. Each main supplies the version
//     its backend needs; everything below is written once.
//
// controls_gallery.hpp is long past the ~20-element mark rule 4 sets, so this
// lands as its own dialog alongside a few already-implemented controls for
// context (ListBox, StaticText, TextCtrl, Separator, CheckBox, Button).

#include "declarative_ui.hpp"

#include <functional>
#include <string>
#include <utility>

// The modal half. Built fresh on every open -- it is a temporary that dies with
// the show() expression, so every ref below belongs to the caller and outlives
// it, as it does for every other dialog in the demos.
inline auto drawConfirmDeleteUI(bool& open, std::string& file, std::string& answer,
    std::string& closeReport, int& closeCount)
{
    constexpr int kRowH = 28;
    constexpr int kLabelH = 20;
    constexpr int kLabelW = 48;
    constexpr Size kButtonSize { 110, 28 };

    return Dialog {
        "Delete file?",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 14).MinSize({380, -1}),
            StaticText{"This cannot be undone. Nothing else in the application"}
                .withSize({-1, kLabelH}),
            StaticText{"accepts input until you answer -- that is Modal()."}
                .withSize({-1, kLabelH}),

            HStack {
                LayoutFlags().Expand().Border(Side::Top, 12),
                StaticText{"File:"}
                    .withSize({kLabelW, kLabelH})
                    .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8)),
                // Bound, not baked into the message above. The parent's list
                // writes this string, so the name is current on all three
                // backends -- not only on the one that rebuilds the tree every
                // frame and would have read it live anyway.
                TextCtrl{file}
                    .withSize({-1, kRowH})
                    .withFlags(LayoutFlags().Expand().CenterVertical())
            },

            Separator{}
                .withSize({-1, 1})
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 12)),

            HStack {
                LayoutFlags().Expand().Border(Side::Top, 12),
                Spacer{},
                // Neither button closes the dialog. Both write the answer and
                // clear the flag, and the flag is what the dialog follows --
                // which is the same path the title bar's close button takes
                // from the other side.
                Button{"Cancel"}
                    .withSize(kButtonSize)
                    .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8))
                    .onClick([&open, &answer]() {
                        answer = "Cancel -- nothing was deleted.";
                        open = false;
                    }),
                Button{"Delete"}
                    .withSize(kButtonSize)
                    .withFlags(LayoutFlags().CenterVertical())
                    .onClick([&open, &answer, &file]() {
                        answer = "Deleted " + file + ".";
                        open = false;
                    })
            }
        }
    }
    .Modal()
    // Fires once however the box went away -- either button, the title bar, or
    // the check box in the binding panel clearing the flag. The count is what
    // makes "exactly once" readable: three opens and closes say three.
    .onClose([&closeReport, &closeCount]() {
        closeReport = "onClose() fired " + std::to_string(++closeCount)
            + "x -- once per close, never twice.";
    });
}

// The parent. `openConfirm` is the one thing each main has to supply: see the
// fourth note at the top of this file.
inline auto drawDeleteFormUI(std::string& file, std::string& answer,
    std::string& closeReport, bool& formDisabled, std::function<void()> openConfirm)
{
    constexpr int kRowH = 28;
    constexpr int kLabelH = 20;
    constexpr int kListW = 220;
    constexpr Size kButtonSize { 150, 28 };

    return Dialog {
        "Confirm before deleting",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 14).MinSize({460, -1}),
            StaticText{"Pick a file, then ask. The box that opens is modal:"}
                .withSize({-1, kLabelH}),
            StaticText{"try clicking this window while it is up."}
                .withSize({-1, kLabelH}),

            HStack {
                LayoutFlags().Expand().Border(Side::Top, 10),
                ListBox<std::string> { {"main.cpp", "layout.cpp", "widgets.hpp", "engine.hpp"}, file }
                    .withVisibleRows(4)
                    .withSize({kListW, -1})
                    .isDisabled(formDisabled),
                Spacer{Size{12, 0}},
                VStack {
                    LayoutFlags().Expand().Proportion(1),
                    Spacer{},
                    Button{"Delete file..."}
                        .withSize(kButtonSize)
                        .withTooltip("Opens an application-modal confirmation")
                        .isDisabled(formDisabled)
                        // Just asks. Setting the flag is enough on ImGui; on wx
                        // and Qt the main's version of this also calls show().
                        .onClick([openConfirm]() { openConfirm(); }),
                    Spacer{}
                }
            },

            Separator{}
                .withSize({-1, 1})
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 12)),

            StaticText{"Last answer (written by the modal, while it was still up):"}
                .withSize({-1, kLabelH})
                .withFlags(LayoutFlags().Border(Side::Top, 10)),
            // Bound, not readonly: an external write only reaches a control
            // through a bound ref, and this one is written from a button in
            // another window. ReadonlyTextCtrl takes a snapshot and would never
            // see it.
            TextCtrl{answer}
                .withSize({-1, kRowH})
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 4)),

            StaticText{"What the modal's onClose() last reported:"}
                .withSize({-1, kLabelH})
                .withFlags(LayoutFlags().Border(Side::Top, 10)),
            TextCtrl{closeReport}
                .withSize({-1, kRowH})
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 4)),

            CheckBox{formDisabled, "Lock the list and the button"}
                .withSize({-1, kRowH})
                .withFlags(LayoutFlags().Border(Side::Top, 12))
        }
    };
}
