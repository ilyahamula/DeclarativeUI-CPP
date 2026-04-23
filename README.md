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

- **Declarative widget tree** — compose layouts using `VStack`, `HStack`, `VGroupBox`, `HGroupBox`, and `Dialog`
- **CRTP widget hierarchy** — `Widget<T>` base with fluent `.withFlags()`, `.withSize()`, `.withPosition()`, `.withStyle()` modifiers
- **Flexible layout system** — `LayoutFlags` with `Expand()`, `Proportion()`, `Border()`, `CenterVertical()`, `Center()`
- **Two-way data binding** — widgets accept either owned values or external references that stay in sync
- **Event callbacks** — `.onClick()`, `.onChange()`, `.onHover()` on supported widgets
- **Multi-backend** — compile against ImGui, wxWidgets, or Qt with `USE_IMGUI` / `USE_WX` / `USE_QT`

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

Select your backend via CMake:

```sh
cmake -DUSE_IMGUI=ON ..
cmake --build .
```

Replace `USE_IMGUI` with `USE_WX` or `USE_QT` as needed.

The default build directory is `build/`. OpenGL deprecation warnings on macOS are expected and come from the ImGui backend, not from this project.