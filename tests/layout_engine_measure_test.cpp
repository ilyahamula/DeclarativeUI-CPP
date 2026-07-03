#include "test_framework.hpp"

#include "frameworks_core/LayoutEngine.hpp"
#include "mock_layout_backend.hpp"

namespace
{

ControlWrapper* fake(int& tag)
{
	return reinterpret_cast<ControlWrapper*>(&tag);
}

const Constraints kLoose { 10000, 10000 };

} // namespace

TEST(measure_leaf_clamps_to_min_max)
{
	int aTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(aTag), { 80, 20 });

	// MinSize is the floor: raises a smaller intrinsic size
	auto floored = makeLeaf(fake(aTag), LayoutFlags().MinSize({ 120, 20 }));
	CHECK(engine.measure(*floored, kLoose) == (Size { 120, 20 }));

	// MaxSize caps a larger intrinsic size
	auto capped = makeLeaf(fake(aTag), LayoutFlags().MaxSize({ 50, 15 }));
	CHECK(engine.measure(*capped, kLoose) == (Size { 50, 15 }));

	// desired is stored on the node
	CHECK(capped->desired == (Size { 50, 15 }));
}

TEST(measure_row_sums_main_and_maxes_cross)
{
	int aTag = 0, bTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(aTag), { 80, 20 });
	mock.setSize(fake(bTag), { 40, 30 });

	auto row = makeBox(Orientation::Horizontal);
	row->add(makeLeaf(fake(aTag)));
	row->add(makeLeaf(fake(bTag)));

	// no borders: default gap between siblings
	const int expected = 80 + LayoutEngine::kDefaultGap + 40;
	CHECK(engine.measure(*row, kLoose) == (Size { expected, 30 }));
}

TEST(measure_gap_rules)
{
	int aTag = 0, bTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(aTag), { 80, 20 });
	mock.setSize(fake(bTag), { 40, 20 });

	// explicit border replaces the default gap, even when smaller
	auto row = makeBox(Orientation::Horizontal);
	row->add(makeLeaf(fake(aTag), LayoutFlags().Border(Side::Right, 5)));
	row->add(makeLeaf(fake(bTag)));
	CHECK(engine.measure(*row, kLoose) == (Size { 80 + 5 + 40, 20 }));

	// facing margins collapse via max, not sum
	auto row2 = makeBox(Orientation::Horizontal);
	row2->add(makeLeaf(fake(aTag), LayoutFlags().Border(Side::Right, 5)));
	row2->add(makeLeaf(fake(bTag), LayoutFlags().Border(Side::Left, 12)));
	CHECK(engine.measure(*row2, kLoose) == (Size { 80 + 12 + 40, 20 }));

	// leading/trailing margins of first/last child count toward the box
	auto row3 = makeBox(Orientation::Horizontal);
	row3->add(makeLeaf(fake(aTag), LayoutFlags().Border(Side::Left, 10)));
	row3->add(makeLeaf(fake(bTag), LayoutFlags().Border(Side::Right, 7)));
	CHECK(engine.measure(*row3, kLoose)
		== (Size { 10 + 80 + LayoutEngine::kDefaultGap + 40 + 7, 20 }));
}

TEST(measure_column_cross_includes_margins)
{
	int aTag = 0, bTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(aTag), { 80, 20 });
	mock.setSize(fake(bTag), { 100, 30 });

	auto column = makeBox(Orientation::Vertical);
	column->add(makeLeaf(fake(aTag), LayoutFlags().Border(Side::Left, 50)));
	column->add(makeLeaf(fake(bTag)));

	// cross = max(80 + 50 left margin, 100) = 130; main = 20 + gap + 30
	CHECK(engine.measure(*column, kLoose)
		== (Size { 130, 20 + LayoutEngine::kDefaultGap + 30 }));
}

TEST(measure_nested_row_in_column)
{
	int aTag = 0, bTag = 0, cTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(aTag), { 60, 20 });  // label
	mock.setSize(fake(bTag), { 100, 24 }); // input
	mock.setSize(fake(cTag), { 90, 25 });  // button below

	auto column = makeBox(Orientation::Vertical);
	auto& row = column->add(makeBox(Orientation::Horizontal));
	row.add(makeLeaf(fake(aTag), LayoutFlags().Border(Side::Right, 5)));
	row.add(makeLeaf(fake(bTag)));
	column->add(makeLeaf(fake(cTag)));

	const int rowWidth = 60 + 5 + 100;
	CHECK(row.desired == Size{}); // not measured yet
	CHECK(engine.measure(*column, kLoose)
		== (Size { rowWidth, 24 + LayoutEngine::kDefaultGap + 25 }));
	CHECK(row.desired == (Size { rowWidth, 24 }));
}

TEST(measure_wrap_at_cap)
{
	int wTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);

	// wrapping text: 6000 px² of glyphs, wraps at the offered width
	mock.setMeasure(fake(wTag), [](const Constraints& c) {
		const int width = std::min(c.maxWidth, 600);
		return Size { width, 6000 / width };
	});

	auto column = makeBox(Orientation::Vertical);
	column->add(makeLeaf(fake(wTag)));

	// loose constraints: text takes its natural 600x10
	CHECK(engine.measure(*column, kLoose) == (Size { 600, 10 }));

	// capped root (auto-fit max width): text wraps taller instead of wider
	CHECK(engine.measure(*column, { 300, 10000 }) == (Size { 300, 20 }));
}

TEST(measure_constraints_shrink_by_child_margins)
{
	int wTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(wTag), { 10, 10 });

	auto column = makeBox(Orientation::Vertical);
	column->add(makeLeaf(fake(wTag), LayoutFlags().Border(Side::All, 10)));

	engine.measure(*column, { 300, 200 });
	CHECK_EQ(mock.measureCalls.size(), (size_t)1);
	// child was offered the parent constraint minus its own margins
	CHECK(mock.measureCalls[0].constraints == (Constraints { 280, 180 }));
}

TEST(measure_tab_panel_takes_largest_page)
{
	int aTag = 0, bTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(aTag), { 200, 50 });
	mock.setSize(fake(bTag), { 120, 90 });

	auto tabs = makeTabPanel();
	auto& page1 = tabs->add(makeBox(Orientation::Vertical));
	page1.add(makeLeaf(fake(aTag)));
	auto& page2 = tabs->add(makeBox(Orientation::Vertical));
	page2.add(makeLeaf(fake(bTag)));

	CHECK(engine.measure(*tabs, kLoose) == (Size { 200, 90 }));
}

TEST(measure_empty_box_is_zero)
{
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	auto box = makeBox(Orientation::Horizontal);
	CHECK(engine.measure(*box, kLoose) == (Size { 0, 0 }));
}
