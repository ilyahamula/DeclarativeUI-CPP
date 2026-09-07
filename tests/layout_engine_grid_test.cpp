#include "test_framework.hpp"

#include "frameworks_core/LayoutEngine.hpp"
#include "mock_layout_backend.hpp"

// Grid measure/arrange (requirements.md R2). The grid is engine-only: no
// backend chrome, no scope, so every rule below is pure arithmetic over the
// node tree and holds identically on all three backends.

namespace
{

ControlWrapper* fake(int& tag)
{
	return reinterpret_cast<ControlWrapper*>(&tag);
}

const Constraints kLoose { 10000, 10000 };
constexpr int kGap = LayoutEngine::kDefaultGap; // 8

} // namespace

TEST(grid_columns_take_widest_cell_rows_take_tallest)
{
	// 2 columns x 2 rows; column 0 is driven by the wider label in row 1,
	// column 1 by the taller field in row 0.
	int a = 0, b = 0, c = 0, d = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(a), { 40, 20 });
	mock.setSize(fake(b), { 100, 30 });
	mock.setSize(fake(c), { 70, 16 });
	mock.setSize(fake(d), { 80, 24 });

	auto grid = makeGrid(2);
	auto& cellA = grid->add(makeLeaf(fake(a)));
	auto& cellB = grid->add(makeLeaf(fake(b)));
	auto& cellC = grid->add(makeLeaf(fake(c)));
	auto& cellD = grid->add(makeLeaf(fake(d)));

	const Size desired = engine.measure(*grid, kLoose);
	// col0 = max(40,70) = 70, col1 = max(100,80) = 100, one 8px gutter
	CHECK_EQ(desired.width, 70 + kGap + 100);
	// row0 = max(20,30) = 30, row1 = max(16,24) = 24, one 8px gutter
	CHECK_EQ(desired.height, 30 + kGap + 24);

	engine.arrange(*grid, { 0, 0, desired.width, desired.height });
	// leaves default to Start on both axes: intrinsic size, top-left of the band
	CHECK(cellA.frame == (Rect { 0, 0, 40, 20 }));
	CHECK(cellB.frame == (Rect { 70 + kGap, 0, 100, 30 }));
	CHECK(cellC.frame == (Rect { 0, 30 + kGap, 70, 16 }));
	CHECK(cellD.frame == (Rect { 70 + kGap, 30 + kGap, 80, 24 }));
	// the column line is straight: both cells of column 1 share an x
	CHECK_EQ(cellB.frame.x, cellD.frame.x);
}

TEST(grid_ragged_last_row_is_allowed)
{
	// 3 columns, 5 cells: the last row has two, and the missing third cell
	// neither adds a row nor disturbs column 2's width.
	int tags[5] = { 0, 0, 0, 0, 0 };
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	for (int& tag : tags)
		mock.setSize(fake(tag), { 30, 10 });

	auto grid = makeGrid(3);
	std::vector<LayoutNode*> cells;
	for (int& tag : tags)
		cells.push_back(&grid->add(makeLeaf(fake(tag))));

	const Size desired = engine.measure(*grid, kLoose);
	CHECK_EQ(desired.width, 30 * 3 + kGap * 2);
	CHECK_EQ(desired.height, 10 * 2 + kGap); // two rows, not three

	engine.arrange(*grid, { 0, 0, desired.width, desired.height });
	CHECK(cells[3]->frame == (Rect { 0, 10 + kGap, 30, 10 }));
	CHECK(cells[4]->frame == (Rect { 30 + kGap, 10 + kGap, 30, 10 }));
}

TEST(grid_cell_margins_pad_the_band_and_set_the_gutter)
{
	// A cell's Border is both its padding inside its own band and the source of
	// the gutter beside it (R2.1) -- deliberately unlike a Box, where a margin
	// only ever becomes the gap.
	int a = 0, b = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(a), { 40, 20 });
	mock.setSize(fake(b), { 40, 20 });

	auto grid = makeGrid(2);
	auto& cellA = grid->add(makeLeaf(fake(a), LayoutFlags().Border(Side::All, 5)));
	auto& cellB = grid->add(makeLeaf(fake(b)));

	const Size desired = engine.measure(*grid, kLoose);
	// col0 = 40 + 5 + 5 = 50; the gutter is the widest facing margin on that
	// line -- max(A's right 5, B's left 0) = 5 -- and an explicit margin beats
	// the kDefaultGap fallback even when it is narrower than the default
	CHECK_EQ(desired.width, 50 + 5 + 40);
	CHECK_EQ(desired.height, 20 + 5 + 5);

	engine.arrange(*grid, { 0, 0, desired.width, desired.height });
	// the margin holds the cell off its own band edge
	CHECK(cellA.frame == (Rect { 5, 5, 40, 20 }));
	CHECK(cellB.frame == (Rect { 50 + 5, 0, 40, 20 }));
}

TEST(grid_gutter_is_the_widest_facing_margin_along_the_whole_line)
{
	// Row 0 sets no margin, row 1 sets 12 on the column line: the line is one
	// gutter for the whole grid, so 12 wins and column 1 stays straight.
	int tags[4] = { 0, 0, 0, 0 };
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	for (int& tag : tags)
		mock.setSize(fake(tag), { 20, 10 });

	auto grid = makeGrid(2);
	grid->add(makeLeaf(fake(tags[0])));
	auto& topRight = grid->add(makeLeaf(fake(tags[1])));
	grid->add(makeLeaf(fake(tags[2])));
	auto& bottomRight = grid->add(makeLeaf(fake(tags[3]), LayoutFlags().Border(Side::Left, 12)));

	const Size desired = engine.measure(*grid, kLoose);
	// col1 = max(20, 20 + 12 left margin) = 32; gutter = max(12, kDefaultGap) = 12
	CHECK_EQ(desired.width, 20 + 12 + 32);

	engine.arrange(*grid, { 0, 0, desired.width, desired.height });
	// both cells share column 1's band origin (20 + the 12px gutter); the
	// bottom cell's own margin then pads it 12 further in
	CHECK_EQ(topRight.frame.x, 20 + 12);
	CHECK_EQ(bottomRight.frame.x, 20 + 12 + 12);
}

TEST(grid_leftover_goes_to_bands_by_max_proportion)
{
	// Column 1 carries Proportion(1) on one of its cells, so it takes all the
	// spare width; column 0 keeps its measured size. Rows are unweighted, so
	// spare height is left on the floor.
	int tags[4] = { 0, 0, 0, 0 };
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	for (int& tag : tags)
		mock.setSize(fake(tag), { 50, 20 });

	auto grid = makeGrid(2);
	auto& a = grid->add(makeLeaf(fake(tags[0])));
	auto& b = grid->add(makeLeaf(fake(tags[1]), LayoutFlags().Proportion(1).Expand()));
	grid->add(makeLeaf(fake(tags[2])));
	auto& d = grid->add(makeLeaf(fake(tags[3])));

	const Size desired = engine.measure(*grid, kLoose);
	CHECK_EQ(desired.width, 50 + kGap + 50);

	// 100 px of spare width, 40 of spare height
	engine.arrange(*grid, { 0, 0, desired.width + 100, desired.height + 40 });
	CHECK_EQ(a.frame.width, 50);        // unweighted column keeps its size
	CHECK_EQ(b.frame.width, 50 + 100);  // Expand() fills the grown band
	CHECK_EQ(b.frame.x + b.frame.width, desired.width + 100); // exact to the pixel
	// column 1's other cell shares the widened band, but is Start-aligned so it
	// keeps its intrinsic width
	CHECK_EQ(d.frame.x, 50 + kGap);
	CHECK_EQ(d.frame.width, 50);
	// no row asked to flex: the grid keeps its measured height
	CHECK_EQ(a.frame.height, 20);
}

TEST(grid_proportion_split_between_two_columns_is_exact)
{
	int a = 0, b = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(a), { 10, 10 });
	mock.setSize(fake(b), { 10, 10 });

	auto grid = makeGrid(2);
	auto& cellA = grid->add(makeLeaf(fake(a), LayoutFlags().Proportion(1).Expand()));
	auto& cellB = grid->add(makeLeaf(fake(b), LayoutFlags().Proportion(2).Expand()));

	engine.measure(*grid, kLoose);
	// leftover = 300 - 20 - gap(8) = 272; weights 1:2 -> 90 + 182 (residue last)
	engine.arrange(*grid, { 0, 0, 300, 10 });
	CHECK_EQ(cellA.frame.width, 10 + 90);
	CHECK_EQ(cellB.frame.width, 10 + 182);
	CHECK_EQ(cellB.frame.x + cellB.frame.width, 300);
}

TEST(grid_cell_alignment_resolves_on_both_axes)
{
	// One row of three cells in bands grown past their intrinsic size: Start
	// keeps top-left, Center centres on both axes, Expand stretches both.
	//
	// Note that a Proportion weights BOTH bands its cell sits in -- the column
	// AND the row -- so the two weighted cells here also make row 0 absorb the
	// spare height, which is what gives the alignments something to resolve
	// against on the vertical axis.
	int s = 0, c = 0, e = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(s), { 20, 10 });
	mock.setSize(fake(c), { 20, 10 });
	mock.setSize(fake(e), { 20, 10 });

	auto grid = makeGrid(3);
	auto& start = grid->add(makeLeaf(fake(s)));
	auto& center = grid->add(makeLeaf(fake(c), LayoutFlags().Center().Proportion(1)));
	auto& expand = grid->add(makeLeaf(fake(e), LayoutFlags().Expand().Proportion(1)));

	engine.measure(*grid, kLoose);
	engine.arrange(*grid, { 0, 0, 200, 50 });

	// the row grew to the full 50 (both weighted cells live in it)
	CHECK_EQ(expand.frame.height, 50);

	// Start: intrinsic size at the band's top-left, unmoved by the taller row
	CHECK(start.frame == (Rect { 0, 0, 20, 10 }));

	// Center: intrinsic size, centred on both axes of its band
	CHECK_EQ(center.frame.width, 20);
	CHECK_EQ(center.frame.height, 10);
	CHECK_EQ(center.frame.y, (50 - 10) / 2);

	// Expand: fills its band on both axes, out to the area's right edge
	CHECK(expand.frame.width > 20);
	CHECK_EQ(expand.frame.x + expand.frame.width, 200);
}

TEST(grid_rows_grow_when_a_cell_weights_them)
{
	int a = 0, b = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(a), { 20, 10 });
	mock.setSize(fake(b), { 20, 10 });

	auto grid = makeGrid(1); // one column, two rows
	auto& top = grid->add(makeLeaf(fake(a)));
	auto& bottom = grid->add(makeLeaf(fake(b), LayoutFlags().Proportion(1).Expand()));

	engine.measure(*grid, kLoose);
	engine.arrange(*grid, { 0, 0, 20, 10 + kGap + 10 + 60 });
	CHECK_EQ(top.frame.height, 10);       // unweighted row keeps its size
	CHECK_EQ(bottom.frame.height, 10 + 60);
	CHECK_EQ(bottom.frame.y + bottom.frame.height, 10 + kGap + 10 + 60);
}

TEST(grid_sizegroup_equalizes_across_two_grids)
{
	// The classic two-form case: the first column of each grid joins one
	// SizeGroup, so both forms line up even though their labels differ.
	int l1 = 0, f1 = 0, l2 = 0, f2 = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(l1), { 40, 20 });
	mock.setSize(fake(f1), { 100, 20 });
	mock.setSize(fake(l2), { 90, 20 }); // the wider label
	mock.setSize(fake(f2), { 100, 20 });

	auto root = makeBox(Orientation::Vertical);
	auto& gridA = root->add(makeGrid(2));
	auto& labelA = gridA.add(makeLeaf(fake(l1), LayoutFlags().SizeGroup(1)));
	gridA.add(makeLeaf(fake(f1)));
	auto& gridB = root->add(makeGrid(2));
	auto& labelB = gridB.add(makeLeaf(fake(l2), LayoutFlags().SizeGroup(1)));
	gridB.add(makeLeaf(fake(f2)));

	engine.resolve(*root);
	// both labels are raised to the group maximum...
	CHECK_EQ(labelA.desired.width, 90);
	CHECK_EQ(labelB.desired.width, 90);
	// ...and each grid's first column follows, so the two forms line up
	CHECK_EQ(gridA.desired.width, 90 + kGap + 100);
	CHECK_EQ(gridB.desired.width, 90 + kGap + 100);
}

TEST(grid_disabled_cascades_to_every_cell)
{
	// isDisabledEffective() reads the leaf's own wrapper, so this one needs real
	// ControlWrappers rather than the mock's tag pointers.
	ControlWrapper widgetA;
	ControlWrapper widgetB;

	bool locked = false;
	auto grid = makeGrid(2);
	grid->disabled.bind(locked);
	auto& cellA = grid->add(makeLeaf(&widgetA));
	auto& cellB = grid->add(makeLeaf(&widgetB));

	CHECK(!cellA.isDisabledEffective());
	locked = true;
	// the flag is bound, so the cascade follows without rebuilding the tree
	CHECK(cellA.isDisabledEffective());
	CHECK(cellB.isDisabledEffective());
	// and it is the grid, not the cells, that carries it
	CHECK(!cellA.disabled.get());
}

TEST(grid_empty_measures_nothing)
{
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	auto grid = makeGrid(3);
	CHECK(engine.measure(*grid, kLoose) == (Size { 0, 0 }));
	engine.arrange(*grid, { 0, 0, 100, 100 }); // must not fall over
	CHECK(grid->children.empty());
}

TEST(grid_single_column_matches_a_vertical_box)
{
	// A 1-column grid and a VStack of the same leaves must agree: same bands,
	// same default gap, same Start alignment.
	int a = 0, b = 0, c = 0, d = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(a), { 30, 10 });
	mock.setSize(fake(b), { 50, 20 });
	mock.setSize(fake(c), { 30, 10 });
	mock.setSize(fake(d), { 50, 20 });

	auto grid = makeGrid(1);
	auto& g1 = grid->add(makeLeaf(fake(a)));
	auto& g2 = grid->add(makeLeaf(fake(b)));
	auto column = makeBox(Orientation::Vertical);
	auto& c1 = column->add(makeLeaf(fake(c)));
	auto& c2 = column->add(makeLeaf(fake(d)));

	const Size gridSize = engine.measure(*grid, kLoose);
	const Size columnSize = engine.measure(*column, kLoose);
	CHECK(gridSize == columnSize);

	engine.arrange(*grid, { 0, 0, gridSize.width, gridSize.height });
	engine.arrange(*column, { 0, 0, columnSize.width, columnSize.height });
	CHECK(g1.frame == c1.frame);
	CHECK(g2.frame == c2.frame);
}
