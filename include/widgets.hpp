#pragma once

#include "frameworks_core/ControlWrappers.hpp"
#include "frameworks_core/CoreTypes/BoundValue.hpp"
#include "frameworks_core/LayoutNode.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

template <typename W>
struct Widget
{
	Widget() = default;

	virtual ~Widget() = default;

	// Emit a leaf node owning the wrapper — no rendering here; the backend
	// realizes/draws it during the layout pass. pre/postCreate fire around
	// wrapper creation during tree build.
	std::unique_ptr<LayoutNode> buildNode()
	{
		if (m_preCreateCallback)
			m_preCreateCallback();

		auto wrapper = createWrapper(m_position, m_size, m_style);
		// Copying carries the binding over when there is one and the snapshot
		// otherwise, so the wrapper never ends up referring to this widget --
		// a temporary that dies with the enclosing declarative expression.
		wrapper->setDisabled(m_disabled);
		wrapper->setTooltip(m_tooltip);
		auto node = makeLeaf(std::move(wrapper), m_flags.value_or(LayoutFlags{}));

		if (m_postCreateCallback)
			m_postCreateCallback();
		if (m_postCreateWithWidgetCallback)
			m_postCreateWithWidgetCallback(node->widget->nativeHandle());
		return node;
	}

	W& preCreate(std::function<void()> preCreateCallback)
	{
		m_preCreateCallback = std::move(preCreateCallback);
		return static_cast<W&>(*this);
	}

	W& postCreate(std::function<void()> postCreateCallback)
	{
		m_postCreateCallback = std::move(postCreateCallback);
		return static_cast<W&>(*this);
	}

	W& postCreate(std::function<void(void*)> postCreateCallback)
	{
		m_postCreateWithWidgetCallback = std::move(postCreateCallback);
		return static_cast<W&>(*this);
	}

	W& withFlags(LayoutFlags flags)
	{
		m_flags = flags;
		return static_cast<W&>(*this);
	}

	W& withPosition(const Position& pos)
	{
		m_position = pos;
		return static_cast<W&>(*this);
	}

	W& withSize(const Size& size)
	{
		m_size = size;
		return static_cast<W&>(*this);
	}

	// Snapshot the flag as it stands now.
	W& isDisabled(const bool& disabled = true)
	{
		m_disabled.snapshot(disabled);
		return static_cast<W&>(*this);
	}

	// Bind to a caller-owned flag: flipping it enables/disables the control
	// without rebuilding the tree. A non-const lvalue selects this overload.
	W& isDisabled(bool& disabled)
	{
		m_disabled.bind(disabled);
		return static_cast<W&>(*this);
	}

	// Hover text for the control. Empty clears it. Snapshot the text as it
	// stands now -- a string literal lands here, as it cannot bind.
	W& withTooltip(const std::string& tooltip)
	{
		m_tooltip.snapshot(tooltip);
		return static_cast<W&>(*this);
	}

	// Bind to a caller-owned string: rewriting it retargets the tooltip without
	// rebuilding the tree. A non-const lvalue selects this overload.
	W& withTooltip(std::string& tooltip)
	{
		m_tooltip.bind(tooltip);
		return static_cast<W&>(*this);
	}

	W& withStyle(long style)
	{
		m_style = style;
		return static_cast<W&>(*this);
	}

private:
	virtual std::unique_ptr<ControlWrapper> createWrapper(
		const Position& pos,
		const Size& size,
		long style) = 0;

private: // callbacks
	std::function<void()> m_preCreateCallback;
	std::function<void()> m_postCreateCallback;
	std::function<void(void*)> m_postCreateWithWidgetCallback;

private:
	DisabledFlag m_disabled;
	TooltipText m_tooltip;
	std::optional<LayoutFlags> m_flags;
	Position m_position { -1, -1 };
	Size m_size { -1, -1 };
	long m_style { 0 };
};

// StaticText -----------------------------------------------------------
struct StaticText : Widget<StaticText>
{
	using super = Widget<StaticText>;

	explicit StaticText(const std::string& text)
		: super()
		, m_text(text)
	{
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<StaticTextWrapper>(m_text, pos, size, style);
	}

private:
	std::string m_text;
};

// TextCtrl -----------------------------------------------------------
struct TextCtrl : Widget<TextCtrl>
{
	using super = Widget<TextCtrl>;

	TextCtrl()
		: super()
	{
	}

	// This constructor pair is the binding contract, and every value widget below
	// repeats it: a non-const lvalue binds (edits write through to the caller's
	// variable), anything else -- a literal, a temporary, a const -- is snapshotted.
	// BoundValue's own constructors mirror the same overload pair, which is why the
	// two bodies are identical.
	explicit TextCtrl(const std::string& text)
		: super()
		, m_value(text)
	{
	}

	explicit TextCtrl(std::string& text)
		: super()
		, m_value(text)
	{
	}

	TextCtrl& onChange(std::function<void(const std::string&)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

	TextCtrl& onChange(std::function<void(const std::string&, void*)> callback)
	{
		m_onChangeWithWidget = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<TextCtrlWrapper>(m_value, pos, size, style, m_onChange, m_onChangeWithWidget);
	}

private:
	BoundValue<std::string> m_value;
	std::function<void(const std::string&)> m_onChange;
	std::function<void(const std::string&, void*)> m_onChangeWithWidget;
};

// PasswordInput -----------------------------------------------------------
struct PasswordInput : Widget<PasswordInput>
{
	using super = Widget<PasswordInput>;

	PasswordInput()
		: super()
	{
	}

	explicit PasswordInput(const std::string& text)
		: super()
		, m_value(text)
	{
	}

	explicit PasswordInput(std::string& text)
		: super()
		, m_value(text)
	{
	}

	PasswordInput& onChange(std::function<void(const std::string&)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

	PasswordInput& onChange(std::function<void(const std::string&, void*)> callback)
	{
		m_onChangeWithWidget = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<PasswordInputWrapper>(m_value, pos, size, style, m_onChange, m_onChangeWithWidget);
	}

private:
	BoundValue<std::string> m_value;
	std::function<void(const std::string&)> m_onChange;
	std::function<void(const std::string&, void*)> m_onChangeWithWidget;
};

// MultiLineTextCtrl -----------------------------------------------------------
struct MultiLineTextCtrl : Widget<MultiLineTextCtrl>
{
	using super = Widget<MultiLineTextCtrl>;

	MultiLineTextCtrl()
		: super()
	{
	}

	explicit MultiLineTextCtrl(const std::string& text)
		: super()
		, m_value(text)
	{
	}

	explicit MultiLineTextCtrl(std::string& text)
		: super()
		, m_value(text)
	{
	}

	MultiLineTextCtrl& onChange(std::function<void(const std::string&)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

	MultiLineTextCtrl& onChange(std::function<void(const std::string&, void*)> callback)
	{
		m_onChangeWithWidget = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<MultiLineTextCtrlWrapper>(m_value, pos, size, style, m_onChange, m_onChangeWithWidget);
	}

private:
	BoundValue<std::string> m_value;
	std::function<void(const std::string&)> m_onChange;
	std::function<void(const std::string&, void*)> m_onChangeWithWidget;
};

// ReadonlyTextCtrl -----------------------------------------------------------
struct ReadonlyTextCtrl : Widget<ReadonlyTextCtrl>
{
	using super = Widget<ReadonlyTextCtrl>;

	explicit ReadonlyTextCtrl(const std::string& text)
		: super()
		, m_text(text)
	{
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<ReadonlyTextCtrlWrapper>(m_text, pos, size, style);
	}

private:
	const std::string& m_text;
};

// ClickableText -----------------------------------------------------------
struct ClickableText : Widget<ClickableText>
{
	using super = Widget<ClickableText>;

	explicit ClickableText(const std::string& text)
		: super()
		, m_text(text)
	{
	}

	ClickableText& onClick(std::function<void()> callback)
	{
		m_onClick = std::move(callback);
		return *this;
	}

	ClickableText& onClick(std::function<void(void*)> callback)
	{
		m_onClickWithWidget = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<ClickableTextWrapper>(m_text, pos, size, style, m_onClick, m_onClickWithWidget);
	}

private:
	std::function<void()> m_onClick;
	std::function<void(void*)> m_onClickWithWidget;
	std::string m_text;
};

// LinkText -----------------------------------------------------------
struct LinkText : Widget<LinkText>
{
	using super = Widget<LinkText>;

	explicit LinkText(const std::string& text)
		: super()
		, m_text(text)
	{
	}

	LinkText& onClick(std::function<void()> callback)
	{
		m_onClick = std::move(callback);
		return *this;
	}

	LinkText& onClick(std::function<void(void*)> callback)
	{
		m_onClickWithWidget = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<LinkTextWrapper>(m_text, pos, size, style, m_onClick, m_onClickWithWidget);
	}

private:
	std::function<void()> m_onClick;
	std::function<void(void*)> m_onClickWithWidget;
	std::string m_text;
};

// Button -----------------------------------------------------------
struct Button : Widget<Button>
{
	using super = Widget<Button>;

	explicit Button(const std::string& str = "")
		: super()
		, m_btnTitle(str)
	{
	}

	Button& onClick(std::function<void()> callback)
	{
		m_onClick = std::move(callback);
		return *this;
	}

	Button& onClick(std::function<void(void*)> callback)
	{
		m_onClickWithWidget = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<ButtonWrapper>(m_btnTitle, pos, size, style, m_onClick, m_onClickWithWidget);
	}

private:
	std::function<void()> m_onClick;
	std::function<void(void*)> m_onClickWithWidget;
	std::string m_btnTitle;
};

// RadioButton -----------------------------------------------------------
template <RadioButtonValue T>
struct RadioButton : Widget<RadioButton<T>>
{
	using super = Widget<RadioButton<T>>;

	RadioButton()
		: super()
	{
	}

	RadioButton(const T& value, const std::string& label = "")
		: super()
		, m_value(value)
		, m_label(label)
	{
	}

	RadioButton(T& value, const std::string& label = "")
		: super()
		, m_value(value)
		, m_label(label)
	{
	}

	RadioButton& onChange(std::function<void(T)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

	RadioButton& onChange(std::function<void(T, void*)> callback)
	{
		m_onChangeWithWidget = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<RadioButtonWrapper<T>>(m_label, m_value, pos, size, style, m_onChange, m_onChangeWithWidget);
	}

private:
	BoundValue<T> m_value;
	std::string m_label;
	std::function<void(T)> m_onChange;
	std::function<void(T, void*)> m_onChangeWithWidget;
};

template <RadioButtonValue T>
RadioButton(T&) -> RadioButton<T>;

template <RadioButtonValue T>
RadioButton(T&, const std::string&) -> RadioButton<T>;

template <RadioButtonValue T>
RadioButton(const T&) -> RadioButton<T>;

template <RadioButtonValue T>
RadioButton(const T&, const std::string&) -> RadioButton<T>;

// CheckBox -----------------------------------------------------------
struct CheckBox : Widget<CheckBox>
{
	using super = Widget<CheckBox>;

	CheckBox()
		: super()
	{
	}

	CheckBox(const bool& initialChecked, const std::string& label = "")
		: super()
		, m_value(initialChecked)
		, m_label(label)
	{
	}

	CheckBox(bool& checked, const std::string& label = "")
		: super()
		, m_value(checked)
		, m_label(label)
	{
	}

	CheckBox& onChange(std::function<void(bool)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

	CheckBox& onChange(std::function<void(bool, void*)> callback)
	{
		m_onChangeWithWidget = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<CheckBoxWrapper>(m_label, pos, size, style, m_value, m_onChange, m_onChangeWithWidget);
	}

private:
	BoundValue<bool> m_value;
	std::string m_label;
	std::function<void(bool)> m_onChange;
	std::function<void(bool, void*)> m_onChangeWithWidget;
};

// ComboBox -----------------------------------------------------------
template <ComboBoxValue T>
struct ComboBox : Widget<ComboBox<T>>
{
	using super = Widget<ComboBox<T>>;

	ComboBox(std::vector<std::string> choices)
		: super()
		, m_choices(std::move(choices))
	{
		if constexpr (std::is_same_v<T, std::string>)
			m_value.snapshot(m_choices.empty() ? T{} : m_choices.front());
	}

	ComboBox(std::vector<std::string> choices, const T& selected)
		: super()
		, m_choices(std::move(choices))
		, m_value(selected)
	{
	}

	ComboBox(std::vector<std::string> choices, T& selected)
		: super()
		, m_choices(std::move(choices))
		, m_value(selected)
	{
	}

	ComboBox& onChange(std::function<void(const T&)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

	ComboBox& onChange(std::function<void(const T&, void*)> callback)
	{
		m_onChangeWithWidget = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<ComboBoxWrapper<T>>(m_choices, m_value, pos, size, style, m_onChange, m_onChangeWithWidget);
	}

private:
	std::vector<std::string> m_choices;
	BoundValue<T> m_value;
	std::function<void(const T&)> m_onChange;
	std::function<void(const T&, void*)> m_onChangeWithWidget;
};

template <ComboBoxValue T>
ComboBox(std::vector<std::string>, T&) -> ComboBox<T>;

template <ComboBoxValue T>
ComboBox(std::vector<std::string>, const T&) -> ComboBox<T>;

// ListBox -----------------------------------------------------------
// Scrollable list of selectable items. The bound type picks the mode:
// int/std::string select one item, std::vector<int>/std::vector<std::string>
// select many. Index bindings address items by position, string bindings by
// item text -- exactly like ComboBox.
template <ListBoxValue T>
struct ListBox : Widget<ListBox<T>>
{
	using super = Widget<ListBox<T>>;

	// Rows shown before the list scrolls. It drives the intrinsic height on all
	// three backends: their native hints disagree far too much (wx sizes to the
	// item count, Qt returns a fixed ~192px, ImGui has no hint at all) for the
	// same tree to lay out identically otherwise.
	static constexpr int kDefaultVisibleRows = 6;

	explicit ListBox(std::vector<std::string> items)
		: super()
		, m_items(std::move(items))
	{
		// Single-select starts on the first item (as ComboBox does); multi-select
		// starts empty -- "no rows selected" is the honest default for a list.
		if constexpr (std::is_same_v<T, std::string>)
			m_value.snapshot(m_items.empty() ? T{} : m_items.front());
	}

	ListBox(std::vector<std::string> items, const T& selected)
		: super()
		, m_items(std::move(items))
		, m_value(selected)
	{
	}

	ListBox(std::vector<std::string> items, T& selected)
		: super()
		, m_items(std::move(items))
		, m_value(selected)
	{
	}

	ListBox& withVisibleRows(int rows)
	{
		m_visibleRows = rows > 0 ? rows : 1;
		return *this;
	}

	ListBox& onChange(std::function<void(const T&)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

	ListBox& onChange(std::function<void(const T&, void*)> callback)
	{
		m_onChangeWithWidget = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<ListBoxWrapper<T>>(m_items, m_value, m_visibleRows, pos, size, style, m_onChange, m_onChangeWithWidget);
	}

private:
	std::vector<std::string> m_items;
	int m_visibleRows = kDefaultVisibleRows;
	BoundValue<T> m_value;
	std::function<void(const T&)> m_onChange;
	std::function<void(const T&, void*)> m_onChangeWithWidget;
};

template <ListBoxValue T>
ListBox(std::vector<std::string>, T&) -> ListBox<T>;

template <ListBoxValue T>
ListBox(std::vector<std::string>, const T&) -> ListBox<T>;

// TreeView -----------------------------------------------------------
// Hierarchical, collapsible list. Items are a nested TreeItem literal and the
// selection is an item's PATH -- its labels from the root joined by '/' --
// because an index says nothing about where an item sits in a tree:
//
//   TreeView { { {"Fruits", { {"Apple"}, {"Banana"} }, true} }, picked }
//
// Single-select by default, on every binding. isMultiSelect() widens it and is
// offered only on the std::vector<std::string> binding, which is the one with
// somewhere to put the extra paths.
template <TreeViewValue T>
struct TreeView : Widget<TreeView<T>>
{
	using super = Widget<TreeView<T>>;

	// Rows shown before the tree scrolls. It drives the intrinsic height on all
	// three backends for the reason ListBox needs the same knob: wxTreeCtrl
	// reports its client area, QTreeWidget a fixed ~192px and ImGui nothing at
	// all, so the same tree would not lay out identically otherwise. Deeper
	// than ListBox's default because a tree spends rows on its categories.
	static constexpr int kDefaultVisibleRows = 8;

	explicit TreeView(std::vector<TreeItem> items)
		: super()
		, m_items(std::move(items))
	{
		// No default selection, unlike ListBox: a tree's first item is usually a
		// category rather than a choice, so "nothing selected" is the honest
		// starting state.
	}

	TreeView(std::vector<TreeItem> items, const T& selected)
		: super()
		, m_items(std::move(items))
		, m_value(selected)
	{
	}

	TreeView(std::vector<TreeItem> items, T& selected)
		: super()
		, m_items(std::move(items))
		, m_value(selected)
	{
	}

	TreeView& withVisibleRows(int rows)
	{
		m_visibleRows = rows > 0 ? rows : 1;
		return *this;
	}

	// Constrained to the vector binding: a single-path binding has nowhere to
	// put a second selection, so asking for multi-select there is a compile
	// error rather than a control that silently drops all but one item.
	TreeView& isMultiSelect(bool multiSelect = true)
		requires MultiSelectTreeViewValue<T>
	{
		m_multiSelect = multiSelect;
		return *this;
	}

	TreeView& onChange(std::function<void(const T&)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

	TreeView& onChange(std::function<void(const T&, void*)> callback)
	{
		m_onChangeWithWidget = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<TreeViewWrapper<T>>(m_items, m_value, m_visibleRows, m_multiSelect, pos, size, style, m_onChange, m_onChangeWithWidget);
	}

private:
	std::vector<TreeItem> m_items;
	int m_visibleRows = kDefaultVisibleRows;
	bool m_multiSelect = false;
	BoundValue<T> m_value;
	std::function<void(const T&)> m_onChange;
	std::function<void(const T&, void*)> m_onChangeWithWidget;
};

// An unbound tree reports its selection through onChange only, so the single
// path binding is the one that costs nothing to default to.
TreeView(std::vector<TreeItem>) -> TreeView<std::string>;

template <TreeViewValue T>
TreeView(std::vector<TreeItem>, T&) -> TreeView<T>;

template <TreeViewValue T>
TreeView(std::vector<TreeItem>, const T&) -> TreeView<T>;

// Table -----------------------------------------------------------
// Multi-column tabular display, with optional per-column sorting and per-column
// cell editing. Columns are a TableColumn list and rows a rectangular block of
// text, so a table is described the same way on every backend:
//
//   Table { { {"File", -1, true}, {"Size", 80, true}, {"Note", -1, false, true} },
//           rows, selectedRow }
//
// Selection binds either an original ROW INDEX (int / std::vector<int>) or the
// text of the first column (std::string / std::vector<std::string>), and -- as
// with ListBox -- the bound type is what picks single- vs multi-select. Index
// bindings address the row's position in `rows`, never its position on screen,
// so they survive sorting; see TableValue in Concepts.hpp for when to prefer
// which.
//
// Passing `rows` as a non-const lvalue BINDS them, which is what makes an
// editable column write back to the caller's data. A const or temporary rows
// argument is a snapshot instead, and edits then reach the caller only through
// onCellChange() -- on ImGui they do not even survive to the next frame, since
// the tree is rebuilt from the caller's data every time.
template <TableValue T>
struct Table : Widget<Table<T>>
{
	using super = Widget<Table<T>>;

	// Rows shown before the table scrolls. It drives the intrinsic height on all
	// three backends for the reason ListBox and TreeView need the same knob:
	// their native hints disagree far too much (wx sizes to its content, Qt
	// returns a fixed ~192px and ImGui has no hint at all) for the same table to
	// lay out identically otherwise.
	static constexpr int kDefaultVisibleRows = 8;

	Table(std::vector<TableColumn> columns, const TableRows& rows)
		: super()
		, m_columns(std::move(columns))
		, m_rows(rows)
	{
		clearSelection();
	}

	Table(std::vector<TableColumn> columns, TableRows& rows)
		: super()
		, m_columns(std::move(columns))
		, m_rows(rows)
	{
		clearSelection();
	}

	Table(std::vector<TableColumn> columns, const TableRows& rows, const T& selected)
		: super()
		, m_columns(std::move(columns))
		, m_rows(rows)
		, m_value(selected)
	{
	}

	Table(std::vector<TableColumn> columns, const TableRows& rows, T& selected)
		: super()
		, m_columns(std::move(columns))
		, m_rows(rows)
		, m_value(selected)
	{
	}

	Table(std::vector<TableColumn> columns, TableRows& rows, const T& selected)
		: super()
		, m_columns(std::move(columns))
		, m_rows(rows)
		, m_value(selected)
	{
	}

	Table(std::vector<TableColumn> columns, TableRows& rows, T& selected)
		: super()
		, m_columns(std::move(columns))
		, m_rows(rows)
		, m_value(selected)
	{
	}

	// The same six again without a column list, for the callers who describe
	// their columns with addColumn() below. Passing an empty vector would do the
	// same job, but `Table { {}, rows, picked }` puts a meaningless brace pair at
	// the front of every such table.
	explicit Table(const TableRows& rows)
		: super()
		, m_rows(rows)
	{
		clearSelection();
	}

	explicit Table(TableRows& rows)
		: super()
		, m_rows(rows)
	{
		clearSelection();
	}

	Table(const TableRows& rows, const T& selected)
		: super()
		, m_rows(rows)
		, m_value(selected)
	{
	}

	Table(const TableRows& rows, T& selected)
		: super()
		, m_rows(rows)
		, m_value(selected)
	{
	}

	Table(TableRows& rows, const T& selected)
		: super()
		, m_rows(rows)
		, m_value(selected)
	{
	}

	Table(TableRows& rows, T& selected)
		: super()
		, m_rows(rows)
		, m_value(selected)
	{
	}

	// Append one column. Both overloads exist because a column is described two
	// ways in practice: a TableColumn when the caller already holds one (or
	// wants to name the fields), and the unpacked form when the interesting part
	// is the label and everything else is a default.
	//
	// Appending, not replacing: a column list passed to the constructor and
	// these calls compose, so a shared base set can be extended per table.
	Table& addColumn(TableColumn column)
	{
		m_columns.push_back(std::move(column));
		return *this;
	}

	// Parameters and defaults mirror TableColumn field for field, so the two
	// overloads cannot drift apart in what an omitted argument means.
	//
	// The label is a constrained template rather than a plain std::string to
	// keep addColumn({"File"}) unambiguous. Spelled that way it could init
	// either overload's first parameter, and a braced list next to the
	// addColumn({"File", 70, true}) that DOES work is too easy to write for the
	// answer to be a compile error. A braced-init-list is a non-deduced context,
	// so this candidate drops out and the TableColumn one takes it -- which is
	// what the caller meant.
	template <typename Label>
		requires std::constructible_from<std::string, Label>
	Table& addColumn(Label&& label, int width = -1, bool sortable = false, bool editable = false)
	{
		m_columns.push_back(TableColumn { std::string(std::forward<Label>(label)), width, sortable, editable });
		return *this;
	}

	Table& withVisibleRows(int rows)
	{
		m_visibleRows = rows > 0 ? rows : 1;
		return *this;
	}

	Table& onChange(std::function<void(const T&)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

	Table& onChange(std::function<void(const T&, void*)> callback)
	{
		m_onChangeWithWidget = std::move(callback);
		return *this;
	}

	// Fired after an editable column's cell is committed, with the row's
	// ORIGINAL index, the column and the new text -- the same order the value
	// binding uses, so a handler reading `rows` already sees the edit.
	Table& onCellChange(std::function<void(int, int, const std::string&)> callback)
	{
		m_onCellChange = std::move(callback);
		return *this;
	}

private:
	// An unbound table starts with nothing selected. Spelled -1 rather than left
	// default-constructed because a default-constructed int is 0, which is a
	// perfectly good row: -1 is what valueFor() reports for "no selection", so
	// this is the same state a user reaches by clicking nothing.
	void clearSelection()
	{
		if constexpr (std::is_same_v<T, int>)
			m_value.snapshot(-1);
	}

	std::unique_ptr<ControlWrapper> createWrapper(
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<TableWrapper<T>>(m_columns, m_rows, m_value, m_visibleRows, pos, size, style, m_onChange, m_onChangeWithWidget, m_onCellChange);
	}

private:
	std::vector<TableColumn> m_columns;
	BoundValue<TableRows> m_rows;
	int m_visibleRows = kDefaultVisibleRows;
	BoundValue<T> m_value;
	std::function<void(const T&)> m_onChange;
	std::function<void(const T&, void*)> m_onChangeWithWidget;
	std::function<void(int, int, const std::string&)> m_onCellChange;
};

// An unbound table reports its selection through onChange only, so the row
// index binding is the one that costs nothing to default to.
Table(std::vector<TableColumn>, const TableRows&) -> Table<int>;
Table(std::vector<TableColumn>, TableRows&) -> Table<int>;
Table(const TableRows&) -> Table<int>;
Table(TableRows&) -> Table<int>;

template <TableValue T>
Table(const TableRows&, T&) -> Table<T>;

template <TableValue T>
Table(const TableRows&, const T&) -> Table<T>;

template <TableValue T>
Table(TableRows&, T&) -> Table<T>;

template <TableValue T>
Table(TableRows&, const T&) -> Table<T>;

template <TableValue T>
Table(std::vector<TableColumn>, const TableRows&, T&) -> Table<T>;

template <TableValue T>
Table(std::vector<TableColumn>, const TableRows&, const T&) -> Table<T>;

template <TableValue T>
Table(std::vector<TableColumn>, TableRows&, T&) -> Table<T>;

template <TableValue T>
Table(std::vector<TableColumn>, TableRows&, const T&) -> Table<T>;

// Slider -----------------------------------------------------------
template <SliderValue T>
struct Slider : Widget<Slider<T>>
{
	using super = Widget<Slider<T>>;

	explicit Slider(Range<T> range)
		: super()
		, m_range(range)
		, m_value(std::as_const(range).min)
	{
	}

	Slider(Range<T> range, const T& value)
		: super()
		, m_range(range)
		, m_value(value)
	{
	}

	Slider(Range<T> range, T& value)
		: super()
		, m_range(range)
		, m_value(value)
	{
	}

	Slider& onChange(std::function<void(T)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

	Slider& onChange(std::function<void(T, void*)> callback)
	{
		m_onChangeWithWidget = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<SliderWrapper<T>>(m_range, m_value, pos, size, style, m_onChange, m_onChangeWithWidget);
	}

private:
	Range<T> m_range;
	BoundValue<T> m_value;
	std::function<void(T)> m_onChange;
	std::function<void(T, void*)> m_onChangeWithWidget;
};

template <SliderValue T>
Slider(Range<T>) -> Slider<T>;

template <SliderValue T>
Slider(Range<T>, T&) -> Slider<T>;

template <SliderValue T>
Slider(Range<T>, const T&) -> Slider<T>;

// SpinBox -----------------------------------------------------------
template <SpinBoxValue T>
struct SpinBox : Widget<SpinBox<T>>
{
	using super = Widget<SpinBox<T>>;

	explicit SpinBox(Range<T> range)
		: super()
		, m_range(range)
		, m_value(std::as_const(range).min)
	{
	}

	SpinBox(Range<T> range, const T& value)
		: super()
		, m_range(range)
		, m_value(value)
	{
	}

	SpinBox(Range<T> range, T& value)
		: super()
		, m_range(range)
		, m_value(value)
	{
	}

	SpinBox& onChange(std::function<void(T)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

	SpinBox& onChange(std::function<void(T, void*)> callback)
	{
		m_onChangeWithWidget = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<SpinBoxWrapper<T>>(m_range, m_value, pos, size, style, m_onChange, m_onChangeWithWidget);
	}

private:
	Range<T> m_range;
	BoundValue<T> m_value;
	std::function<void(T)> m_onChange;
	std::function<void(T, void*)> m_onChangeWithWidget;
};

template <SpinBoxValue T>
SpinBox(Range<T>) -> SpinBox<T>;

template <SpinBoxValue T>
SpinBox(Range<T>, T&) -> SpinBox<T>;

template <SpinBoxValue T>
SpinBox(Range<T>, const T&) -> SpinBox<T>;

// DatePicker -----------------------------------------------------------
struct DatePicker : Widget<DatePicker>
{
	using super = Widget<DatePicker>;

	DatePicker()
		: super()
	{
	}

	explicit DatePicker(const Date& value)
		: super()
		, m_value(value)
	{
	}

	explicit DatePicker(Date& value)
		: super()
		, m_value(value)
	{
	}

	DatePicker& onChange(std::function<void(const Date&)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

	DatePicker& onChange(std::function<void(const Date&, void*)> callback)
	{
		m_onChangeWithWidget = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<DatePickerWrapper>(m_value, pos, size, style, m_onChange, m_onChangeWithWidget);
	}

private:
	BoundValue<Date> m_value;
	std::function<void(const Date&)> m_onChange;
	std::function<void(const Date&, void*)> m_onChangeWithWidget;
};

// ToggleButton -----------------------------------------------------------
struct ToggleButton : Widget<ToggleButton>
{
	using super = Widget<ToggleButton>;

	ToggleButton()
		: super()
	{
	}

	ToggleButton(const bool& initialToggled, const std::string& label = "")
		: super()
		, m_value(initialToggled)
		, m_label(label)
	{
	}

	ToggleButton(bool& toggled, const std::string& label = "")
		: super()
		, m_value(toggled)
		, m_label(label)
	{
	}

	ToggleButton& onChange(std::function<void(bool)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

	ToggleButton& onChange(std::function<void(bool, void*)> callback)
	{
		m_onChangeWithWidget = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<ToggleButtonWrapper>(m_label, m_value, pos, size, style, m_onChange, m_onChangeWithWidget);
	}

private:
	std::string m_label;
	BoundValue<bool> m_value;
	std::function<void(bool)> m_onChange;
	std::function<void(bool, void*)> m_onChangeWithWidget;
};

// ColorPicker -----------------------------------------------------------
struct ColorPicker : Widget<ColorPicker>
{
	using super = Widget<ColorPicker>;

	explicit ColorPicker(Color& value)
		: super()
		, m_value(value)
	{
	}

	explicit ColorPicker(const Color& value)
		: super()
		, m_value(value)
	{
	}

	ColorPicker& onChange(std::function<void(const Color&)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

	ColorPicker& onChange(std::function<void(const Color&, void*)> callback)
	{
		m_onChangeWithWidget = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<ColorPickerWrapper>(m_value, pos, size, style, m_onChange, m_onChangeWithWidget);
	}

private:
	BoundValue<Color> m_value;
	std::function<void(const Color&)> m_onChange;
	std::function<void(const Color&, void*)> m_onChangeWithWidget;
};

// Separator -----------------------------------------------------------
struct Separator : Widget<Separator>
{
	using super = Widget<Separator>;

	Separator() : super() {}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<SeparatorWrapper>(pos, size, style);
	}
};

// ProgressBar -----------------------------------------------------------
struct ProgressBar : Widget<ProgressBar>
{
	using super = Widget<ProgressBar>;

	explicit ProgressBar(const float& value)
		: super()
		, m_value(value)
	{
	}

	explicit ProgressBar(float& value)
		: super()
		, m_value(value)
	{
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<ProgressBarWrapper>(m_value, pos, size, style);
	}

private:
	BoundValue<float> m_value;
};

// Image -----------------------------------------------------------
struct Image : Widget<Image>
{
	using super = Widget<Image>;

	explicit Image(const std::string& filePath)
		: super()
		, m_filePath(filePath)
	{
	}

	Image& onClick(std::function<void()> callback)
	{
		m_onClick = std::move(callback);
		return *this;
	}

	Image& onClick(std::function<void(void*)> callback)
	{
		m_onClickWithWidget = std::move(callback);
		return *this;
	}

	Image& onHover(std::function<void()> callback)
	{
		m_onHover = std::move(callback);
		return *this;
	}

	Image& onHover(std::function<void(void*)> callback)
	{
		m_onHoverWithWidget = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<ImageWrapper>(m_filePath, pos, size, style, m_onClick, m_onClickWithWidget, m_onHover, m_onHoverWithWidget);
	}

private:
	std::string m_filePath;
	std::function<void()> m_onClick;
	std::function<void()> m_onHover;
	std::function<void(void*)> m_onClickWithWidget;
	std::function<void(void*)> m_onHoverWithWidget;
};

// TimePicker -----------------------------------------------------------
struct TimePicker : Widget<TimePicker>
{
	using super = Widget<TimePicker>;

	TimePicker()
		: super()
	{
	}

	explicit TimePicker(const Time& value)
		: super()
		, m_value(value)
	{
	}

	explicit TimePicker(Time& value)
		: super()
		, m_value(value)
	{
	}

	TimePicker& onChange(std::function<void(const Time&)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

	TimePicker& onChange(std::function<void(const Time&, void*)> callback)
	{
		m_onChangeWithWidget = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<TimePickerWrapper>(m_value, pos, size, style, m_onChange, m_onChangeWithWidget);
	}

private:
	BoundValue<Time> m_value;
	std::function<void(const Time&)> m_onChange;
	std::function<void(const Time&, void*)> m_onChangeWithWidget;
};
