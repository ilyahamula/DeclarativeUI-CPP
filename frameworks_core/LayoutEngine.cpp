#include "frameworks_core/LayoutEngine.hpp"

#include "frameworks_core/ILayoutBackend.hpp"
#include "frameworks_core/LayoutNode.hpp"

#include <algorithm>
#include <unordered_map>
#include <vector>

namespace
{

// Clamp a measured size to the node's optional Min/MaxSize flags.
Size clampToFlags(Size size, const LayoutNode& node)
{
	if (const auto minSize = node.flags.minSize())
	{
		size.width = std::max(size.width, minSize->width);
		size.height = std::max(size.height, minSize->height);
	}
	if (const auto maxSize = node.flags.maxSize())
	{
		size.width = std::min(size.width, maxSize->width);
		size.height = std::min(size.height, maxSize->height);
	}
	return size;
}

// Constraints offered to a child: the parent's, reduced by the child's own
// margins (which the parent will spend around it).
Constraints childConstraints(const Constraints& c, const LayoutNode& child)
{
	const EdgeInsets margin = child.flags.border();
	return Constraints {
		std::max(0, c.maxWidth - (margin.left + margin.right)),
		std::max(0, c.maxHeight - (margin.top + margin.bottom)),
	};
}

// Effectively-unbounded constraint for the free axis of an auto-fit measure;
// kept far from INT_MAX so downstream sums cannot overflow.
constexpr int kUnbounded = 1 << 24;

int mainFloorOf(const LayoutNode& node, bool horizontal)
{
	if (const auto minSize = node.flags.minSize())
		return horizontal ? minSize->width : minSize->height;
	return 0;
}

// Distributes positive leftover across Proportion(>0) children (flex-grow),
// exact to the pixel: the last weighted child absorbs rounding residue.
void growProportioned(std::vector<int>& mains, const LayoutNode& box, int leftover)
{
	int totalWeight = 0;
	int lastWeighted = -1;
	for (size_t i = 0; i < box.children.size(); ++i)
	{
		if (box.children[i]->flags.proportion() > 0)
		{
			totalWeight += box.children[i]->flags.proportion();
			lastWeighted = (int)i;
		}
	}
	if (totalWeight == 0)
		return;

	int given = 0;
	for (size_t i = 0; i < box.children.size(); ++i)
	{
		const int weight = box.children[i]->flags.proportion();
		if (weight <= 0)
			continue;
		const int share = (int)i == lastWeighted
			? leftover - given
			: (int)((long long)leftover * weight / totalWeight);
		mains[i] += share;
		given += share;
	}
}

// The negative-leftover shrink order: proportioned children first (by
// weight, down to their MinSize floors), then everyone in reverse
// declaration order; any residue overflows the parent and is clipped by
// the backend.
void shrinkToFit(std::vector<int>& mains, const LayoutNode& box, int deficit, bool horizontal)
{
	const size_t n = box.children.size();

	while (deficit > 0)
	{
		int totalWeight = 0;
		for (size_t i = 0; i < n; ++i)
		{
			const LayoutNode& child = *box.children[i];
			if (child.flags.proportion() > 0 && mains[i] > mainFloorOf(child, horizontal))
				totalWeight += child.flags.proportion();
		}
		if (totalWeight == 0)
			break;

		const int deficitBefore = deficit;
		for (size_t i = 0; i < n && deficit > 0; ++i)
		{
			const LayoutNode& child = *box.children[i];
			const int weight = child.flags.proportion();
			const int floor = mainFloorOf(child, horizontal);
			if (weight <= 0 || mains[i] <= floor)
				continue;
			const int share = std::max(1, (int)((long long)deficitBefore * weight / totalWeight));
			const int take = std::min({ share, mains[i] - floor, deficit });
			mains[i] -= take;
			deficit -= take;
		}
	}

	for (int i = (int)n - 1; i >= 0 && deficit > 0; --i)
	{
		const int floor = mainFloorOf(*box.children[i], horizontal);
		const int take = std::min(mains[i] - floor, deficit);
		if (take > 0)
		{
			mains[i] -= take;
			deficit -= take;
		}
	}
}

// Box content extent computed from the children's already-filled `desired`
// (pure arithmetic — no backend calls), so the SizeGroup re-sum can rerun it.
Size boxContentExtent(const LayoutNode& node)
{
	const bool horizontal = node.orientation == Orientation::Horizontal;

	int main = 0;
	int cross = 0;
	const LayoutNode* prev = nullptr;
	for (const auto& child : node.children)
	{
		const EdgeInsets margin = child->flags.border();
		const int childMain = horizontal ? child->desired.width : child->desired.height;
		const int childCross = horizontal ? child->desired.height : child->desired.width;
		const int crossMargins = horizontal ? margin.top + margin.bottom
											: margin.left + margin.right;

		if (prev != nullptr)
			main += LayoutEngine::gapBetween(*prev, *child, node.orientation);
		main += childMain;
		cross = std::max(cross, childCross + crossMargins);
		prev = child.get();
	}

	if (!node.children.empty())
	{
		const EdgeInsets first = node.children.front()->flags.border();
		const EdgeInsets last = node.children.back()->flags.border();
		main += horizontal ? first.left + last.right
						   : first.top + last.bottom;
	}

	return horizontal ? Size { main, cross } : Size { cross, main };
}

// The column/row bands of a grid, derived purely from the children's already
// filled `desired` -- so arrange can rerun it after the SizeGroup re-sum without
// touching the backend, exactly as boxContentExtent is rerun for a Box.
//
// A cell's Border is BOTH its padding inside its band and the source of the
// gutter beside it (requirements.md R2.1): the band is the widest cell plus its
// own margins, and the line between two bands is `max` of the facing margins,
// falling back to kDefaultGap. That is deliberately more generous than a Box,
// where a margin only ever becomes the gap -- in a grid, a margin has to hold
// the cell off its own band edge as well, since neighbouring rows may set
// different ones.
struct GridBands
{
	std::vector<int> colWidths;
	std::vector<int> rowHeights;
	std::vector<int> colGaps;  // colGaps[c] follows column c
	std::vector<int> rowGaps;  // rowGaps[r] follows row r
};

int gridColumns(const LayoutNode& node)
{
	return std::max(1, node.columns);
}

int gridRows(const LayoutNode& node)
{
	const int cols = gridColumns(node);
	return ((int)node.children.size() + cols - 1) / cols; // ragged last row allowed
}

// Cell at (row, col), or nullptr when a ragged last row stops short.
const LayoutNode* gridCell(const LayoutNode& node, int row, int col)
{
	const size_t index = (size_t)row * gridColumns(node) + (size_t)col;
	return index < node.children.size() ? node.children[index].get() : nullptr;
}

GridBands gridBands(const LayoutNode& node)
{
	const int cols = gridColumns(node);
	const int rows = gridRows(node);

	GridBands bands;
	bands.colWidths.assign((size_t)cols, 0);
	bands.rowHeights.assign((size_t)rows, 0);
	bands.colGaps.assign((size_t)std::max(0, cols - 1), 0);
	bands.rowGaps.assign((size_t)std::max(0, rows - 1), 0);

	for (int r = 0; r < rows; ++r)
	{
		for (int c = 0; c < cols; ++c)
		{
			const LayoutNode* cell = gridCell(node, r, c);
			if (cell == nullptr)
				continue;
			const EdgeInsets margin = cell->flags.border();
			bands.colWidths[(size_t)c] = std::max(bands.colWidths[(size_t)c],
				cell->desired.width + margin.left + margin.right);
			bands.rowHeights[(size_t)r] = std::max(bands.rowHeights[(size_t)r],
				cell->desired.height + margin.top + margin.bottom);
		}
	}

	// One gutter per grid line: the widest facing margin found anywhere along it
	// wins, so a column line stays straight down the whole grid. The kDefaultGap
	// fallback is applied ONCE, to that maximum -- folding it in per pair (as
	// gapBetween does for a Box) would let the default override a deliberately
	// tighter margin somewhere along the line.
	const auto lineGap = [](int widestMargin) {
		return widestMargin > 0 ? widestMargin : LayoutEngine::kDefaultGap;
	};
	for (int c = 0; c + 1 < cols; ++c)
	{
		int widest = 0;
		for (int r = 0; r < rows; ++r)
		{
			const LayoutNode* left = gridCell(node, r, c);
			const LayoutNode* right = gridCell(node, r, c + 1);
			if (left != nullptr && right != nullptr)
				widest = std::max(widest,
					std::max(left->flags.border().right, right->flags.border().left));
		}
		bands.colGaps[(size_t)c] = lineGap(widest);
	}
	for (int r = 0; r + 1 < rows; ++r)
	{
		int widest = 0;
		for (int c = 0; c < cols; ++c)
		{
			const LayoutNode* above = gridCell(node, r, c);
			const LayoutNode* below = gridCell(node, r + 1, c);
			if (above != nullptr && below != nullptr)
				widest = std::max(widest,
					std::max(above->flags.border().bottom, below->flags.border().top));
		}
		bands.rowGaps[(size_t)r] = lineGap(widest);
	}
	return bands;
}

int sumOf(const std::vector<int>& values)
{
	int total = 0;
	for (int value : values)
		total += value;
	return total;
}

Size gridContentExtent(const LayoutNode& node)
{
	if (node.children.empty())
		return Size { 0, 0 };
	const GridBands bands = gridBands(node);
	return Size {
		sumOf(bands.colWidths) + sumOf(bands.colGaps),
		sumOf(bands.rowHeights) + sumOf(bands.rowGaps),
	};
}

// Flex weight of each band: the largest Proportion any cell in it asked for.
// A cell weights the column it sits in and the row it sits in independently,
// which is what lets one Proportion(1) field widen its whole column.
std::vector<int> bandWeights(const LayoutNode& node, bool columns)
{
	const int cols = gridColumns(node);
	const int rows = gridRows(node);
	std::vector<int> weights((size_t)(columns ? cols : rows), 0);
	for (int r = 0; r < rows; ++r)
	{
		for (int c = 0; c < cols; ++c)
		{
			const LayoutNode* cell = gridCell(node, r, c);
			if (cell == nullptr)
				continue;
			int& weight = weights[(size_t)(columns ? c : r)];
			weight = std::max(weight, cell->flags.proportion());
		}
	}
	return weights;
}

// Positive leftover across weighted bands, exact to the pixel: the last
// weighted band absorbs the rounding residue, as growProportioned does for a Box.
void growBands(std::vector<int>& bands, const std::vector<int>& weights, int leftover)
{
	int totalWeight = 0;
	int lastWeighted = -1;
	for (size_t i = 0; i < weights.size(); ++i)
	{
		if (weights[i] > 0)
		{
			totalWeight += weights[i];
			lastWeighted = (int)i;
		}
	}
	if (totalWeight == 0)
		return; // no cell asked to flex: the grid keeps its measured size

	int given = 0;
	for (size_t i = 0; i < weights.size(); ++i)
	{
		if (weights[i] <= 0)
			continue;
		const int share = (int)i == lastWeighted
			? leftover - given
			: (int)((long long)leftover * weights[i] / totalWeight);
		bands[i] += share;
		given += share;
	}
}

// Resolve one axis of a cell inside its band. Stretch fills the band; the rest
// keep the measured extent and only move within it.
void applyBandAlign(Align align, int bandOrigin, int bandExtent, int& extent, int& pos)
{
	switch (align)
	{
	case Align::Stretch:
		extent = bandExtent;
		pos = bandOrigin;
		break;
	case Align::Center:
		pos = bandOrigin + (bandExtent - extent) / 2;
		break;
	case Align::End:
		pos = bandOrigin + (bandExtent - extent);
		break;
	case Align::Start:
		pos = bandOrigin;
		break;
	}
}

// The viewport a scrolling axis is capped to: the caller's MaxSize on that axis
// when they set a usable one, else the engine default. MaxSize is also applied
// to the whole node by clampToFlags afterwards -- capping here as well is
// deliberate and harmless (min is idempotent), and it keeps the rule readable
// in one place. A non-positive component reads as "not set on this axis", the
// same convention withSize() uses.
int scrollViewportCap(const LayoutNode& node, bool horizontal)
{
	if (const auto maxSize = node.flags.maxSize())
	{
		const int cap = horizontal ? maxSize->width : maxSize->height;
		if (cap > 0)
			return cap;
	}
	return LayoutEngine::kDefaultScrollViewport;
}

// A scroll panel is one child plus a decision per axis: a scrolling axis takes
// the viewport (capped), a non-scrolling one behaves exactly like a Box's.
Size scrollContentExtent(const LayoutNode& node)
{
	if (node.children.empty())
		return Size { 0, 0 };

	const LayoutNode& child = *node.children.front();
	const EdgeInsets margin = child.flags.border();
	Size content {
		child.desired.width + margin.left + margin.right,
		child.desired.height + margin.top + margin.bottom,
	};
	if (scrollsHorizontally(node.scroll))
		content.width = std::min(content.width, scrollViewportCap(node, true));
	if (scrollsVertically(node.scroll))
		content.height = std::min(content.height, scrollViewportCap(node, false));
	return content;
}

Size tabPanelContentExtent(const LayoutNode& node)
{
	Size desired { 0, 0 };
	for (const auto& child : node.children)
	{
		const EdgeInsets margin = child->flags.border();
		desired.width = std::max(desired.width, child->desired.width + margin.left + margin.right);
		desired.height = std::max(desired.height, child->desired.height + margin.top + margin.bottom);
	}
	return desired;
}

// Content extent plus the container's chrome (filled during measure).
Size containerExtent(const LayoutNode& node)
{
	Size content;
	switch (node.kind)
	{
	case NodeKind::TabPanel:    content = tabPanelContentExtent(node); break;
	case NodeKind::Grid:        content = gridContentExtent(node); break;
	case NodeKind::ScrollPanel: content = scrollContentExtent(node); break;
	default:                    content = boxContentExtent(node); break;
	}
	return Size {
		content.width + node.chrome.left + node.chrome.right,
		content.height + node.chrome.top + node.chrome.bottom,
	};
}

// A container's frame with its chrome removed: the area its children get.
Rect contentArea(const LayoutNode& node)
{
	return Rect {
		node.frame.x + node.chrome.left,
		node.frame.y + node.chrome.top,
		std::max(0, node.frame.width - node.chrome.left - node.chrome.right),
		std::max(0, node.frame.height - node.chrome.top - node.chrome.bottom),
	};
}

void collectSizeGroups(LayoutNode& node, std::unordered_map<int, std::vector<LayoutNode*>>& groups)
{
	if (const auto id = node.flags.sizeGroup())
		groups[*id].push_back(&node);
	for (const auto& child : node.children)
		collectSizeGroups(*child, groups);
}

// Post-order re-sum: recompute every container's desired from its children
// and raise SizeGroup members to their group maximum. Returns true if any
// desired changed (another pass may then be needed for nested groups).
bool resumDesired(LayoutNode& node, const std::unordered_map<int, Size>& groupMaxima)
{
	bool changed = false;
	for (const auto& child : node.children)
		changed |= resumDesired(*child, groupMaxima);

	Size fresh = node.isLeaf() ? node.desired : containerExtent(node);
	if (const auto id = node.flags.sizeGroup())
	{
		const Size& groupMax = groupMaxima.at(*id);
		fresh.width = std::max(fresh.width, groupMax.width);
		fresh.height = std::max(fresh.height, groupMax.height);
	}
	fresh = clampToFlags(fresh, node);

	if (!(fresh == node.desired))
	{
		node.desired = fresh;
		changed = true;
	}
	return changed;
}

} // unnamed namespace

LayoutEngine::LayoutEngine(ILayoutBackend& backend)
	: m_backend(backend)
{
}

int LayoutEngine::gapBetween(const LayoutNode& prev, const LayoutNode& next, Orientation orient)
{
	const EdgeInsets p = prev.flags.border();
	const EdgeInsets n = next.flags.border();
	const int gap = orient == Orientation::Horizontal
		? std::max(p.right, n.left)
		: std::max(p.bottom, n.top);
	return gap > 0 ? gap : kDefaultGap;
}

Size LayoutEngine::measure(LayoutNode& node, const Constraints& c)
{
	Size desired { 0, 0 };
	if (node.isLeaf())
	{
		desired = m_backend.measure(node, c);
	}
	else
	{
		// containers: chrome surrounds the content on top of any margins
		node.chrome = m_backend.containerInsets(node);
		const Constraints inner {
			std::max(0, c.maxWidth - node.chrome.left - node.chrome.right),
			std::max(0, c.maxHeight - node.chrome.top - node.chrome.bottom),
		};
		switch (node.kind)
		{
		case NodeKind::TabPanel:    measureTabPanel(node, inner); break;
		case NodeKind::Grid:        measureGrid(node, inner); break;
		case NodeKind::ScrollPanel: measureScrollPanel(node, inner); break;
		default:                    measureBox(node, inner); break;
		}
		desired = containerExtent(node);
	}

	node.desired = clampToFlags(desired, node);
	return node.desired;
}

Size LayoutEngine::measureBox(LayoutNode& node, const Constraints& c)
{
	for (const auto& child : node.children)
		measure(*child, childConstraints(c, *child));
	return boxContentExtent(node);
}

Size LayoutEngine::measureGrid(LayoutNode& node, const Constraints& c)
{
	// Cells are measured with the grid's own constraints minus their margins,
	// like a Box's children: a cell does not know its column's width until the
	// bands below are folded, and nothing here depends on that.
	for (const auto& child : node.children)
		measure(*child, childConstraints(c, *child));
	return gridContentExtent(node);
}

Size LayoutEngine::measureScrollPanel(LayoutNode& node, const Constraints& c)
{
	if (node.children.empty())
		return Size { 0, 0 };

	// The content is measured as if it had all the room it wants on every
	// scrolling axis -- that measurement IS the virtual extent the panel will
	// scroll over. A non-scrolling axis keeps the panel's own constraint, so
	// wrapping content still wraps to the available width.
	LayoutNode& child = *node.children.front();
	Constraints inner = childConstraints(c, child);
	if (scrollsHorizontally(node.scroll))
		inner.maxWidth = kUnbounded;
	if (scrollsVertically(node.scroll))
		inner.maxHeight = kUnbounded;
	measure(child, inner);

	return scrollContentExtent(node);
}

Size LayoutEngine::measureTabPanel(LayoutNode& node, const Constraints& c)
{
	// Pages overlap, so the panel needs the largest page on both axes.
	// The tab-bar chrome height is backend-specific and added by the
	// adapter when placing (T2.6+); the engine sizes content only.
	for (const auto& child : node.children)
		measure(*child, childConstraints(c, *child));
	return tabPanelContentExtent(node);
}

void LayoutEngine::arrange(LayoutNode& node, const Rect& area)
{
	node.frame = area;
	switch (node.kind)
	{
	case NodeKind::Leaf:
		break;
	case NodeKind::Box:
	case NodeKind::GroupBox:
		arrangeBox(node);
		break;
	case NodeKind::Grid:
		arrangeGrid(node);
		break;
	case NodeKind::ScrollPanel:
		arrangeScrollPanel(node);
		break;
	case NodeKind::TabPanel:
		arrangeTabPanel(node);
		break;
	}
}

void LayoutEngine::arrangeBox(LayoutNode& node)
{
	if (node.children.empty())
		return;

	const Rect area = contentArea(node);
	const bool horizontal = node.orientation == Orientation::Horizontal;
	const size_t n = node.children.size();

	// main-axis extents start from the measured sizes
	std::vector<int> mains(n);
	int mainsSum = 0;
	for (size_t i = 0; i < n; ++i)
	{
		const Size& d = node.children[i]->desired;
		mains[i] = horizontal ? d.width : d.height;
		mainsSum += mains[i];
	}

	std::vector<int> gaps(n); // gaps[i] follows child i
	int gapsSum = 0;
	for (size_t i = 0; i + 1 < n; ++i)
	{
		gaps[i] = gapBetween(*node.children[i], *node.children[i + 1], node.orientation);
		gapsSum += gaps[i];
	}

	const EdgeInsets firstMargin = node.children.front()->flags.border();
	const EdgeInsets lastMargin = node.children.back()->flags.border();
	const int leading = horizontal ? firstMargin.left : firstMargin.top;
	const int trailing = horizontal ? lastMargin.right : lastMargin.bottom;

	const int areaMain = horizontal ? area.width : area.height;
	const int leftover = areaMain - leading - trailing - gapsSum - mainsSum;
	if (leftover > 0)
		growProportioned(mains, node, leftover);
	else if (leftover < 0)
		shrinkToFit(mains, node, -leftover, horizontal);

	// place children along the main axis; cross axis per crossAlign
	int cursor = (horizontal ? area.x : area.y) + leading;
	const int areaCross = horizontal ? area.height : area.width;
	for (size_t i = 0; i < n; ++i)
	{
		LayoutNode& child = *node.children[i];
		const EdgeInsets margin = child.flags.border();
		const int crossLeading = horizontal ? margin.top : margin.left;
		const int crossTrailing = horizontal ? margin.bottom : margin.right;
		const int band = std::max(0, areaCross - crossLeading - crossTrailing);
		const int desiredCross = horizontal ? child.desired.height : child.desired.width;

		int crossExtent = desiredCross;
		int crossOffset = crossLeading;
		switch (child.flags.crossAlign(child.kind, node.orientation))
		{
		case Align::Stretch:
			crossExtent = band;
			break;
		case Align::Center:
			crossOffset = crossLeading + (band - desiredCross) / 2;
			break;
		case Align::End:
			crossOffset = crossLeading + (band - desiredCross);
			break;
		case Align::Start:
			break;
		}

		const Rect childArea = horizontal
			? Rect { cursor, area.y + crossOffset, mains[i], crossExtent }
			: Rect { area.x + crossOffset, cursor, crossExtent, mains[i] };
		arrange(child, childArea);

		cursor += mains[i] + (i + 1 < n ? gaps[i] : 0);
	}
}

void LayoutEngine::arrangeGrid(LayoutNode& node)
{
	if (node.children.empty())
		return;

	const Rect area = contentArea(node);
	const int cols = gridColumns(node);
	const int rows = gridRows(node);
	GridBands bands = gridBands(node);

	// Positive leftover goes to bands by the largest Proportion any cell in that
	// band asked for; a band no cell weighted keeps its measured size. Negative
	// leftover is not redistributed in v1 -- it overflows and the backend clips,
	// exactly as a Box's residue does once every floor is spent.
	const int widthLeftover = area.width - sumOf(bands.colWidths) - sumOf(bands.colGaps);
	if (widthLeftover > 0)
		growBands(bands.colWidths, bandWeights(node, true), widthLeftover);
	const int heightLeftover = area.height - sumOf(bands.rowHeights) - sumOf(bands.rowGaps);
	if (heightLeftover > 0)
		growBands(bands.rowHeights, bandWeights(node, false), heightLeftover);

	// Band origins, walked once so every cell in a column shares an x.
	std::vector<int> colX((size_t)cols, 0);
	int cursor = area.x;
	for (int c = 0; c < cols; ++c)
	{
		colX[(size_t)c] = cursor;
		cursor += bands.colWidths[(size_t)c] + (c + 1 < cols ? bands.colGaps[(size_t)c] : 0);
	}
	std::vector<int> rowY((size_t)rows, 0);
	cursor = area.y;
	for (int r = 0; r < rows; ++r)
	{
		rowY[(size_t)r] = cursor;
		cursor += bands.rowHeights[(size_t)r] + (r + 1 < rows ? bands.rowGaps[(size_t)r] : 0);
	}

	for (int r = 0; r < rows; ++r)
	{
		for (int c = 0; c < cols; ++c)
		{
			const size_t index = (size_t)r * (size_t)cols + (size_t)c;
			if (index >= node.children.size())
				continue; // ragged last row
			LayoutNode& cell = *node.children[index];

			// The band minus the cell's own margins is what the cell may use;
			// both axes then resolve independently through the same crossAlign
			// rules a Box applies to its cross axis. A row's cross axis is
			// vertical and a column's is horizontal, which is why the two calls
			// pass opposite orientations.
			const EdgeInsets margin = cell.flags.border();
			const int bandX = colX[(size_t)c] + margin.left;
			const int bandY = rowY[(size_t)r] + margin.top;
			const int bandW = std::max(0, bands.colWidths[(size_t)c] - margin.left - margin.right);
			const int bandH = std::max(0, bands.rowHeights[(size_t)r] - margin.top - margin.bottom);

			const Align hAlign = cell.flags.crossAlign(cell.kind, Orientation::Vertical);
			const Align vAlign = cell.flags.crossAlign(cell.kind, Orientation::Horizontal);

			int width = cell.desired.width;
			int x = bandX;
			applyBandAlign(hAlign, bandX, bandW, width, x);
			int height = cell.desired.height;
			int y = bandY;
			applyBandAlign(vAlign, bandY, bandH, height, y);

			arrange(cell, Rect { x, y, width, height });
		}
	}
}

void LayoutEngine::arrangeScrollPanel(LayoutNode& node)
{
	if (node.children.empty())
		return;

	// The content gets a VIRTUAL rect: the viewport on a non-scrolling axis
	// (R3.3 -- content fills it, so a short list still spans the width), and
	// max(viewport, desired) on a scrolling one, which is what there is to
	// scroll over. Its origin is the panel's content origin, so the whole
	// subtree is laid out in the panel's own space and the backend's scope
	// translates it (T1.4b). Until that scope exists the frames are simply
	// absolute, which is why a ScrollPanel currently draws like a Box.
	LayoutNode& child = *node.children.front();
	const Rect area = contentArea(node);
	const EdgeInsets margin = child.flags.border();
	const int viewportWidth = std::max(0, area.width - margin.left - margin.right);
	const int viewportHeight = std::max(0, area.height - margin.top - margin.bottom);

	const int width = scrollsHorizontally(node.scroll)
		? std::max(viewportWidth, child.desired.width)
		: viewportWidth;
	const int height = scrollsVertically(node.scroll)
		? std::max(viewportHeight, child.desired.height)
		: viewportHeight;

	arrange(child, Rect { area.x + margin.left, area.y + margin.top, width, height });
}

void LayoutEngine::arrangeTabPanel(LayoutNode& node)
{
	// Pages overlap: every page fills the panel's content area (minus its
	// own margins); the backend shows only the active one.
	const Rect area = contentArea(node);
	for (const auto& child : node.children)
	{
		const EdgeInsets margin = child->flags.border();
		const Rect pageArea {
			area.x + margin.left,
			area.y + margin.top,
			std::max(0, area.width - margin.left - margin.right),
			std::max(0, area.height - margin.top - margin.bottom),
		};
		arrange(*child, pageArea);
	}
}

void LayoutEngine::traverse(LayoutNode& node)
{
	if (node.isLeaf())
	{
		m_backend.place(node, node.frame);
		return;
	}
	if (!m_backend.beginContainer(node, node.frame))
		return; // hidden subtree (e.g. inactive tab page)
	for (const auto& child : node.children)
		traverse(*child);
	m_backend.endContainer(node);
}

Size LayoutEngine::resolve(LayoutNode& root, Size contentSize)
{
	const bool autoFit = contentSize.width <= 0 || contentSize.height <= 0;
	const EdgeInsets margin = root.flags.border();
	const int marginW = margin.left + margin.right;
	const int marginH = margin.top + margin.bottom;

	const Constraints c = autoFit
		? Constraints { std::max(0, m_maxAutoFitWidth - marginW), kUnbounded }
		: Constraints { std::max(0, contentSize.width - marginW),
						std::max(0, contentSize.height - marginH) };
	measure(root, c);
	reconcileSizeGroups(root);

	// The root's own margins live outside its desired size; the content
	// area adds them back, and the root is arranged inside them.
	return autoFit
		? Size { root.desired.width + marginW, root.desired.height + marginH }
		: contentSize;
}

void LayoutEngine::render(LayoutNode& root, Size resolvedSize)
{
	const EdgeInsets margin = root.flags.border();
	const Rect rootArea {
		margin.left,
		margin.top,
		std::max(0, resolvedSize.width - margin.left - margin.right),
		std::max(0, resolvedSize.height - margin.top - margin.bottom),
	};
	arrange(root, rootArea);
	traverse(root);
}

Size LayoutEngine::run(LayoutNode& root, Size contentSize)
{
	const Size resolved = resolve(root, contentSize);
	render(root, resolved);
	return resolved;
}

void LayoutEngine::reconcileSizeGroups(LayoutNode& root)
{
	std::unordered_map<int, std::vector<LayoutNode*>> groups;
	collectSizeGroups(root, groups);
	if (groups.empty())
		return;

	// Raising a member can grow a container that is itself a member of
	// another group, so iterate to a fixpoint. Sizes only ever grow toward
	// the (finite) group maxima, so this converges; the pass cap is a
	// safety net, not an expected exit.
	for (int pass = 0; pass < kMaxReconcilePasses; ++pass)
	{
		std::unordered_map<int, Size> maxima;
		for (const auto& [id, members] : groups)
		{
			Size groupMax { 0, 0 };
			for (const LayoutNode* member : members)
			{
				groupMax.width = std::max(groupMax.width, member->desired.width);
				groupMax.height = std::max(groupMax.height, member->desired.height);
			}
			maxima[id] = groupMax;
		}

		if (!resumDesired(root, maxima))
			break;
	}
}
