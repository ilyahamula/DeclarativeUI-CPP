#include "test_framework.hpp"
#include "fake_widget.hpp"

#include "frameworks_core/LayoutEngine.hpp"
#include "mock_layout_backend.hpp"

// isHidden(): a hidden node takes no part in the layout. In a Box it is as if
// it had never been declared -- no size, no margins, no gap either side; in a
// Grid it leaves an empty slot; it joins no SizeGroup; and traversal hands it
// to the backend's hide() instead of placing it or anything under it.

namespace
{

ControlWrapper* fake(int& tag)
{
	return testfw::fakeWidget(&tag);
}

const Constraints kLoose { 10000, 10000 };

} // namespace

TEST(hidden_child_takes_no_space_or_gap_in_a_box)
{
	int aTag = 0, bTag = 0, cTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(aTag), { 50, 20 });
	mock.setSize(fake(bTag), { 70, 20 });
	mock.setSize(fake(cTag), { 30, 20 });

	auto row = makeBox(Orientation::Horizontal);
	auto& a = row->add(makeLeaf(fake(aTag)));
	auto& b = row->add(makeLeaf(fake(bTag), LayoutFlags().Border(Side::All, 20)));
	auto& c = row->add(makeLeaf(fake(cTag)));
	b.hidden.snapshot(true);

	const Size desired = engine.measure(*row, kLoose);
	// 50 + gap 8 + 30: b's width, its 20 px margins and its second gap are gone
	CHECK_EQ(desired.width, 88);
	CHECK_EQ(desired.height, 20);

	engine.arrange(*row, { 0, 0, 88, 20 });
	CHECK(a.frame == (Rect { 0, 0, 50, 20 }));
	CHECK(c.frame == (Rect { 58, 0, 30, 20 }));
	CHECK_EQ(b.frame.width, 0);
	CHECK_EQ(b.frame.height, 0);
}

TEST(hidden_node_is_not_measured_or_placed_and_is_handed_to_hide)
{
	int aTag = 0, bTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(aTag), { 50, 20 });
	mock.setSize(fake(bTag), { 50, 20 });

	auto column = makeBox(Orientation::Vertical);
	column->add(makeLeaf(fake(aTag)));
	auto& group = column->add(makeGroupBox(Orientation::Vertical, "group"));
	group.add(makeLeaf(fake(bTag)));
	group.hidden.snapshot(true);

	engine.run(*column);
	// b sits under the hidden group: never measured, never placed
	CHECK_EQ(mock.measureCalls.size(), static_cast<std::size_t>(1));
	CHECK_EQ(mock.placed.size(), static_cast<std::size_t>(1));
	CHECK(mock.placed.front().widget == fake(aTag));
	CHECK_EQ(mock.hiddenNodes.size(), static_cast<std::size_t>(1));
	CHECK(mock.hiddenNodes.front() == &group);
}

TEST(hidden_node_ignores_min_size_and_size_group)
{
	int aTag = 0, bTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(aTag), { 40, 20 });
	mock.setSize(fake(bTag), { 200, 20 });

	auto column = makeBox(Orientation::Vertical);
	auto& a = column->add(makeLeaf(fake(aTag), LayoutFlags().SizeGroup(1)));
	auto& b = column->add(makeLeaf(fake(bTag), LayoutFlags().SizeGroup(1).MinSize({ 300, 50 })));
	b.hidden.snapshot(true);

	engine.resolve(*column);
	CHECK(b.desired == (Size { 0, 0 }));
	CHECK_EQ(a.desired.width, 40); // the hidden 200 px member does not widen it
	CHECK_EQ(column->desired.height, 20);
}

TEST(hidden_grid_cell_keeps_its_slot)
{
	int t[4] = {};
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	for (int& tag : t)
		mock.setSize(fake(tag), { 40, 20 });

	auto grid = makeGrid(2);
	auto& c0 = grid->add(makeLeaf(fake(t[0])));
	auto& c1 = grid->add(makeLeaf(fake(t[1])));
	auto& c2 = grid->add(makeLeaf(fake(t[2])));
	auto& c3 = grid->add(makeLeaf(fake(t[3])));
	c1.hidden.snapshot(true);

	engine.measure(*grid, kLoose);
	engine.arrange(*grid, { 0, 0, 88, 48 });
	// c2 stays in column 0 of row 1 -- the hidden c1 did not let it reflow
	CHECK(c0.frame == (Rect { 0, 0, 40, 20 }));
	CHECK(c2.frame == (Rect { 0, 28, 40, 20 }));
	CHECK(c3.frame == (Rect { 48, 28, 40, 20 }));
	CHECK_EQ(c1.frame.width, 0);
}

TEST(hidden_flag_is_latched_per_measure)
{
	int aTag = 0, bTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(aTag), { 50, 20 });
	mock.setSize(fake(bTag), { 50, 20 });

	bool hide = false;
	auto column = makeBox(Orientation::Vertical);
	column->add(makeLeaf(fake(aTag)));
	auto& b = column->add(makeLeaf(fake(bTag)));
	b.hidden.bind(hide);

	CHECK_EQ(engine.measure(*column, kLoose).height, 48);
	hide = true;
	// arrange reads what measure latched, not the live flag
	engine.arrange(*column, { 0, 0, 50, 48 });
	CHECK_EQ(b.frame.height, 20);
	CHECK_EQ(engine.measure(*column, kLoose).height, 20);
	CHECK(b.hiddenApplied);
}
