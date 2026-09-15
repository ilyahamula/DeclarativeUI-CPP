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
| Lists & tables    | `ListBox<T>`, `TreeView<T>`, `Table<T>` |
| Numeric           | `SpinBox<T>`, `Slider<T>` |
| Pickers           | `DatePicker`, `TimePicker`, `ColorPicker` |
| Display           | `ProgressBar`, `Separator` (horizontal or vertical), `Image` |
| Layout            | `Spacer` |
| Containers        | `VStack` / `HStack`, `Grid`, `ScrollPanel`, `HSplitter` / `VSplitter`, `Expander`, `VGroupBox` / `HGroupBox`, `TabPanel` + `Tab` |
| Top-level         | `Dialog`, `Window`, `MessageBox` |
| Application chrome| `MenuBar` + `Menu` + `MenuItem` (on a `Window`) |

`ListBox`, `TreeView` and `Table` all take `.withVisibleRows(n)`, which drives their
intrinsic height identically on every backend — the native hints disagree far too much
for the same tree to lay out the same way otherwise. `TreeView` addresses items by path
(`"src/engine"`) rather than index; `Table` columns are individually sortable and
editable, and rows keep their original index so a binding survives sorting.

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