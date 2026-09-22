#pragma once

#include "widgets.hpp"
#include "stacks.hpp"
#include "grid.hpp"
#include "scrollpanel.hpp"
#include "splitter.hpp"
#include "expander.hpp"
#include "groupboxes.hpp"
#include "dialog.hpp"
#include "window.hpp"
#include "menus.hpp"
#include "tabpanel.hpp"
#include "messagebox.hpp"
#include "filedialog.hpp"

#include <type_traits>

static_assert(NodeBuildable<Button>);
static_assert(NodeBuildable<TextCtrl>);
static_assert(NodeBuildable<StaticText>);
static_assert(NodeBuildable<Spacer>);
static_assert(NodeBuildable<Separator>);
static_assert(NodeBuildable<ToolBar>);
static_assert(NodeBuildable<StatusBar>);
static_assert(NodeBuildable<ReadonlyTextCtrl>);
static_assert(NodeBuildable<ProgressBar>);
// Indeterminate() needs the valueless spelling: a busy bar has no number to show.
static_assert(std::is_default_constructible_v<ProgressBar>);
static_assert(NodeBuildable<FilePicker>);
static_assert(NodeBuildable<Image>);
static_assert(NodeBuildable<ListBox<int>>);
static_assert(NodeBuildable<ListBox<std::string>>);
static_assert(NodeBuildable<ListBox<std::vector<int>>>);
static_assert(NodeBuildable<ListBox<std::vector<std::string>>>);
static_assert(NodeBuildable<CheckListBox<std::vector<int>>>);
static_assert(NodeBuildable<CheckListBox<std::vector<std::string>>>);
// A checked SET is always a vector: there is no single-value CheckListBox
// spelling, so the two ListBox selection types must not compile here.
static_assert(CheckListValue<std::vector<int>>);
static_assert(CheckListValue<std::vector<std::string>>);
static_assert(!CheckListValue<int>);
static_assert(!CheckListValue<std::string>);
static_assert(NodeBuildable<TreeView<std::string>>);
static_assert(NodeBuildable<TreeView<std::vector<std::string>>>);
static_assert(NodeBuildable<Table<int>>);
static_assert(NodeBuildable<Table<std::string>>);
static_assert(NodeBuildable<Table<std::vector<int>>>);
static_assert(NodeBuildable<Table<std::vector<std::string>>>);
static_assert(NodeBuildable<Grid<StaticText, TextCtrl>>);
static_assert(NodeBuildable<ScrollPanel<VStack<Button>>>);
static_assert(NodeBuildable<Splitter<VStack<Button>, VStack<Button>>>);
static_assert(NodeBuildable<HSplitter<VStack<Button>, VStack<TextCtrl>>>);
static_assert(NodeBuildable<VSplitter<VStack<Button>, VStack<TextCtrl>>>);
static_assert(NodeBuildable<Expander<VStack<Button>>>);
static_assert(NodeBuildable<Expander<CheckBox>>);
static_assert(NodeBuildable<HStack<Button, TextCtrl>>);
static_assert(NodeBuildable<VStack<StaticText, Button>>);

static_assert(TopLevel<Dialog<VStack<Button>>>);
static_assert(TopLevel<Window<VStack<Button>>>);
// A lifecycle is not something the two top-level spellings disagree about: both
// can be shown against a caller-owned flag and both report their own closing.
static_assert(FlagShowable<Dialog<VStack<Button>>>);
static_assert(FlagShowable<Window<VStack<Button>>>);
static_assert(CloseObservable<Dialog<VStack<Button>>>);
static_assert(CloseObservable<Window<VStack<Button>>>);
// Modality is a Dialog's alone: a Window is the application frame, and a frame
// that refuses input to every other window is a dialog by another name.
static_assert(ModalWindow<Dialog<VStack<Button>>>);
static_assert(!ModalWindow<Window<VStack<Button>>>);
// A menu bar attaches to a Window and only a Window: wxMenuBar needs a wxFrame,
// which is the reason Window exists next to Dialog.
static_assert(MenuBarHost<Window<VStack<Button>>>);
static_assert(!MenuBarHost<Dialog<VStack<Button>>>);
// withContextMenu() is leaf-only, like withTooltip: a container has no native
// window to deliver a right-click, so asking for one must not compile.
static_assert(ContextMenuHost<Button>);
static_assert(ContextMenuHost<ToolBar>);
static_assert(ContextMenuHost<StatusBar>);
static_assert(ContextMenuHost<FilePicker>);
static_assert(ContextMenuHost<Table<int>>);
static_assert(ContextMenuHost<CheckListBox<std::vector<int>>>);
static_assert(!ContextMenuHost<VStack<Button>>);
static_assert(!ContextMenuHost<HStack<Button>>);
static_assert(!ContextMenuHost<TabPanel<Tab<VStack<Button>>>>);
// withPlaceholder() is single-line only: wxTextCtrl::SetHint does nothing on a
// wxTE_MULTILINE control on any wx port, so the modifier is declared on the two
// single-line fields and asking a multi-line one for it must not compile.
static_assert(PlaceholderHost<TextCtrl>);
static_assert(PlaceholderHost<PasswordInput>);
static_assert(!PlaceholderHost<MultiLineTextCtrl>);
static_assert(!PlaceholderHost<ReadonlyTextCtrl>);
// withScaleMode() belongs to the one widget that owns pixels of its own, and
// withAlign() to the one that is nothing but text in a frame -- asking any
// other leaf for either must not compile.
static_assert(ScaleModeHost<Image>);
static_assert(!ScaleModeHost<StaticText>);
static_assert(TextAlignHost<StaticText>);
static_assert(!TextAlignHost<Image>);
static_assert(!TextAlignHost<Button>);

static_assert(TabContent<VStack<Button>>);
static_assert(IsTab<Tab<VStack<Button>>>);
static_assert(NodeBuildable<TabPanel<Tab<VStack<Button>>>>);