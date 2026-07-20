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

TEST(arrange_worked_example_row)
{
	// architecture.md worked example:
	// HStack { StaticText.CenterVertical().Border(Right,5), input.Proportion(1).Expand() }
	int labelTag = 0, inputTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(labelTag), { 60, 20 });
	mock.setSize(fake(inputTag), { 100, 24 });

	auto row = makeBox(Orientation::Horizontal);
	auto& label = row->add(makeLeaf(fake(labelTag), LayoutFlags().CenterVertical().Border(Side::Right, 5)));
	auto& input = row->add(makeLeaf(fake(inputTag), LayoutFlags().Proportion(1).Expand()));

	engine.measure(*row, kLoose);
	engine.arrange(*row, { 0, 0, 300, 30 });

	// label keeps measured width, centered in the 30-high band
	CHECK(label.frame == (Rect { 0, 5, 60, 20 }));
	// leftover (300-60-5-100=135) goes entirely to the Proportion(1) input,
	// which also stretches to the full band height
	CHECK(input.frame == (Rect { 65, 0, 235, 30 }));
}

TEST(arrange_proportion_split_is_exact)
{
	int aTag = 0, bTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(aTag), { 10, 10 });
	mock.setSize(fake(bTag), { 10, 10 });

	auto row = makeBox(Orientation::Horizontal);
	auto& a = row->add(makeLeaf(fake(aTag), LayoutFlags().Proportion(1)));
	auto& b = row->add(makeLeaf(fake(bTag), LayoutFlags().Proportion(2)));

	engine.measure(*row, kLoose);
	// leftover = 300 - 20 - gap(8) = 272; weights 1:2 -> 90 + 182 (residue to last)
	engine.arrange(*row, { 0, 0, 300, 10 });
	CHECK_EQ(a.frame.width, 10 + 90);
	CHECK_EQ(b.frame.width, 10 + 182);
	// exact to the pixel: last child ends at the area's right edge
	CHECK_EQ(b.frame.x + b.frame.width, 300);
}

TEST(arrange_cross_align_start_center_stretch)
{
	int sTag = 0, cTag = 0, eTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(sTag), { 40, 10 });
	mock.setSize(fake(cTag), { 40, 10 });
	mock.setSize(fake(eTag), { 40, 10 });

	auto column = makeBox(Orientation::Vertical);
	auto& start = column->add(makeLeaf(fake(sTag)));                                  // leaf default = Start
	auto& center = column->add(makeLeaf(fake(cTag), LayoutFlags().CenterHorizontal()));
	auto& stretch = column->add(makeLeaf(fake(eTag), LayoutFlags().Expand()));

	engine.measure(*column, kLoose);
	engine.arrange(*column, { 0, 0, 200, 100 });

	CHECK(start.frame == (Rect { 0, 0, 40, 10 }));
	CHECK_EQ(center.frame.x, (200 - 40) / 2);
	CHECK_EQ(center.frame.width, 40);
	CHECK_EQ(stretch.frame.x, 0);
	CHECK_EQ(stretch.frame.width, 200);
}

TEST(arrange_widest_wins_in_column)
{
	// two group boxes + a button, no flags: containers stretch to the
	// column band, the leaf keeps its intrinsic size
	int aTag = 0, bTag = 0, btnTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(aTag), { 220, 40 });
	mock.setSize(fake(bTag), { 340, 40 });
	mock.setSize(fake(btnTag), { 90, 25 });

	auto column = makeBox(Orientation::Vertical);
	auto& boxA = column->add(makeGroupBox(Orientation::Vertical, "Credentials"));
	boxA.add(makeLeaf(fake(aTag)));
	auto& boxB = column->add(makeGroupBox(Orientation::Vertical, "Options"));
	boxB.add(makeLeaf(fake(bTag)));
	auto& button = column->add(makeLeaf(fake(btnTag)));

	engine.measure(*column, kLoose);
	CHECK_EQ(column->desired.width, 340);
	engine.arrange(*column, { 0, 0, 340, 200 });

	// both group boxes fill the widest child's band
	CHECK_EQ(boxA.frame.width, 340);
	CHECK_EQ(boxB.frame.width, 340);
	CHECK_EQ(button.frame.width, 90);
}

TEST(arrange_tallest_wins_in_row)
{
	int aTag = 0, bTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(aTag), { 100, 60 });
	mock.setSize(fake(bTag), { 100, 90 });

	auto row = makeBox(Orientation::Horizontal);
	auto& boxA = row->add(makeGroupBox(Orientation::Vertical, "A"));
	boxA.add(makeLeaf(fake(aTag)));
	auto& boxB = row->add(makeGroupBox(Orientation::Vertical, "B"));
	boxB.add(makeLeaf(fake(bTag)));

	engine.measure(*row, kLoose);
	engine.arrange(*row, { 0, 0, 300, 90 });

	CHECK_EQ(boxA.frame.height, 90);
	CHECK_EQ(boxB.frame.height, 90);
}

TEST(arrange_shrink_order_proportioned_first)
{
	int aTag = 0, bTag = 0, cTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(aTag), { 100, 20 });
	mock.setSize(fake(bTag), { 200, 20 });
	mock.setSize(fake(cTag), { 60, 20 });

	auto makeRow = [&](LayoutNode*& a, LayoutNode*& b, LayoutNode*& c) {
		auto row = makeBox(Orientation::Horizontal);
		a = &row->add(makeLeaf(fake(aTag)));
		b = &row->add(makeLeaf(fake(bTag), LayoutFlags().Proportion(1).MinSize({ 50, 20 })));
		c = &row->add(makeLeaf(fake(cTag)));
		return row;
	};

	// content = 100+200+60 + 2*8 gaps = 376
	// deficit 126: the proportioned child absorbs it all (200 -> 74)
	LayoutNode *a, *b, *c;
	auto row = makeRow(a, b, c);
	engine.measure(*row, kLoose);
	engine.arrange(*row, { 0, 0, 250, 20 });
	CHECK_EQ(a->frame.width, 100);
	CHECK_EQ(b->frame.width, 74);
	CHECK_EQ(c->frame.width, 60);
	CHECK_EQ(c->frame.x + c->frame.width, 250);

	// deficit 176 > proportioned slack (150): b hits its 50 floor, the
	// residue (26) comes from the last child in reverse declaration order
	LayoutNode *a2, *b2, *c2;
	auto row2 = makeRow(a2, b2, c2);
	engine.measure(*row2, kLoose);
	engine.arrange(*row2, { 0, 0, 200, 20 });
	CHECK_EQ(b2->frame.width, 50);
	CHECK_EQ(c2->frame.width, 60 - 26);
	CHECK_EQ(a2->frame.width, 100);
}

TEST(arrange_tab_pages_fill_panel)
{
	int aTag = 0, bTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(aTag), { 100, 40 });
	mock.setSize(fake(bTag), { 60, 80 });

	auto tabs = makeTabPanel();
	auto& page1 = tabs->add(makeBox(Orientation::Vertical));
	page1.add(makeLeaf(fake(aTag)));
	auto& page2 = tabs->add(makeBox(Orientation::Vertical, LayoutFlags().Border(Side::All, 5)));
	page2.add(makeLeaf(fake(bTag)));

	engine.measure(*tabs, kLoose);
	engine.arrange(*tabs, { 10, 20, 200, 100 });

	// pages overlap the same area, each inset by its own margins
	CHECK(page1.frame == (Rect { 10, 20, 200, 100 }));
	CHECK(page2.frame == (Rect { 15, 25, 190, 90 }));
}

TEST(run_autofit_resolves_window_and_traverses)
{
	int aTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(aTag), { 100, 50 });

	auto root = makeBox(Orientation::Vertical, LayoutFlags().Border(Side::All, 10));
	root->add(makeLeaf(fake(aTag)));

	// auto-fit: window = desired + root margins
	const Size window = engine.run(*root);
	CHECK(window == (Size { 120, 70 }));
	// root arranged inside its own margins
	CHECK(root->frame == (Rect { 10, 10, 100, 50 }));
	CHECK(mock.frameOf(fake(aTag)) == (Rect { 10, 10, 100, 50 }));
	// traversal reached the backend in order
	CHECK_EQ(mock.callLog.size(), (size_t)3);
	CHECK_EQ(mock.callLog[0], std::string("begin:"));
	CHECK_EQ(mock.callLog[1], std::string("place"));
	CHECK_EQ(mock.callLog[2], std::string("end:"));
}

TEST(run_fixed_window_keeps_size)
{
	int aTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(aTag), { 100, 50 });

	auto root = makeBox(Orientation::Vertical);
	auto& leaf = *root->children.emplace(root->children.end(), makeLeaf(fake(aTag), LayoutFlags().Expand()));

	const Size window = engine.run(*root, { 400, 300 });
	CHECK(window == (Size { 400, 300 }));
	CHECK(root->frame == (Rect { 0, 0, 400, 300 }));
	// Expand: the leaf stretches across the column band
	CHECK_EQ(leaf->frame.width, 400);
	CHECK_EQ(leaf->frame.height, 50);
}

TEST(container_chrome_grows_desired_and_insets_children)
{
	int aTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(aTag), { 100, 40 });
	// group boxes carry chrome: 4px border, 20px title row
	mock.chromeFn = [](const LayoutNode& node) {
		return node.kind == NodeKind::GroupBox ? EdgeInsets { 4, 4, 20, 4 } : EdgeInsets{};
	};

	auto box = makeGroupBox(Orientation::Vertical, "Titled");
	auto& leaf = box->add(makeLeaf(fake(aTag)));

	engine.measure(*box, kLoose);
	// desired = content + chrome
	CHECK(box->desired == (Size { 100 + 8, 40 + 24 }));

	engine.arrange(*box, { 0, 0, 108, 64 });
	// the child is arranged inside the chrome
	CHECK(leaf.frame == (Rect { 4, 20, 100, 40 }));
}

TEST(traverse_skips_hidden_subtrees)
{
	int aTag = 0, bTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(aTag), { 50, 20 });
	mock.setSize(fake(bTag), { 50, 20 });

	auto tabs = makeTabPanel();
	auto& active = tabs->add(makeBox(Orientation::Vertical));
	active.label = "Active";
	active.add(makeLeaf(fake(aTag)));
	auto& inactive = tabs->add(makeBox(Orientation::Vertical));
	inactive.label = "Hidden";
	inactive.add(makeLeaf(fake(bTag)));

	mock.hiddenContainers.insert(&inactive);
	engine.run(*tabs, { 200, 100 });

	// only the visible page's leaf was placed
	CHECK_EQ(mock.placed.size(), (size_t)1);
	CHECK(mock.placed[0].widget == fake(aTag));
	// hidden page: begin logged as skip, no end, subtree not entered
	bool sawSkip = false, sawHiddenEnd = false;
	for (const auto& entry : mock.callLog)
	{
		if (entry == "skip:Hidden")
			sawSkip = true;
		if (entry == "end:Hidden")
			sawHiddenEnd = true;
	}
	CHECK(sawSkip);
	CHECK(!sawHiddenEnd);
}

TEST(run_autofit_caps_wrapping_width)
{
	int wTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setMeasure(fake(wTag), [](const Constraints& c) {
		const int width = std::min(c.maxWidth, 2000); // wants to be very wide
		return Size { width, 6000 / width };
	});

	auto root = makeBox(Orientation::Vertical);
	root->add(makeLeaf(fake(wTag)));

	engine.setMaxAutoFitWidth(300);
	const Size window = engine.run(*root);
	// the cap kept the window at 300 wide; the text wrapped taller instead
	CHECK(window == (Size { 300, 20 }));
}
