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

- **Declarative widget tree** — compose layouts using `VStack`, `HStack`, `VGroupBox`, `HGroupBox`, `TabPanel`, and `Dialog`
- **Framework-owned layout engine** — one shared measure/arrange engine computes every rectangle; backends only measure native widgets and place them, so the same tree follows identical layout rules on every backend (see `docs/specs/custom_layout_system/`)
- **Sensible defaults, no flags required** — widgets size to their content (text fields never collapse below their text), sibling group boxes in a column equalize to the widest one (tallest in a row), and dialogs auto-fit their content
- **Flexible layout flags** — `LayoutFlags` with `Expand()`, `Proportion()`, `Border()`, `CenterVertical()`, `Center()`, `MinSize()`/`MaxSize()`, `SizeGroup()` (equalize across parents), and `AutoGrow()` (field re-measures as you type)
- **Dialog sizing policy** — dialogs are not user-resizable by default; opt in with `Dialog::Resizable()`, where the auto-fit size becomes the initial *and minimum* size so content can never be clipped
- **CRTP widget hierarchy** — `Widget<T>` base with fluent `.withFlags()`, `.withSize()`, `.withPosition()`, `.withStyle()` modifiers
- **Two-way data binding** — widgets accept either owned values or external references that stay in sync
- **Event callbacks** — `.onClick()`, `.onChange()`, `.onHover()` on supported widgets
- **Multi-backend** — compile against ImGui, wxWidgets, or Qt by switching one CMake variable

## Widget Catalogue

| Category       | Widgets |
|----------------|---------|
| Text           | `StaticText`, `ReadonlyTextCtrl`, `ClickableText`, `LinkText` |
| Input          | `TextCtrl`, `PasswordInput`, `MultiLineTextCtrl` |
| Numeric        | `SpinBox<T>`, `Slider<T>` |
| Selection      | `Button`, `ToggleButton`, `CheckBox`, `RadioButton<T>`, `ComboBox<T>` |
| Date & Time    | `DatePicker`, `TimePicker` |
| Media          | `Image` |

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