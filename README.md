# DeclarativeUI-CPP

A declarative, backend-agnostic UI framework for C++. Describe your UI as a composable widget tree and target different rendering backends — **ImGui**, **wxWidgets**, or **Qt** — by switching a single compile-time flag.

Inspired by [this video](https://www.youtube.com/watch?v=xu4pI72zlO4).

---

## Overview

DeclarativeUI-CPP lets you write UI code once using a clean, chainable builder API, then compile it against the backend of your choice without touching your UI logic.

```cpp
return Dialog {
    "Controls Demo",
    VStack {
        LayoutFlags().Expand().Border(Side::All, 10),
        VGroupBox { "Text Input",
            MultiLineTextCtrl{multilineText}.withFlags(LayoutFlags().Expand()),
            HStack {
                StaticText{"Password:"}.withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 5)),
                PasswordInput{password}.withFlags(LayoutFlags().Proportion(1).Expand())
            }
        },
        HGroupBox { "Date & Time",
            LayoutFlags().Border(Side::All, 5),
            VStack { StaticText{"Date"}, DatePicker{date} },
            VStack { StaticText{"Time"}, TimePicker{time} }
        },
        HStack {
            StaticText{""}.withFlags(LayoutFlags().Proportion(1)),
            Button{"Check"}.withFlags(LayoutFlags().CenterVertical()).onClick(onCheck)
        }
    }
};
```

---

## Features

- **Declarative widget tree** — compose layouts using `VStack`, `HStack`, `VGroupBox`, `HGroupBox`, `TabPanel`, and `Dialog` or `Window`
- **Framework-owned layout engine** — one shared measure/arrange engine computes every rectangle; backends only measure native widgets and place them, so the same tree follows identical layout rules on every backend (see `docs/specs/custom_layout_system/`)
- **Sensible defaults, no flags required** — widgets size to their content (text fields never collapse below their text), sibling group boxes in a column equalize to the widest one (tallest in a row), and dialogs auto-fit their content
- **Flexible layout flags** — `LayoutFlags` with `Expand()`, `Proportion()`, `Border()`, `CenterVertical()`, `Center()`, `MinSize()`/`MaxSize()`, `SizeGroup()` (equalize across parents), and `AutoGrow()` (field re-measures as you type)
- **Dialog sizing policy** — dialogs are not user-resizable by default; opt in with `Dialog::Resizable()`, where the auto-fit size becomes the initial *and minimum* size so content can never be clipped
- **CRTP widget hierarchy** — `Widget<T>` base with fluent `.withFlags()`, `.withSize()`, `.withPosition()`, `.withStyle()` modifiers
- **Two-way data binding** — how you pass the value decides: `Slider{range, 50}` takes a snapshot, `Slider{range, myValue}` *binds* to your variable and writes edits straight back to it. Two controls sharing one variable stay in step with no callback wiring, and a value changed from anywhere else is picked up live
- **Selection mode from the bound type** — `ListBox<std::string>` selects one item, `ListBox<std::vector<int>>` selects many; `Table` binds either a row index or a key column. There is no mode flag to keep in step with the value
- **Disable anything** — `.isDisabled(flag)` on any widget, or on a `VStack`/`GroupBox`/`TabPanel`/`Tab` to grey out its whole subtree. Bind it to a `bool&` and it flips live, without rebuilding the tree
- **Tooltips** — `.withTooltip("…")` on any leaf widget, either fixed text or bound to a `std::string&` that can change at runtime
- **Event callbacks** — `.onClick()`, `.onChange()`, `.onHover()`, plus `.onCellChange()` on `Table`; each also has an overload receiving the native widget handle
- **Multi-backend** — compile against ImGui, wxWidgets, or Qt by switching one CMake variable

## Widget Catalogue

| Category          | Widgets |
|-------------------|---------|
| Text              | `StaticText`, `ReadonlyTextCtrl`, `ClickableText`, `LinkText` |
| Text input        | `TextCtrl`, `PasswordInput`, `MultiLineTextCtrl` |
| Buttons & choice  | `Button`, `ToggleButton`, `CheckBox`, `RadioButton<T>`, `ComboBox<T>` |
| Lists & tables    | `ListBox<T>`, `CheckListBox<T>`, `TreeView<T>`, `Table<T>` |
| Numeric           | `SpinBox<T>`, `Slider<T>` |
| Pickers           | `DatePicker`, `TimePicker`, `ColorPicker`, `FilePicker` (Open / Save / Directory) |
| Display           | `ProgressBar`, `Separator` (horizontal or vertical), `Image` |
| Layout            | `Spacer` |
| Chrome            | `ToolBar` + `ToolItem`, `StatusBar` + `StatusField` |
| Containers        | `VStack` / `HStack`, `Grid`, `ScrollPanel`, `HSplitter` / `VSplitter`, `Expander`, `VGroupBox` / `HGroupBox`, `TabPanel` + `Tab` |
| Top-level         | `Dialog`, `Window`, `MessageBox`, `FileDialog` |
| Application chrome| `MenuBar` + `Menu` + `MenuItem` (on a `Window`), `.withContextMenu()` on any leaf |

`ListBox`, `CheckListBox`, `TreeView` and `Table` all take `.withVisibleRows(n)`, which
drives their intrinsic height identically on every backend — the native hints disagree
far too much for the same tree to lay out the same way otherwise. `TreeView` addresses
items by path (`"src/engine"`) rather than index; `Table` columns are individually
sortable and editable, and rows keep their original index so a binding survives sorting.

`CheckListBox` is a list with a checkbox on every row, and its bound value **is** the
checked set — so it is always a vector, and there is no single-value spelling because
"one box ticked" is not a different control. Highlight selection is deliberately not
part of it: a row can be highlighted without being ticked.

```cpp
CheckListBox{ {"Formatter", "Linter", "Debugger"}, enabledPlugins }  // vector<string>: by text
    .withVisibleRows(4)
    .onChange([&](const std::vector<std::string>& on) { reload(on); })

CheckListBox{ items, checkedRows }                                   // vector<int>: by position
```

Which vector you bind decides how the ticks are named: `std::vector<std::string>` names
them by item *text*, so two lists holding the same items in a different order tick the
same rows; `std::vector<int>` names them by *position*, which is the right choice when
the labels are not unique. Both decode through the same helpers `ListBox`'s multi-select
bindings use, so what one control calls "ticked" the other calls "selected".

`HSplitter` / `VSplitter` are arranged by the engine rather than by a native
splitter — `wxSplitterWindow` and `QSplitter` own their children's geometry, which
is exactly what the layout engine takes back — so the same tree divides the same
way on all three backends. Bind the sash position to an `int&` and dragging writes
through to it; writing it from anywhere else moves the sash.

`Dialog` and `Window` are the two top-level spellings and share the engine behind
them; the defaults are opposites, because the roles are. A `Dialog` is a transient
box the engine sizes exactly and the user cannot resize. A `Window` is the
application frame — `wxFrame`, `QMainWindow`, an ImGui window in the host
viewport — so it is **resizable by default** with the auto-fit size as its floor,
and `Fixed()` is what opts out. A `Window` is also the only thing a menu bar can
attach to on wx. `show(bool& open)` makes a caller-owned bool the single truth
about whether the window is up: clearing it closes the window, closing the window
clears it, and `onClose()` fires exactly once either way.

`Window::withMenuBar()` attaches nested menus with separators, submenus, checkable
items and per-item disabling — native chrome outside the content area on wx and Qt,
a menu row inside the window on ImGui. A shortcut is written once as text
(`.withShortcut("Ctrl+Shift+S")`), parsed once into a `Shortcut`, and mapped by each
backend to its own accelerator; **`Ctrl` means Cmd on macOS on all three**, so one
string reads native everywhere. A check item is an ordinary bound value:
`.checkable(wordWrap)` and a `CheckBox` on the same `bool&` stay in step, and
`.isDisabled(flag)` greys an item live.

`ToolBar` is the row of commands under the menu bar. It is a container natively — a
`wxToolBar`, a `QToolBar`, a drawn button row on ImGui — but a **leaf** to the layout
engine: the native control lays its own tools out, so the engine sizes one rectangle.
A tool with no icon, or one whose icon fails to load, shows its label instead, so a
toolbar is never blank.

```cpp
ToolBar {{
    ToolItem{"New"}.withIcon("icons/new.png").onClick([&] { newFile(); }),
    ToolItem::Separator(),
    ToolItem{"Wrap"}.toggled(wordWrap),          // a check tool, on the caller's bool
    ToolItem{"Delete"}.isDisabled(locked),       // greys live
}}
```

`StatusBar` is the row of text along the bottom. It is the one widget that defaults to
`Expand()` — a status bar that did not span its parent would not be one — and it
deliberately **does not measure its own text**: a field is there to show a string
written from somewhere else, so measuring it would let an arriving message resize an
auto-fit window. Fields with a fixed width keep it; the rest share what is left.

```cpp
StatusBar {{
    StatusField{ status },              // bound: anything that writes it shows live
    StatusField{ "Ln 1, Col 1", 120 },  // fixed width
    StatusField{ "UTF-8", 70 },
}}
StatusBar{ status }                     // or one stretched field, the common case
```

`FilePicker` is a path field with a Browse button, in one of three modes. **Both** ways
of setting the path commit identically — picking one in the dialog and typing one into
the field — so nothing downstream can tell them apart, and **cancelling leaves the path
alone** rather than clearing it.

```cpp
FilePicker{ projectPath }                       // bound: edits write through
    .withMode(FileMode::Open)                   // or Save, or Directory
    .withFilter("Images (*.png;*.jpg)|*.png;*.jpg|All files|*")
    .withDialogTitle("Open a project")

FileDialog{"Export"}                            // one-shot, like MessageBox
    .withMode(FileMode::Save)
    .onResult([&](const std::string& path) {    // "" on cancel
        if (!path.empty()) exportTo(path);
    })
    .show();
```

The filter is written once in the wx wildcard spelling and parsed once into a
`FileFilter`; each backend then rebuilds its *own* wildcard string from the fields,
because the three disagree on every separator. Browse opens the platform dialog on wx
and Qt. **On ImGui it opens a browser the framework draws itself** — there is no OS
dialog and this project takes no new dependency for one — so it is deliberately simpler
than the native ones: a breadcrumb, a listing, a filter combo, a name field for Save,
Open/Cancel, and no favourites or previews. `FileDialog` blocks on wx and Qt and does
not on ImGui, where nothing may stop the frame loop, which is why the answer belongs in
`onResult` on all three rather than in code after `show()`.

The same `MenuItem` model is a right-click menu on any leaf:

```cpp
Table { columns, rows, selected }
    .withContextMenu({
        MenuItem{"Open"}.onSelect([&] { open(); }),
        MenuItem::Separator(),
        MenuItem{"Copy"}.withSubmenu({ MenuItem{"Name"}, MenuItem{"Path"} }),
        MenuItem{"Delete"}.isDisabled(locked),
    })
```

`.withContextMenu()` is **leaf-only**, like `.withTooltip()` — a container has no
native window to deliver a right-click, so there is no container overload and asking
for one will not compile. A **disabled** control opens no menu, exactly as it shows no
tooltip. A popup is rebuilt from the model each time it opens, so bound check marks and
bound disabling are always current without anything polling them; a shortcut on a
context-menu item is *displayed* but not registered, on every backend.

```cpp
Window { "Editor", content }
    .withMenuBar(MenuBar { {
        Menu { "File", {
            MenuItem{"New"}.withShortcut("Ctrl+N").onSelect([&] { newFile(); }),
            MenuItem::Separator(),
            MenuItem{"Recent"}.withSubmenu({ MenuItem{"main.cpp"}, MenuItem{"layout.cpp"} }),
        } },
        Menu { "View", {
            MenuItem{"Word wrap"}.withShortcut("Ctrl+Shift+W").checkable(wordWrap),
        } },
    } })
    .show(open);
```

`Expander` folds a section away behind a clickable header. Collapsed, the content
costs the layout *nothing at all* — not its size, not its margins, not even the gap
above it — so an auto-fit dialog shrinks and grows as sections close and open. Bind
the open state to a `bool&` and clicking the header writes through to it, exactly as
the splitter's sash does with its `int&`.

---

## Building

Select your backend via the `UI_FRAMEWORK` CMake variable (`IMGUI`, `WX`, or `QT`):

```sh
cmake -DUI_FRAMEWORK=IMGUI -B build/imgui
cmake --build build/imgui
```

Per-backend build directories (`build/imgui`, `build/wx`, `build/qt`) can coexist.
Unit tests for the layout engine are backend-free:

```sh
ctest --test-dir build/imgui        # or run build/<backend>/tests/layout_tests
```

OpenGL deprecation warnings on macOS are expected and come from the ImGui backend, not from this project.