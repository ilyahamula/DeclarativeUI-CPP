#pragma once

#include "frameworks_core/ILayoutBackend.hpp"
#include "frameworks_core/LayoutNode.hpp"

#include <cstddef>
#include <memory>
#include <vector>

// Children generated from caller data -- VForEach / HForEach (include/foreach.hpp).
//
// A container whose rows come from a vector cannot have them fixed at compile
// time the way a VStack's are, and on the retained backends the tree is built
// once: so the node carries a NodeSource, and the window's session asks it,
// every poll, whether the data moved since the rows were last built. On ImGui
// nothing asks -- the whole tree, rows included, is rebuilt every frame.

// What changed since the last build: every row (the count changed), or just
// the rows whose item differs.
struct SourceUpdate
{
	bool all = false;
	std::vector<std::size_t> rows;

	bool empty() const { return !all && rows.empty(); }
};

class NodeSource
{
public:
	virtual ~NodeSource() = default;

	// Compare the data with what the rows were last built from, adopt it, and
	// say which rows are now stale. An unbound source never changes.
	virtual SourceUpdate update() = 0;

	virtual std::size_t count() const = 0;

	// Row `index` of the adopted data, as a fresh node subtree.
	virtual std::unique_ptr<LayoutNode> buildRow(std::size_t index) = 0;
};

// Replace the stale rows of `node` with fresh ones. The old rows' native
// windows are handed to the backend to destroy BEFORE their nodes -- and the
// wrappers those nodes own -- go away.
inline void regenerateChildren(LayoutNode& node, const SourceUpdate& update, ILayoutBackend& backend)
{
	NodeSource& source = *node.source;
	if (update.all)
	{
		for (const auto& child : node.children)
			backend.forget(*child);
		node.children.clear();
		for (std::size_t i = 0; i < source.count(); ++i)
			node.add(source.buildRow(i));
		return;
	}
	for (const std::size_t i : update.rows)
	{
		backend.forget(*node.children[i]);
		auto fresh = source.buildRow(i);
		fresh->parent = &node;
		node.children[i] = std::move(fresh);
	}
}

// One pass of a retained session's poll: bring ForEach rows up to date with
// their data, and report whether anything the engine reads has changed since
// the last layout -- a sash dragged, a section opened, a node shown or hidden,
// rows regenerated. The session then re-measures once, however many changed.
//
// It compares the live value against what the last pass LATCHED (`requested`,
// `applied`, `hiddenApplied`), and only where that pass actually went: not
// under a hidden node, not into a folded section's content. A node the engine
// never measured has nothing latched to compare with, and comparing it anyway
// would re-measure on every poll.
//
// `fresh` collects the rows built in this pass, for the session to wire up
// (AutoGrow) once the re-measure has realized them.
inline bool refreshTree(LayoutNode& node, ILayoutBackend& backend, std::vector<LayoutNode*>& fresh)
{
	bool dirty = node.hidden.get() != node.hiddenApplied;
	if (node.hiddenApplied)
		return dirty;
	if (node.kind == NodeKind::Splitter && node.split.position.get() != node.split.requested)
		dirty = true;
	if (node.kind == NodeKind::Expander && node.expander.expanded.get() != node.expander.applied)
		dirty = true;

	if (node.source)
	{
		const SourceUpdate update = node.source->update();
		if (!update.empty())
		{
			regenerateChildren(node, update, backend);
			if (update.all)
			{
				for (const auto& child : node.children)
					fresh.push_back(child.get());
			}
			else
			{
				for (const std::size_t i : update.rows)
					fresh.push_back(node.children[i].get());
			}
			dirty = true;
		}
	}

	for (std::size_t i = 0; i < node.children.size(); ++i)
	{
		// A folded section's content (child 1) was not measured.
		if (node.kind == NodeKind::Expander && i == 1 && !node.expander.applied)
			continue;
		dirty |= refreshTree(*node.children[i], backend, fresh);
	}
	return dirty;
}
