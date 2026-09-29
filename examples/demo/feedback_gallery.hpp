#pragma once

// Feedback: the gallery for a Dialog's LIFECYCLE -- Modal(), onClose() and
// show(bool&) -- as an OK/Cancel form, which is the shape those three exist for,
// and for the Toast: feedback that needs no dialog at all.
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
//   * OPENING is declared on the button: onClickShow(confirmOpen, Dialog{...})
//     is the whole of it, written once for all three backends. It is a show()
//     from the click handler -- wx and Qt destroy the native dialog when it
//     closes, so each open is a fresh one; ImGui ADOPTS a handler's show() and
//     keeps drawing the dialog until its flag clears, nested inside this
//     window, which is what lets a modal open on top of another window's frame.
//     The flag also keeps it to ONE: click Delete again while the box is up
//     and nothing happens. No main supplies anything, and none calls the box.
//   * A TOAST is the other half of feedback: "Save" answers with a notice that
//     asks nothing, takes no focus and leaves on its own after 2 s. Click it a
//     few times and the notices stack upwards from the bottom-right corner
//     instead of overlapping. The modal's Delete button raises one too, from a
//     handler inside a modal: on ImGui it is drawn on the foreground draw list,
//     so it stays above the modal's dimming; on wx and Qt it is a window of its
//     own that is never activated, so the modal keeps the keyboard.
//
// controls_gallery.hpp is long past the ~20-element mark rule 4 sets, so this
// lands as its own dialog alongside a few already-implemented controls for
// context (ListBox, StaticText, TextCtrl, Separator, CheckBox, Button).
//
// The About panel at the bottom is the gallery for RICHTEXT: read-only text with
// **bold**, *italic*, [links](url) and {#rrggbb}colour{/}, written as markup.
// Three things to watch:
//
//   * it WRAPS by one rule on all three backends. No native rich-text control
//     is involved anywhere: the markup is parsed once into runs, one shared
//     routine breaks them into lines at spaces, and each backend only draws the
//     pieces with its own fonts. The paragraphs are pinned at one width, so the
//     line breaks move only as far as the fonts' glyph widths differ.
//   * a link is REPORTED, never opened. onLink(url) hands the url to the
//     caller, which here writes it to the field below the text and raises a
//     toast. Press on a link, drag off it and let go: nothing fires, on any
//     backend -- the press and the release must both land on the same link.
//   * it is an ordinary leaf, so withTooltip() and withContextMenu() come free.

#include "declarative_ui.hpp"

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
                        Toast{"Deleted " + file}
                            .withStyle(MessageBoxStyle::Warning)
                            .show();
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

// The parent. It declares the modal on its own button: see the fourth note at
// the top of this file.
inline auto drawDeleteFormUI(std::string& file, std::string& answer,
    std::string& closeReport, bool& formDisabled, bool& confirmOpen, int& closeCount)
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
                        // The box, declared where it is asked for. One open at
                        // a time on every backend: the flag is set while it is
                        // up, and a click then does nothing.
                        .onClickShow(confirmOpen,
                            drawConfirmDeleteUI(confirmOpen, file, answer, closeReport, closeCount)),
                    // No dialog, no flag, no answer: the notice is shown and
                    // forgotten. The message is copied when it is shown, so
                    // each toast keeps the file name it was raised for.
                    Button{"Save"}
                        .withSize(kButtonSize)
                        .withTooltip("Shows a 2-second toast -- click it a few times")
                        .withFlags(LayoutFlags().Border(Side::Top, 8))
                        .isDisabled(formDisabled)
                        .onClick([&file]() {
                            Toast{"Saved " + file}.withDuration(2000).show();
                        }),
                    // The other top-level spelling, and the other overload: no
                    // flag, because nothing here needs to read or clear it. The
                    // framework keeps one per title, so it is still at most one
                    // log window. Its fields are bound, so the log follows the
                    // form while both are open.
                    Button{"Show log window..."}
                        .withSize(kButtonSize)
                        .withTooltip("Opens a Window -- at most one")
                        .withFlags(LayoutFlags().Border(Side::Top, 8))
                        .onClickShow(
                            Window {
                                "Deletion log",
                                VStack {
                                    LayoutFlags().Expand().Border(Side::All, 14).MinSize({360, -1}),
                                    StaticText{"Last answer:"}.withSize({-1, kLabelH}),
                                    TextCtrl{answer}
                                        .withSize({-1, kRowH})
                                        .withFlags(LayoutFlags().Expand().Border(Side::Top, 4)),
                                    StaticText{"Last onClose() report:"}
                                        .withSize({-1, kLabelH})
                                        .withFlags(LayoutFlags().Border(Side::Top, 10)),
                                    TextCtrl{closeReport}
                                        .withSize({-1, kRowH})
                                        .withFlags(LayoutFlags().Expand().Border(Side::Top, 4))
                                }
                            }),
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

            CheckBox{formDisabled, "Lock the list and the buttons"}
                .withSize({-1, kRowH})
                .withFlags(LayoutFlags().Border(Side::Top, 12))
        }
    };
}

// RichText's gallery: an About panel, with a few already-implemented controls
// around it for context (StaticText, TextCtrl, Separator, Button).
inline auto drawAboutUI(std::string& lastLink)
{
    constexpr int kRowH = 28;
    constexpr int kLabelH = 20;
    constexpr int kTextW = 420;
    constexpr Size kButtonSize { 110, 28 };

    // Writes the url where the panel shows it and says so with a toast: the
    // framework never opens a url itself, so the "opening" is the caller's.
    auto follow = [&lastLink](const std::string& url) {
        lastLink = url;
        Toast{"Link: " + url}.withDuration(1500).show();
    };

    return Dialog {
        "About DeclarativeUI",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 14),
            RichText{"**DeclarativeUI** {#808080}version 0.9{/}"}
                .withSize({kTextW, -1}),

            RichText{"One widget tree, compiled against *ImGui*, *wxWidgets* or *Qt* by "
                     "switching one CMake variable. A single layout engine computes every "
                     "rectangle, so **the same tree lays out identically** on every backend."}
                .withSize({kTextW, -1})
                .withFlags(LayoutFlags().Border(Side::Top, 10))
                .withTooltip("RichText wraps at spaces, by one rule on all three backends"),

            RichText{"Status: {#2e9e44}**stable**{/} layout engine, "
                     "{#d08a00}*preview*{/} self-rendered backend.\n"
                     "Markup is literal when escaped: \\*not italic\\*, \\[not a link\\]."}
                .withSize({kTextW, -1})
                .withFlags(LayoutFlags().Border(Side::Top, 10)),

            RichText{"Read the [README](https://example.com/readme), browse the "
                     "[**widget catalogue**](https://example.com/widgets) or "
                     "[report a bug](https://example.com/issues)."}
                .withSize({kTextW, -1})
                .withFlags(LayoutFlags().Border(Side::Top, 10))
                .onLink(follow)
                .withContextMenu({
                    MenuItem{"Open the README"}.onSelect([follow]() { follow("https://example.com/readme"); }),
                    MenuItem{"Open the issue tracker"}.onSelect([follow]() { follow("https://example.com/issues"); }),
                }),

            Separator{}
                .withSize({-1, 1})
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 12)),

            StaticText{"Last link clicked (reported by onLink, not opened):"}
                .withSize({-1, kLabelH})
                .withFlags(LayoutFlags().Border(Side::Top, 10)),
            TextCtrl{lastLink}
                .withSize({-1, kRowH})
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 4)),

            HStack {
                LayoutFlags().Expand().Border(Side::Top, 12),
                Spacer{},
                Button{"Clear"}
                    .withSize(kButtonSize)
                    .onClick([&lastLink]() { lastLink.clear(); })
            }
        }
    };
}
