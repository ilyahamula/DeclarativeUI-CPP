#include "test_framework.hpp"
#include "fake_widget.hpp"

#include "frameworks_core/LayoutEngine.hpp"
#include "frameworks_core/NodeSource.hpp"
#include "mock_layout_backend.hpp"

#include <string>
#include <vector>

// Rows generated from data (VForEach / HForEach): regenerateChildren() and the
// sessions' refreshTree() walk, driven through a plain NodeSource so the engine
// side is tested without any backend.

namespace
{

ControlWrapper* fake(int& tag)
{
	return testfw::fakeWidget(&tag);
}

// One leaf per item; the leaf's widget tag is the item's slot in `tags`.
class ListSource : public NodeSource
{
public:
	ListSource(const std::vector<int>* bound, std::vector<int> items)
		: m_bound(bound), m_items(std::move(items))
	{
	}

	SourceUpdate update() override
	{
		SourceUpdate update;
		if (m_bound->size() != m_items.size())
			update.all = true;
		else
			for (std::size_t i = 0; i < m_items.size(); ++i)
				if ((*m_bound)[i] != m_items[i])
					update.rows.push_back(i);
		if (!update.empty())
			m_items = *m_bound;
		return update;
	}

	std::size_t count() const override { return m_items.size(); }

	std::unique_ptr<LayoutNode> buildRow(std::size_t index) override
	{
		++built;
		return makeLeaf(fake(tags[(std::size_t)m_items[index]]));
	}

	int tags[16] = {};
	int built = 0;

private:
	const std::vector<int>* m_bound;
	std::vector<int> m_items;
};

std::unique_ptr<LayoutNode> makeForEach(std::vector<int>& data, ListSource*& out)
{
	auto node = makeBox(Orientation::Vertical);
	auto source = std::make_shared<ListSource>(&data, data);
	for (std::size_t i = 0; i < source->count(); ++i)
		node->add(source->buildRow(i));
	out = source.get();
	node->source = source;
	return node;
}

} // namespace

TEST(foreach_rows_follow_a_growing_vector)
{
	std::vector<int> data { 1, 2 };
	ListSource* source = nullptr;
	auto root = makeForEach(data, source);
	MockLayoutBackend mock;
	mock.defaultSize = { 40, 20 };
	LayoutEngine engine(mock);
	CHECK_EQ(engine.resolve(*root).height, 48);

	std::vector<LayoutNode*> fresh;
	CHECK(!refreshTree(*root, mock, fresh)); // nothing moved yet

	data.push_back(3);
	CHECK(refreshTree(*root, mock, fresh));
	CHECK_EQ(root->children.size(), static_cast<std::size_t>(3));
	CHECK_EQ(fresh.size(), static_cast<std::size_t>(3)); // a count change rebuilds every row
	CHECK_EQ(engine.resolve(*root).height, 76);
	for (const auto& child : root->children)
		CHECK(child->parent == root.get());
}

TEST(foreach_rebuilds_only_the_rows_that_changed)
{
	std::vector<int> data { 1, 2, 3 };
	ListSource* source = nullptr;
	auto root = makeForEach(data, source);
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	engine.resolve(*root);
	LayoutNode* first = root->children[0].get();
	LayoutNode* third = root->children[2].get();
	const int builtBefore = source->built;

	data[1] = 5;
	std::vector<LayoutNode*> fresh;
	CHECK(refreshTree(*root, mock, fresh));
	CHECK_EQ(source->built - builtBefore, 1);
	CHECK(root->children[0].get() == first);  // untouched rows keep their nodes
	CHECK(root->children[2].get() == third);
	CHECK_EQ(fresh.size(), static_cast<std::size_t>(1));
	CHECK(root->children[1]->widget == fake(source->tags[5]));
}

// refreshTree compares against what the last pass LATCHED, and only where the
// pass went -- a node never measured must not keep the session re-measuring.
TEST(refresh_tree_ignores_what_the_last_pass_skipped)
{
	int aTag = 0, bTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);

	bool hideB = false;
	auto root = makeBox(Orientation::Vertical);
	auto& expander = root->add(makeExpander("x"));
	expander.add(makeLeaf(fake(aTag)));
	auto& content = expander.add(makeBox(Orientation::Vertical));
	auto& b = content.add(makeLeaf(fake(bTag)));
	b.hidden.bind(hideB);
	engine.resolve(*root); // collapsed: content never measured

	std::vector<LayoutNode*> fresh;
	hideB = true;          // inside the folded section: not laid out, not compared
	CHECK(!refreshTree(*root, mock, fresh));

	auto split = makeSplitter(Orientation::Horizontal);
	split->add(makeBox(Orientation::Vertical));
	split->add(makeLeaf(fake(aTag)));
	split->add(makeBox(Orientation::Vertical));
	engine.run(*split, { 200, 50 });
	CHECK(!refreshTree(*split, mock, fresh)); // "-1 = half" is not a change
	split->split.position.snapshot(120);
	CHECK(refreshTree(*split, mock, fresh));   // a drag is
}
