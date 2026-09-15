#pragma once

// The application shell: the gallery for Window, MenuBar and Splitter.
//
// It is a real Window -- wxFrame, QMainWindow, an ImGui window in the host
// viewport -- which is the thing a Dialog cannot be: wxMenuBar attaches to a
// wxFrame and nothing else. That is what the menu bar below is here to use; the
// tool bar and status bar join it in T2.4-T2.5. The layout is the shape they
// will all hang off: a browser pane on the left, the work area on the right,
// and a draggable sash between.
//
// Four things about a MenuBar are worth watching:
//
//   * It is the SAME model on all three backends -- MenuItem is plain data in
//     frameworks_core/CoreTypes/, walked by wx into a wxMenuBar, by Qt into a
//     QMenuBar, and drawn by ImGui. Nothing below is written per backend.
//   * A shortcut is parsed ONCE, into fields, and each backend maps those to
//     its own accelerator. "Ctrl" means Cmd on macOS on all three, so one
//     string reads native everywhere.
//   * A check item is an ordinary bound value: "Lock the shell" and the check
//     box further down are the same bool, and so are "Word wrap" and the check
//     box in the control panel (value_binding.hpp) -- across two windows.
//   * isDisabled(bool&) greys live: lock the shell and Undo/Redo go with it.
//
// The StatusBar along the bottom is the third piece of chrome and the same
// story as the tool bar: a native container (wxStatusBar, QStatusBar) that is a
// LEAF to the engine, and the only widget in the framework that defaults to
// Expand() -- a status bar that did not span its parent would not be one. Its
// first field is bound to the tree view's selection, so it follows a click
// without anything in the demo wiring the two together.
//
// The ToolBar under it is the same story again: one ToolItem model, drawn as a
// wxToolBar, a QToolBar and a hand-drawn ImGui button row. Its "Wrap" tool is a
// CHECK tool bound to the same bool as the View menu's check item, so pressing
// either moves the other. Natively a container on wx and Qt, it is a LEAF to
// the engine -- the native control lays its own tools out, so the engine sizes
// one rectangle and the backend fills it.
//
// The same model serves right-click menus: `.withContextMenu({...})` on the
// Table and on the Close button below. Two things to watch there:
//
//   * a DISABLED control opens no menu at all -- lock the shell and the table
//     stops offering one, exactly as it stops offering its tooltip;
//   * the modifier is leaf-only, like withTooltip. There is no container
//     overload to call: a stack has no native window to deliver a right-click.
//
// Two things about a Window are worth watching, both the inverse of a Dialog:
//
//   * it is RESIZABLE by default, with the auto-fit size as its floor -- drag
//     it larger and the splitter panes take the room; Fixed() opts out;
//   * whoever closes it, the caller's bool and onClose() agree. The close
//     button clears the flag, and clearing the flag closes the window -- the
//     second dialog below does exactly that from a CheckBox.
//
// controls_gallery.hpp is long past the ~20-element mark rule 4 sets, so the
// splitter lands here instead, alongside a few already-implemented controls for
// context (TreeView, Table, StaticText, CheckBox, Button).
//
// Two things about a Splitter are worth watching:
//
//   * There is no native splitter behind it. wxSplitterWindow and QSplitter own
//     their children's geometry, which is exactly what the layout engine exists
//     to take back, so all three backends get engine-arranged panes and one 6 px
//     leaf that knows how to be dragged. Drag the sash on each build and the
//     panes land on the same pixel.
//   * The position is the FIRST pane's extent along the splitter's own axis, and
//     it is the panes that re-measure when it changes -- not just the rectangles
//     that move. Narrow the tree pane far enough and the table's columns fold.
//
// Splitters nest: the right-hand pane here is itself a VSplitter, so the shell
// has the three-region layout an editor usually wants. Neither sash can be
// dragged past its pane's floor, which the outer splitter sets explicitly with
// withMinPaneSize() and the inner one leaves at the 40 px default.

#include "declarative_ui.hpp"

#include <string>
#include <utility>
#include <vector>

// `shellOpen` is the bool the shell is shown against and `shellStatus` the line
// onClose() writes when it goes away -- both belong to the caller and have to
// outlive the window, which is modeless on every backend. The control panel in
// value_binding.hpp shares the same two.
inline auto drawAppShellUI(
    std::string& selectedFile,
    TableRows& rows,
    int& selectedRow,
    std::string& notes,
    bool& shellDisabled,
    bool& shellOpen,
    std::string& shellStatus,
    bool& wordWrap)
{
    // Every handler and bound flag below belongs to the caller and outlives the
    // window: the backends COPY the menu model, but a menu is not walked until
    // the user opens one, long after this function has returned.
    MenuBar menuBar { {
        Menu { "File", {
            MenuItem{"New"}.withShortcut("Ctrl+N")
                .onSelect([&notes]() { notes = "File > New"; }),
            MenuItem{"Open..."}.withShortcut("Ctrl+O")
                .onSelect([&notes]() { notes = "File > Open..."; }),
            MenuItem{"Save"}.withShortcut("Ctrl+S")
                .onSelect([&notes]() { notes = "File > Save"; }),
            MenuItem::Separator(),
            // Submenus nest to any depth; the parent item opens, it never selects.
            MenuItem{"Recent"}.withSubmenu({
                MenuItem{"main.cpp"}.onSelect([&notes]() { notes = "Recent > main.cpp"; }),
                MenuItem{"layout.cpp"}.onSelect([&notes]() { notes = "Recent > layout.cpp"; }),
                MenuItem::Separator(),
                MenuItem{"Clear list"}.onSelect([&notes]() { notes = "Recent > Clear list"; }),
            }),
            MenuItem::Separator(),
            // Closing from the menu is the same act as closing from the title
            // bar: clear the bool and the window follows.
            MenuItem{"Close"}.withShortcut("Ctrl+W")
                .onSelect([&shellOpen]() { shellOpen = false; }),
        } },
        Menu { "Edit", {
            // Greyed live: tick "Lock the shell" and these go with it.
            MenuItem{"Undo"}.withShortcut("Ctrl+Z").isDisabled(shellDisabled)
                .onSelect([&notes]() { notes = "Edit > Undo"; }),
            MenuItem{"Redo"}.withShortcut("Ctrl+Shift+Z").isDisabled(shellDisabled)
                .onSelect([&notes]() { notes = "Edit > Redo"; }),
            MenuItem::Separator(),
            // The same bool as the check box below the splitter.
            MenuItem{"Lock the shell"}.checkable(shellDisabled),
        } },
        Menu { "View", {
            // The same bool as the check box in the control panel window.
            // Value first, then the callback -- so this handler already reads
            // the state the user just selected.
            MenuItem{"Word wrap"}.withShortcut("Ctrl+Shift+W").checkable(wordWrap)
                .onSelect([&notes, &wordWrap]() {
                    notes = wordWrap ? "Word wrap is ON" : "Word wrap is OFF";
                }),
            // A snapshot check item: the framework owns this one, so ticking it
            // is visible but nothing outside the menu can see or change it.
            MenuItem{"Show line numbers"}.checkable(true),
        } },
        Menu { "Help", {
            MenuItem{"About"}.withShortcut("F1")
                .onSelect([&notes]() { notes = "DeclarativeUI-CPP -- one tree, three backends."; }),
            // A snapshot disabled flag: greyed for good, not bound to anything.
            MenuItem{"Check for updates..."}.isDisabled(true),
        } },
    } };

    // Same pinning discipline as the other galleries: explicit sizes on the
    // leaves and MinSize on the containers, so the three backends compute the
    // same frames despite their different native metrics.
    constexpr int kRowH = 26;
    constexpr int kLabelH = 20;
    constexpr int kShellH = 360;
    constexpr Size kButtonSize { 110, 28 };

    const std::vector<TreeItem> projectTree {
        { "src", {
            { "main.cpp" },
            { "engine", { { "layout.cpp" }, { "measure.cpp" } } },
        }, true },
        { "include", { { "widgets.hpp" }, { "splitter.hpp" } } },
        { "tests", { { "layout_tests.cpp" } } },
    };

    const std::vector<TableColumn> columns {
        { "File",  -1, /*sortable*/ true },
        { "Size",  70, /*sortable*/ true },
        { "Notes", -1, /*sortable*/ false, /*editable*/ true },
    };

    return Window {
        "Application Shell",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 10),
            StaticText{"Drag either sash; the panes re-measure, they do not just move."}
                .withSize({-1, kLabelH}),
            // The tool bar. No icon files ship with the demo, so every tool
            // falls back to its label -- which is exactly the R9.3 path a
            // missing icon takes, and it renders identically on all three.
            ToolBar {{
                ToolItem{"New"}.withTooltip("New file")
                    .onClick([&notes]() { notes = "Toolbar > New"; }),
                ToolItem{"Open"}.withTooltip("Open file")
                    .onClick([&notes]() { notes = "Toolbar > Open"; }),
                ToolItem{"Save"}.withTooltip("Save file")
                    .onClick([&notes]() { notes = "Toolbar > Save"; }),
                ToolItem::Separator(),
                // A check tool: down while the bool is true, and it is the same
                // bool as the View menu's check item and the control panel's
                // check box -- a four-way binding once the toolbar joins in.
                ToolItem{"Wrap"}.withTooltip("Word wrap (Ctrl+Shift+W)")
                    .toggled(wordWrap),
                ToolItem::Separator(),
                // Greys out live when the shell is locked, like Edit > Undo.
                ToolItem{"Delete"}.withTooltip("Disabled while the shell is locked")
                    .isDisabled(shellDisabled)
                    .onClick([&notes]() { notes = "Toolbar > Delete"; }),
            }}
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 8)),
            // 240 is the first pane's width in pixels, snapshotted from a
            // literal -- the splitter keeps its own copy of it. The binding
            // demo in value_binding.hpp is where an int& goes instead.
            HSplitter { LayoutFlags().Expand().Border(Side::Top, 8).MinSize({720, kShellH}),
                240,
                VStack {
                    LayoutFlags().Expand(),
                    StaticText{"Project"}
                        .withSize({-1, kLabelH}),
                    TreeView { projectTree, selectedFile }
                        .withVisibleRows(10)
                        .withFlags(LayoutFlags().Proportion(1).Expand().Border(Side::Top, 4))
                        .isDisabled(shellDisabled)
                },
                VSplitter { LayoutFlags().Expand(),
                    220,
                    VStack {
                        LayoutFlags().Expand(),
                        StaticText{"Files"}
                            .withSize({-1, kLabelH}),
                        Table { columns, rows, selectedRow }
                            .withVisibleRows(6)
                            .withFlags(LayoutFlags().Proportion(1).Expand().Border(Side::Top, 4))
                            .isDisabled(shellDisabled)
                            // Right-click the table. Same MenuItem model as the
                            // menu bar, so separators and submenus work here
                            // too -- and locking the shell disables the table,
                            // after which it opens no menu at all (a disabled
                            // control has no context menu on any backend).
                            .withContextMenu({
                                MenuItem{"Open"}.onSelect([&notes]() { notes = "Context > Open"; }),
                                MenuItem{"Rename..."}.onSelect([&notes]() { notes = "Context > Rename..."; }),
                                MenuItem::Separator(),
                                MenuItem{"Copy"}.withSubmenu({
                                    MenuItem{"Copy name"}.onSelect([&notes]() { notes = "Context > Copy name"; }),
                                    MenuItem{"Copy path"}.onSelect([&notes]() { notes = "Context > Copy path"; }),
                                }),
                                MenuItem::Separator(),
                                MenuItem{"Reveal in project"}.onSelect([&notes]() { notes = "Context > Reveal"; }),
                            })
                    },
                    VStack {
                        LayoutFlags().Expand(),
                        StaticText{"Notes"}
                            .withSize({-1, kLabelH}),
                        MultiLineTextCtrl{notes}
                            .withFlags(LayoutFlags().Proportion(1).Expand().Border(Side::Top, 4))
                            .isDisabled(shellDisabled)
                    }
                }
                // The inner sash keeps the 40 px default on both panes.
            }
            // Neither outer pane may be dragged below this, whatever the user
            // does with the sash -- a pane's own MinSize would say the same
            // thing, and the larger of the two wins.
            .withMinPaneSize(160, 300),
            CheckBox{shellDisabled, "Lock the whole shell (sashes included)"}
                .withSize({-1, kRowH})
                .withFlags(LayoutFlags().Border(Side::Top, 10)),
            Separator{}
                .withSize({-1, 1})
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 10)),
            // The status bar. Its first field is BOUND to the same string the
            // tree view writes its selection into, so clicking a file in the
            // project pane updates the bar live -- nothing polls it in the
            // demo, the backends do. The other two are fixed-width snapshots.
            StatusBar {{
                StatusField{ selectedFile },
                StatusField{ "Ln 1, Col 1", 120 },
                StatusField{ "UTF-8", 70 },
            }}
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 10)),
            HStack {
                LayoutFlags().Border(Side::Top, 10),
                Spacer{},
                // Closing from inside is the same act as closing from the
                // title bar: clear the bool and the window follows.
                Button{"Close"}
                    .withSize(kButtonSize)
                    .withFlags(LayoutFlags().CenterVertical())
                    .onClick([&shellOpen]() { shellOpen = false; })
                    // A context menu on a plain leaf, carrying the two bound
                    // kinds of item: a check mark that is the same bool as the
                    // box above and the Edit menu's item, and a command that
                    // greys itself live when that bool goes true.
                    .withContextMenu({
                        MenuItem{"Lock the shell"}.checkable(shellDisabled),
                        MenuItem::Separator(),
                        MenuItem{"Close the shell"}.withShortcut("Ctrl+W")
                            .onSelect([&shellOpen]() { shellOpen = false; }),
                        MenuItem{"Delete"}.isDisabled(shellDisabled)
                            .onSelect([&notes]() { notes = "Context > Delete"; }),
                    })
            }
        }
    }
    .withMenuBar(std::move(menuBar))
    // Fires once however the window went away -- this button, the title bar's
    // close button, or the control panel's check box clearing the bool.
    .onClose([&shellStatus]() {
        shellStatus = "Shell closed -- onClose() fired once.";
    });
}
