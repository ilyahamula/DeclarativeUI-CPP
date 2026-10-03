#pragma once

#include "widgets.hpp"
#include "stacks.hpp"
#include "foreach.hpp"
#include "radiogroup.hpp"
#include "observable.hpp"
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
#include "toast.hpp"
#include "show_action.hpp"
#include "ui_thread.hpp"

#include <type_traits>

static_assert(NodeBuildable<Button>);
static_assert(NodeBuildable<TextCtrl>);
static_assert(NodeBuildable<StaticText>);
static_assert(NodeBuildable<RichText>);
static_assert(NodeBuildable<Spacer>);
static_assert(NodeBuildable<Separator>);
static_assert(NodeBuildable<ToolBar>);
static_assert(NodeBuildable<StatusBar>);
static_assert(NodeBuildable<ReadonlyTextCtrl>);
// Display content binds like any value: a non-const lvalue follows the caller.
static_assert(std::is_constructible_v<StaticText, std::string&>);
static_assert(std::is_constructible_v<ReadonlyTextCtrl, std::string&>);
static_assert(std::is_constructible_v<ListBox<std::string>, ItemList&, std::string&>);
static_assert(std::is_constructible_v<ComboBox<int>, ItemList&, int&>);
static_assert(std::is_constructible_v<CheckListBox<std::vector<int>>, ItemList&, std::vector<int>&>);
static_assert(NodeBuildable<RadioButton<bool>>);
static_assert(NodeBuildable<RadioButton<int>>);
// An int radio names its own value and must be bound: the radios sharing one
// int are the group. The old order-based spelling, and a snapshot int that
// could never know about its siblings, must not compile.
static_assert(std::is_constructible_v<RadioButton<int>, int&, int, const char*>);
static_assert(!std::is_constructible_v<RadioButton<int>, int&, const char*>);
static_assert(!std::is_constructible_v<RadioButton<int>, const int&, int, const char*>);
static_assert(std::is_constructible_v<RadioButton<bool>, bool&, const char*>);
static_assert(std::is_constructible_v<RadioButton<bool>, const bool&, const char*>);
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
// A Button opens either top-level spelling on click -- and nothing that is not
// one: a node is shown IN a window, never AS one. The flag-less spelling keys a
// framework-owned flag by title, which is why both spellings can name one.
static_assert(TitledTopLevel<Dialog<VStack<Button>>>);
static_assert(TitledTopLevel<Window<VStack<Button>>>);
static_assert(ClickShowHost<Button, Dialog<VStack<Button>>>);
static_assert(ClickShowHost<Button, Window<VStack<Button>>>);
static_assert(!ClickShowHost<Button, VStack<Button>>);
static_assert(!ClickShowHost<CheckBox, Dialog<VStack<Button>>>);
// ShowAction() is a COMMAND: it fits every callback that takes nothing -- a
// menu item, a tool, a link -- and no callback that reports a value, whose
// window would depend on that value and so cannot be declared ahead of it.
static_assert(std::same_as<decltype(ShowAction(std::declval<Dialog<VStack<Button>>>())),
	std::function<void()>>);
static_assert(requires(MenuItem item) { item.onSelect(ShowAction(std::declval<Window<VStack<Button>>>())); });
static_assert(requires(ToolItem tool, bool& open) { tool.onClick(ShowAction(open, std::declval<Dialog<VStack<Button>>>())); });
static_assert(requires(LinkText link) { link.onClick(ShowAction(std::declval<Dialog<VStack<Button>>>())); });
template <typename T>
concept ChangeTakesAction = requires(T element, std::function<void()> action) {
	element.onChange(action);
};
template <typename T>
concept ResultTakesAction = requires(T element, std::function<void()> action) {
	element.onResult(action);
};
template <typename T>
concept LinkTakesAction = requires(T element, std::function<void()> action) {
	element.onLink(action);
};
static_assert(LinkReporter<RichText>);
static_assert(!LinkReporter<LinkText>);
static_assert(!LinkTakesAction<RichText>);
static_assert(!ChangeTakesAction<CheckBox>);
static_assert(!ChangeTakesAction<ListBox<std::string>>);
static_assert(!ResultTakesAction<FileDialog>);
static_assert(!ResultTakesAction<MessageBox>);
// withContextMenu() is leaf-only, like withTooltip: a container has no native
// window to deliver a right-click, so asking for one must not compile.
static_assert(ContextMenuHost<Button>);
static_assert(ContextMenuHost<RichText>);
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
// Enter: single-line fields report it, and one Button per window answers it.
static_assert(EnterHost<TextCtrl>);
static_assert(EnterHost<PasswordInput>);
static_assert(!EnterHost<MultiLineTextCtrl>);
static_assert(DialogKeyButton<Button>);
// SearchField: a leaf with the usual binding pair and a placeholder; its Enter
// is onSearch, not onEnter (it never hands Enter on to the default button).
static_assert(NodeBuildable<SearchField>);
static_assert(PlaceholderHost<SearchField>);
static_assert(!EnterHost<SearchField>);
static_assert(std::is_constructible_v<SearchField, std::string&>);
static_assert(std::is_constructible_v<SearchField, const std::string&>);
// Focus and validity: the three text fields only.
static_assert(FocusHost<TextCtrl>);
static_assert(FocusHost<PasswordInput>);
static_assert(FocusHost<MultiLineTextCtrl>);
static_assert(!FocusHost<Button>);
static_assert(!FocusHost<ReadonlyTextCtrl>);
static_assert(!DialogKeyButton<ToggleButton>);
static_assert(requires(Button b) { { b.withIcon(std::string{}) } -> std::same_as<Button&>; });
// withScaleMode() belongs to the one widget that owns pixels of its own, and
// withAlign() to the one that is nothing but text in a frame -- asking any
// other leaf for either must not compile.
static_assert(ScaleModeHost<Image>);
static_assert(!ScaleModeHost<StaticText>);
static_assert(TextAlignHost<StaticText>);
static_assert(!TextAlignHost<Image>);
static_assert(!TextAlignHost<Button>);
static_assert(!TextAlignHost<RichText>);
// A Toast is shown over the windows and leaves on its own: not a node and not
// a flag-showable window, and so neither something a Button can onClickShow()
// nor something with an open flag. MessageBox is the notice that waits for an answer instead.
static_assert(TimedNotice<Toast>);
static_assert(!FlagShowable<Toast>);
static_assert(!ClickShowHost<Button, Toast>);
static_assert(!TimedNotice<MessageBox>);

// postToUi() takes a plain command, like ShowAction(): anything a click handler
// could be, including a ShowAction itself.
static_assert(std::is_invocable_v<decltype(&postToUi), std::function<void()>>);

// isHidden() is everywhere a node is: on every widget and every container --
// except a single Tab, since wxNotebook cannot hide a page without removing it
// (hide the whole TabPanel instead).
template <typename T>
concept Hideable = requires(T element, bool& flag) {
	{ element.isHidden(flag) } -> std::same_as<T&>;
	{ element.isHidden(true) } -> std::same_as<T&>;
};
static_assert(Hideable<Button>);
static_assert(Hideable<StaticText>);
static_assert(Hideable<SearchField>);
// VirtualList: rows on demand; count and selection follow the binding pair.
static_assert(NodeBuildable<VirtualList>);
static_assert(Hideable<VirtualList>);
static_assert(std::is_constructible_v<VirtualList, int&, VirtualList::RowText, int&>);
static_assert(std::is_constructible_v<VirtualList, const int&, VirtualList::RowText>);
// Calendar: a month view on the usual Date binding pair.
static_assert(NodeBuildable<Calendar>);
static_assert(Hideable<Calendar>);
static_assert(std::is_constructible_v<Calendar, Date&>);
static_assert(std::is_constructible_v<Calendar, const Date&>);
static_assert(std::is_default_constructible_v<Calendar>);
// Spinner: a leaf with no value; its running flag binds.
static_assert(NodeBuildable<Spinner>);
static_assert(Hideable<Spinner>);
static_assert(std::is_same_v<decltype(std::declval<Spinner&>().isRunning(std::declval<bool&>())), Spinner&>);
static_assert(std::is_same_v<decltype(std::declval<Spinner&>().withImage(std::string{})), Spinner&>);
// EditableCombo: free text with suggestions; both text and items bind.
static_assert(NodeBuildable<EditableCombo>);
static_assert(Hideable<EditableCombo>);
static_assert(PlaceholderHost<EditableCombo>);
static_assert(std::is_constructible_v<EditableCombo, std::string&, ItemList&>);
static_assert(std::is_constructible_v<EditableCombo, const std::string&, const ItemList&>);
static_assert(Hideable<VStack<Button>>);
static_assert(Hideable<HStack<Button>>);
static_assert(Hideable<VGroupBox<Button>>);
static_assert(Hideable<Grid<Button>>);
static_assert(Hideable<ScrollPanel<VStack<Button>>>);
static_assert(Hideable<HSplitter<VStack<Button>, VStack<Button>>>);
static_assert(Hideable<Expander<VStack<Button>>>);
static_assert(Hideable<TabPanel<Tab<VStack<Button>>>>);
static_assert(!Hideable<Tab<VStack<Button>>>);

// VForEach / HForEach are containers like a stack, built from a vector.
namespace foreach_check
{
inline auto row = [](const std::string& s) { return StaticText{s}; };
inline auto rowWithIndex = [](const std::string& s, std::size_t) { return StaticText{s}; };
}
static_assert(NodeBuildable<VForEach<std::string, decltype(foreach_check::row)>>);
static_assert(NodeBuildable<HForEach<std::string, decltype(foreach_check::rowWithIndex)>>);
static_assert(Hideable<VForEach<std::string, decltype(foreach_check::row)>>);

// RadioGroup: a stack of RadioButton<int>s, so its index must be bound too.
static_assert(NodeBuildable<RadioGroup>);
static_assert(Hideable<RadioGroup>);
static_assert(std::is_constructible_v<RadioGroup, int&, std::vector<std::string>>);
static_assert(!std::is_constructible_v<RadioGroup, const int&, std::vector<std::string>>);

// Observable<T> binds wherever T& does -- and only as a binding: a const one
// has no T& to hand out, so it can never be mistaken for a snapshot.
static_assert(std::is_convertible_v<Observable<ItemList>&, ItemList&>);
static_assert(std::is_convertible_v<Observable<TableRows>&, TableRows&>);
static_assert(!std::is_convertible_v<const Observable<TableRows>&, TableRows&>);
static_assert(std::is_constructible_v<ListBox<std::string>, Observable<ItemList>&, std::string&>);

static_assert(TabContent<VStack<Button>>);
static_assert(IsTab<Tab<VStack<Button>>>);
static_assert(NodeBuildable<TabPanel<Tab<VStack<Button>>>>);