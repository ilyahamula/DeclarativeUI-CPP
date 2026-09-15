#include "frameworks_core/qt/LayoutBackend.hpp"
#include "frameworks_core/qt/MenuBuilder.hpp"

#include "frameworks_core/LayoutNode.hpp"
#include "frameworks_core/qt/RefSync.hpp"

#include <QFontMetrics>
#include <QGroupBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QScrollArea>
#include <QStyle>
#include <QTabWidget>

#include <algorithm>

namespace
{

// Floor for editable fields when empty: N average characters wide.
constexpr int kTextFloorChars = 10;
// Allowance for the text frame's own padding around the text extent.
constexpr int kTextFramePadding = 16;

// Qt treats '&' as a mnemonic marker in titles; user text must escape it.
QString labelText(const std::string& label)
{
	QString text = QString::fromStdString(label);
	text.replace(QLatin1String("&"), QLatin1String("&&"));
	return text;
}

// Applies a node's effective disabled state to the widget that was just
// created for it (leaf control, group box, tab widget or page), and keeps
// polling when some flag in the chain is caller-owned. Hooked at the one place
// each widget passes through exactly once, rather than repeated in realize().
void applyDisabled(QWidget* window, const LayoutNode& node)
{
	window->setEnabled(!node.isDisabledEffective());

	// A bound flag can flip at any time and Qt applies the state only here, at
	// creation -- so poll it like any other external ref. A chain that is
	// fixed, or holds no bound flag at all, can never change again.
	const auto sources = node.disabledSources();
	if (sources.fixed || sources.refs.empty())
		return;

	bindExternalRefSync(window,
		[window] { return window->isEnabled(); },
		[refs = sources.refs] {
			return std::none_of(refs.begin(), refs.end(),
				[](const bool* flag) { return *flag; });
		},
		[window](bool enable) { window->setEnabled(enable); });
}

// Applies a leaf's tooltip to the widget just created for it, and keeps polling
// when the text is caller-owned. No disabled check is needed: Qt does not
// deliver tooltip events to a disabled widget -- the wx twin relies on the same
// native behaviour, and the ImGui backend reproduces it by hand.
void applyTooltip(QWidget* window, const ControlWrapper& widget)
{
	auto push = [window](const std::string& text) {
		window->setToolTip(QString::fromStdString(text)); // empty clears it
	};
	push(widget.tooltip());

	// The string is caller-owned and outlives every widget, so it may be
	// captured; the wrapper it came from must never be.
	const std::string* bound = widget.boundTooltip();
	if (bound == nullptr)
		return; // a snapshot cannot change behind us

	bindExternalRefSync(window,
		[window] { return window->toolTip().toStdString(); },
		[bound] { return *bound; },
		push);
}

// Gives a leaf its right-click menu -- the Qt twin of the wx applyContextMenu,
// same contract. The model is COPIED into the handler: the wrapper that supplied
// it must never be captured, and the copy is what an unbound check item's state
// then lives in. A bound one writes through to the caller's bool.
//
// Nothing is polled. The menu is rebuilt from the model every time it opens, so
// bound check and disabled flags are read at that moment.
//
// No disabled check is needed here: Qt sends no context-menu request to a
// disabled widget, exactly as it sends no tooltip event.
void applyContextMenu(QWidget* window, const ControlWrapper& widget)
{
	if (widget.contextMenu().empty())
		return;

	window->setContextMenuPolicy(Qt::CustomContextMenu);
	QObject::connect(window, &QWidget::customContextMenuRequested, window,
		[window, items = widget.contextMenu()](const QPoint& pos) mutable {
			popupContextMenu(window, items, pos);
		});
}

// The content half of an Expander: the child that folds away, as opposed to
// the header leaf beside it. Read from the node's own parent rather than from
// the scope stack, so it holds wherever the subtree is entered from.
bool isExpanderContent(const LayoutNode& node)
{
	return node.parent != nullptr && node.parent->kind == NodeKind::Expander;
}

} // unnamed namespace

QtLayoutBackend::QtLayoutBackend(QWidget* host)
	: m_host(host)
{
	m_stack.push_back({ nullptr, host, 0, 0 });
}

Rect QtLayoutBackend::toLocal(const Rect& frame) const
{
	const Scope& scope = m_stack.back();
	return { frame.x - scope.originX, frame.y - scope.originY, frame.width, frame.height };
}

Size QtLayoutBackend::measure(const LayoutNode& leaf, const Constraints& c)
{
	ControlWrapper* widget = leaf.widget;
	if (widget->nativeHandle() == nullptr)
	{
		widget->realize(m_host); // create the native widget + connect signals
		// A windowless leaf (Spacer) creates nothing: there is no widget to
		// carry the disabled state or the tooltip, and none to hang their
		// polling timers on. Its realize() runs again on the next pass, which
		// is why creating nothing has to stay idempotent.
		if (auto* created = static_cast<QWidget*>(widget->nativeHandle()))
		{
			applyDisabled(created, leaf);
			applyTooltip(created, *widget);
			applyContextMenu(created, *widget);
		}
	}

	auto* window = static_cast<QWidget*>(widget->nativeHandle());
	// Windowless: the wrapper's own measurement is the whole story. It already
	// applies the explicit withSize() overrides, so nothing below is missed --
	// the leaf is pure geometry the engine positions and Qt never draws.
	if (window == nullptr)
		return widget->measureContent(c);

	const QSize hint = window->sizeHint();
	Size size { hint.width(), hint.height() };

	// content-based floor for text fields (initial content unless AutoGrow)
	QString textValue;
	bool isText = false;
	bool isMultiLine = false;
	if (auto* line = qobject_cast<QLineEdit*>(window))
	{
		isText = true;
		textValue = line->text();
	}
	else if (auto* edit = qobject_cast<QPlainTextEdit*>(window))
	{
		isText = true;
		isMultiLine = true;
		textValue = edit->toPlainText().section(QLatin1Char('\n'), 0, 0);
	}
	if (isText)
	{
		auto cached = m_textFloorWidths.find(&leaf);
		if (leaf.flags.autoGrow() || cached == m_textFloorWidths.end())
		{
			const QString probe = textValue.isEmpty()
				? QString(kTextFloorChars, QLatin1Char('M'))
				: textValue;
			const int floor = window->fontMetrics().horizontalAdvance(probe) + kTextFramePadding;
			cached = m_textFloorWidths.insert_or_assign(&leaf, floor).first;
		}
		size.width = std::max(size.width, cached->second);
		if (isMultiLine)
			size.height = std::max(size.height,
				window->fontMetrics().height() * 4 + kTextFramePadding);
	}

	// explicit withSize() dimensions override the intrinsic size per axis
	const Size& explicitSize = widget->explicitSize();
	if (explicitSize.width > 0)
		size.width = explicitSize.width;
	if (explicitSize.height > 0)
		size.height = explicitSize.height;
	return size;
}

void QtLayoutBackend::place(const LayoutNode& leaf, const Rect& frame)
{
	auto* window = static_cast<QWidget*>(leaf.widget->nativeHandle());
	if (window == nullptr)
		return;
	if (window->parentWidget() != currentParent())
	{
		window->setParent(currentParent()); // setParent hides the widget
		window->show();
	}
	const Rect local = toLocal(frame);
	window->setGeometry(local.x, local.y, local.width, local.height);
}

QWidget* QtLayoutBackend::ensureContainer(const LayoutNode& node)
{
	if (const auto it = m_containers.find(&node); it != m_containers.end())
		return it->second;

	// created against the host first; beginContainer reparents as needed
	QWidget* window = nullptr;
	if (node.kind == NodeKind::GroupBox)
		window = new QGroupBox(labelText(node.label), m_host);
	else if (node.kind == NodeKind::TabPanel)
		window = new QTabWidget(m_host);
	else if (isExpanderContent(node))
	{
		// A collapsed section has to take its whole subtree out of view, and a
		// widget scope is the one handle that does it in a single call however
		// deep that subtree runs -- the same trick a tab page relies on.
		window = new QWidget(m_host);
	}
	else if (node.kind == NodeKind::ScrollPanel)
	{
		auto* area = new QScrollArea(m_host);
		area->setFrameShape(QFrame::NoFrame);
		// The inner widget is sized by the engine, not by Qt: it IS the virtual
		// rect, and the children placed into it are what scrolls.
		area->setWidgetResizable(false);
		area->setWidget(new QWidget(area));
		window = area;
	}
	if (window != nullptr)
		applyDisabled(window, node);
	m_containers[&node] = window;
	return window;
}

EdgeInsets QtLayoutBackend::containerInsets(const LayoutNode& node)
{
	if (node.kind == NodeKind::GroupBox)
	{
		auto* box = ensureContainer(node);
		// title row on top, thin frame around; QGroupBox has no exact
		// metrics API without a layout, so derive from the font
		const int side = 8;
		return { side, side, box->fontMetrics().height() + 6, side };
	}
	if (node.kind == NodeKind::TabPanel)
	{
		auto* tabs = ensureContainer(node);
		// tab bar on top; deterministic estimate (the real bar reports a
		// useful height only after pages exist)
		return { 2, 2, tabs->fontMetrics().height() + 14, 2 };
	}
	if (node.kind == NodeKind::ScrollPanel)
	{
		// The scrollbar gutter, reserved on the cross side of each scrolling
		// axis and reserved ALWAYS: containerInsets runs before the content is
		// measured, so "only when it overflows" could read no more than the
		// previous pass and would let a panel near the boundary oscillate.
		QWidget* area = ensureContainer(node);
		const int bar = area->style()->pixelMetric(QStyle::PM_ScrollBarExtent, nullptr, area);
		return {
			0,
			scrollsVertically(node.scroll) ? bar : 0,
			0,
			scrollsHorizontally(node.scroll) ? bar : 0,
		};
	}
	return {};
}

bool QtLayoutBackend::beginContainer(const LayoutNode& node, const Rect& frame)
{
	Scope scope { &node, currentParent(), m_stack.back().originX, m_stack.back().originY };
	const bool isTabPage = m_stack.back().node != nullptr
		&& m_stack.back().node->kind == NodeKind::TabPanel;

	if (isExpanderContent(node))
	{
		// The panel becomes the parent and coordinate origin of the section's
		// subtree, exactly as a tab page does -- so hiding it hides everything
		// in it, whatever was realized while the section was open.
		QWidget* panel = ensureContainer(node);
		if (panel->parentWidget() != scope.parent)
			panel->setParent(scope.parent); // setParent hides the widget
		const bool expanded = node.parent->expander.applied;
		panel->setVisible(expanded);
		if (!expanded)
			return false; // collapsed: engine skips the subtree, as for an inactive tab page

		const Rect local = toLocal(frame);
		panel->setGeometry(local.x, local.y, local.width, local.height);
		scope.parent = panel;
		scope.originX = frame.x;
		scope.originY = frame.y;
		m_stack.push_back(scope);
		return true;
	}

	if (node.kind == NodeKind::GroupBox || node.kind == NodeKind::TabPanel
		|| node.kind == NodeKind::ScrollPanel)
	{
		QWidget* window = ensureContainer(node);
		if (window->parentWidget() != scope.parent)
		{
			window->setParent(scope.parent);
			window->show();
		}
		const Rect local = toLocal(frame);
		window->setGeometry(local.x, local.y, local.width, local.height);
		if (node.kind == NodeKind::GroupBox)
			window->lower(); // chrome stays behind its sibling content

		if (node.kind == NodeKind::ScrollPanel)
		{
			// The inner widget becomes the parent and coordinate origin of the
			// subtree, exactly as a tab page does; QScrollArea scrolls by moving
			// that widget, so the children never need the scroll offset applied
			// to them. Its size is the content's arranged frame -- the engine
			// already sized that to the full extent there is to scroll over.
			auto* area = static_cast<QScrollArea*>(window);
			area->setVerticalScrollBarPolicy(scrollsVertically(node.scroll)
				? Qt::ScrollBarAlwaysOn : Qt::ScrollBarAlwaysOff);
			area->setHorizontalScrollBarPolicy(scrollsHorizontally(node.scroll)
				? Qt::ScrollBarAlwaysOn : Qt::ScrollBarAlwaysOff);

			QWidget* inner = area->widget();
			const Rect virt = node.children.empty() ? Rect{} : node.children.front()->frame;
			inner->resize(virt.width, virt.height);
			inner->show();
			scope.parent = inner;
			scope.originX = frame.x;
			scope.originY = frame.y;
		}
	}
	else if (isTabPage)
	{
		// a page: a plain QWidget added to the QTabWidget becomes the parent
		// and coordinate origin of the page's subtree; the tab widget
		// manages which page is visible, so every page is still traversed
		auto* tabs = static_cast<QTabWidget*>(m_containers[m_stack.back().node]);
		QWidget* page = nullptr;
		if (const auto it = m_containers.find(&node); it != m_containers.end())
		{
			page = it->second;
		}
		else
		{
			page = new QWidget(tabs);
			tabs->addTab(page, labelText(node.label));
			applyDisabled(page, node);
			m_containers[&node] = page;
		}
		scope.parent = page;
		scope.originX = frame.x;
		scope.originY = frame.y;
	}

	m_stack.push_back(scope);
	return true;
}

void QtLayoutBackend::endContainer(const LayoutNode&)
{
	m_stack.pop_back();
}
