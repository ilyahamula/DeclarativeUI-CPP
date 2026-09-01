#include "frameworks_core/qt/LayoutBackend.hpp"

#include "frameworks_core/LayoutNode.hpp"
#include "frameworks_core/qt/RefSync.hpp"

#include <QFontMetrics>
#include <QGroupBox>
#include <QLineEdit>
#include <QPlainTextEdit>
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

Size QtLayoutBackend::measure(const LayoutNode& leaf, const Constraints&)
{
	ControlWrapper* widget = leaf.widget;
	if (widget->nativeHandle() == nullptr)
	{
		widget->realize(m_host); // create the native widget + connect signals
		applyDisabled(static_cast<QWidget*>(widget->nativeHandle()), leaf);
		applyTooltip(static_cast<QWidget*>(widget->nativeHandle()), *widget);
	}

	auto* window = static_cast<QWidget*>(widget->nativeHandle());
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
	return {};
}

bool QtLayoutBackend::beginContainer(const LayoutNode& node, const Rect& frame)
{
	Scope scope { &node, currentParent(), m_stack.back().originX, m_stack.back().originY };
	const bool isTabPage = m_stack.back().node != nullptr
		&& m_stack.back().node->kind == NodeKind::TabPanel;

	if (node.kind == NodeKind::GroupBox || node.kind == NodeKind::TabPanel)
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
