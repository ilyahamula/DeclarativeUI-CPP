#pragma once

// Pickers: the gallery dialog for FilePicker, the CheckListBox and the one-shot
// FileDialog, alongside a few already-implemented controls for context
// (StaticText, TextCtrl, Button, CheckBox, Separator).
//
// controls_gallery.hpp is long past the ~20-element mark rule 4 sets, so the
// pickers land here instead.
//
// A FilePicker is a path field plus a Browse button, in one of three modes.
// What it opens is the one place the three backends genuinely differ:
//
//   * wx  -- wxFilePickerCtrl / wxDirPickerCtrl, so the platform's own dialog
//   * Qt  -- a QLineEdit + QToolButton composite; Browse runs the blocking
//            QFileDialog, so the control flow matches wx exactly
//   * ImGui -- no OS dialog exists and the project takes no dependency for
//            one, so Browse opens a browser the framework draws itself
//            (frameworks_core/imgui/FileBrowserPopup.cpp): a breadcrumb, a
//            listing, a filter combo, a name field for Save, Open/Cancel.
//            Deliberately simpler than the native dialogs -- no favourites, no
//            previews, no network places.
//
// Two behaviours are worth watching for, because they are the same on all three:
//
//   * TYPING a path is as much a selection as picking one. Edit the field and
//     the bound string and onChange both see it (R11.4).
//   * CANCELLING leaves the path alone. Cancel is not a selection of "" -- the
//     empty string is what onResult reports, but nothing is written.
//
// The CheckListBox in the middle is a list with a checkbox on every row. Its
// value is the CHECKED SET and so is always a vector: std::vector<std::string>
// here names the ticked plugins by their text, std::vector<int> would name them
// by position. Highlight selection is NOT part of it -- a row can be
// highlighted without being ticked on all three backends.
//
// The placeholder box below it is T3.3: greyed text shown only while a field is
// EMPTY. It is a label, not a value -- nothing reads it back, and no backend
// measures it, so the wording can be as long as it likes without moving this
// auto-fit dialog. wx is the one backend whose field could notice (its best
// size is pinned across SetHint for exactly that reason). Single-line only:
// MultiLineTextCtrl does not declare the modifier, because wxTextCtrl::SetHint
// does nothing on a wxTE_MULTILINE control on any wx port.
//
// The FileDialog button below is the other half of T3.1: a one-shot dialog that
// is not a widget at all, called from a handler exactly as MessageBox is. Note
// it is BLOCKING on wx and Qt and non-blocking on ImGui, where nothing may stop
// the frame loop -- which is why the answer belongs in onResult on all three
// rather than in code after show().
//
// Geometry is pinned throughout (rules.md G6): the three backends measure a
// path field very differently, so withSize() on the leaves is what makes the
// frames come out identical.

#include "declarative_ui.hpp"
#include "filedialog.hpp"

#include <string>
#include <vector>

// The checked set read back as one line. Callers seed their summary string with
// it so the field below starts out agreeing with the boxes.
inline std::string pluginSummary(const std::vector<std::string>& plugins)
{
    if (plugins.empty())
        return "(none)";
    std::string text = plugins.front();
    for (std::size_t i = 1; i < plugins.size(); ++i)
        text += ", " + plugins[i];
    return text;
}

inline auto drawPickersGalleryUI(std::string& openPath, std::string& savePath,
    std::string& folderPath, std::string& lastDialogResult, bool& pickersDisabled,
    std::vector<std::string>& enabledPlugins, std::string& enabledSummary,
    std::string& searchTerm, std::string& apiKey)
{
    constexpr int kRowH = 28;
    constexpr int kLabelH = 20;
    constexpr int kBoxW = 520;
    constexpr int kLabelW = 96;
    constexpr Size kPickerSize { 380, 28 };
    constexpr Size kButtonSize { 150, 28 };
    constexpr int kPluginsW = 220;
    constexpr int kSummaryW = 270;

    return Dialog {
        "Pickers Gallery",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 10),
            VGroupBox { "FilePicker",
                LayoutFlags().Expand().MinSize({kBoxW, -1}),
                StaticText{"Browse opens the native dialog on wx and Qt, a drawn one on ImGui."}
                    .withSize({-1, kLabelH}),

                // Open: the filter is the wx wildcard spelling, parsed once by
                // CoreTypes/FileFilter.hpp and mapped to each backend's own.
                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 8),
                    StaticText{"Open:"}
                        .withSize({kLabelW, kLabelH})
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8)),
                    FilePicker{openPath}
                        .withMode(FileMode::Open)
                        .withFilter("Sources (*.cpp;*.hpp)|*.cpp;*.hpp|All files|*")
                        .withDialogTitle("Open a source file")
                        .withSize(kPickerSize)
                        .withTooltip("Pick an existing file, or type a path")
                        .isDisabled(pickersDisabled)
                },

                // Save: the same control in the mode that lets the user NAME a
                // file that does not exist yet.
                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 6),
                    StaticText{"Save as:"}
                        .withSize({kLabelW, kLabelH})
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8)),
                    FilePicker{savePath}
                        .withMode(FileMode::Save)
                        .withFilter("Text (*.txt)|*.txt|All files|*")
                        .withDialogTitle("Save the report as")
                        .withSize(kPickerSize)
                        .isDisabled(pickersDisabled)
                },

                // Directory: no filter, because there are no files to filter --
                // every backend ignores one set here.
                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 6),
                    StaticText{"Folder:"}
                        .withSize({kLabelW, kLabelH})
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8)),
                    FilePicker{folderPath}
                        .withMode(FileMode::Directory)
                        .withDialogTitle("Choose an output folder")
                        .withSize(kPickerSize)
                        .isDisabled(pickersDisabled)
                },

                CheckBox{pickersDisabled, "Disable the pickers above"}
                    .withSize({-1, kRowH})
                    .withFlags(LayoutFlags().Border(Side::Top, 8))
            },

            Separator{}
                .withSize({-1, 1})
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 10)),

            VGroupBox { "CheckListBox",
                LayoutFlags().Expand().MinSize({kBoxW, -1}).Border(Side::Top, 10),
                StaticText{"A checkbox per row; the bound vector IS the checked set."}
                    .withSize({-1, kLabelH}),
                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 8),
                    // withVisibleRows() drives the intrinsic HEIGHT on all three
                    // backends -- their native hints disagree far too much
                    // otherwise (wx sizes to the item count, Qt returns a fixed
                    // ~192 px, ImGui has no hint at all). The width is pinned
                    // for the same reason the pickers above are.
                    CheckListBox{
                        {"Formatter", "Linter", "Debugger", "Profiler", "Spell check"},
                        enabledPlugins}
                        .withVisibleRows(4)
                        .withSize({kPluginsW, -1})
                        .withTooltip("Tick the plugins to load at startup")
                        .isDisabled(pickersDisabled)
                        // Value first, then the callback: the bound vector is
                        // already the new set when this runs, so it is also
                        // what the handler is handed.
                        .onChange([&enabledSummary](const std::vector<std::string>& plugins) {
                            enabledSummary = pluginSummary(plugins);
                        }),
                    Spacer{Size{12, 0}},
                    // The same set as text. A BOUND, disabled field rather than
                    // a ReadonlyTextCtrl, for the reason the FileDialog result
                    // below is one: the readonly control owns a copy of its text
                    // and would never show a value that arrives later on wx and
                    // Qt, where the tree is built once.
                    VStack {
                        LayoutFlags().Expand(),
                        StaticText{"Enabled:"}.withSize({-1, kLabelH}),
                        TextCtrl{enabledSummary}
                            .withSize({kSummaryW, kRowH})
                            .withFlags(LayoutFlags().Border(Side::Top, 4))
                            .isDisabled(),
                        Spacer{}
                    }
                }
            },

            Separator{}
                .withSize({-1, 1})
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 10)),

            VGroupBox { "withPlaceholder (single-line fields)",
                LayoutFlags().Expand().MinSize({kBoxW, -1}).Border(Side::Top, 10),
                StaticText{"The hint shows while a field is empty and vanishes as you type."}
                    .withSize({-1, kLabelH}),
                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 8),
                    StaticText{"Search:"}
                        .withSize({kLabelW, kLabelH})
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8)),
                    // Starts empty, so this is the one showing its hint.
                    TextCtrl{searchTerm}
                        .withPlaceholder("Search files…")
                        .withSize(kPickerSize)
                        .withTooltip("Type to hide the hint; clear it to bring it back")
                },
                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 6),
                    StaticText{"API key:"}
                        .withSize({kLabelW, kLabelH})
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8)),
                    // A password field is what most wants a hint: the echo
                    // hides the value, so there is nothing else to say what
                    // belongs here.
                    PasswordInput{apiKey}
                        .withPlaceholder("Paste your key")
                        .withSize(kPickerSize)
                },
                // The same modifier with the field NOT empty, so the hint is
                // hidden from the first frame -- the state the two above only
                // reach once the user types.
                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 6),
                    StaticText{"Prefilled:"}
                        .withSize({kLabelW, kLabelH})
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8)),
                    TextCtrl{std::string{"already has text"}}
                        .withPlaceholder("you should not see this")
                        .withSize(kPickerSize)
                }
            },

            Separator{}
                .withSize({-1, 1})
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 10)),

            VGroupBox { "FileDialog (one-shot, like MessageBox)",
                LayoutFlags().Expand().MinSize({kBoxW, -1}).Border(Side::Top, 10),
                StaticText{"Not a widget: no node, no layout. Called from the handler."}
                    .withSize({-1, kLabelH}),
                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 8),
                    Button{"Open a file..."}
                        .withSize(kButtonSize)
                        .isDisabled(pickersDisabled)
                        .onClick([&lastDialogResult] {
                            FileDialog{"Pick any file"}
                                .withMode(FileMode::Open)
                                .withFilter("All files|*")
                                .onResult([&lastDialogResult](const std::string& path) {
                                    // "" on cancel -- the framework's "no
                                    // selection" spelling for a path, so the
                                    // result can be written straight through.
                                    lastDialogResult = path.empty() ? "(cancelled)" : path;
                                })
                                .show();
                        }),
                    Spacer{Size{12, 0}},
                    Button{"Choose a folder..."}
                        .withSize(kButtonSize)
                        .isDisabled(pickersDisabled)
                        .onClick([&lastDialogResult] {
                            FileDialog{"Pick a folder"}
                                .withMode(FileMode::Directory)
                                .onResult([&lastDialogResult](const std::string& path) {
                                    lastDialogResult = path.empty() ? "(cancelled)" : path;
                                })
                                .show();
                        }),
                    Spacer{}
                },
                // A BOUND field, disabled, rather than a ReadonlyTextCtrl: the
                // readonly one owns a copy of its text (T0.1) and so would
                // never show a result that arrives later on wx and Qt, where
                // the tree is built once. Bound, the RefSync poll mirrors it.
                // Pinned width, because a display widget must not measure its
                // live text -- an arriving path would resize this auto-fit
                // dialog.
                HStack {
                    LayoutFlags().Expand().Border(Side::Top, 8),
                    StaticText{"Result:"}
                        .withSize({kLabelW, kLabelH})
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 8)),
                    TextCtrl{lastDialogResult}
                        .withSize(kPickerSize)
                        .isDisabled()
                }
            }
        }
    };
}
