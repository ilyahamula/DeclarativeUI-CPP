#include "test_framework.hpp"

#include "frameworks_core/LayoutEngine.hpp"
#include "mock_layout_backend.hpp"

// Splitter measure/arrange (requirements.md R4). The sash is an ordinary leaf
// here -- a mock one 6 px thick -- which is exactly how the backends see it:
// nothing in the engine knows the sash can be dragged, only that the middle
// child of a Splitter divides the area the other two share.

namespace
{

ControlWrapper* fake(int& tag)
{
	return reinterpret_cast<ControlWrapper*>(&tag);
}

const Constraints kLoose { 10000, 10000 };
constexpr int kSash = SplitterState::kSashThickness; // 6
constexpr int kMinPane = SplitterState::kDefaultMinPane; // 40

// The tree every test below starts from: two leaf panes with a sash between
// them, laid out inside `area` once measured.
struct Fixture
{
	int firstTag = 0;
	int sashTag = 0;
	int secondTag = 0;

	MockLayoutBackend mock;
	LayoutEngine engine { mock };
	std::unique_ptr<LayoutNode> splitter;
	LayoutNode* first = nullptr;
	LayoutNode* sash = nullptr;
	LayoutNode* second = nullptr;

	Fixture(Orientation orient, Size firstSize, Size secondSize,
		LayoutFlags firstFlags = {}, LayoutFlags secondFlags = {})
	{
		mock.setSize(fake(firstTag), firstSize);
		mock.setSize(fake(secondTag), secondSize);
		mock.setSize(fake(sashTag), orient == Orientation::Horizontal
			? Size { kSash, 0 } : Size { 0, kSash });

		splitter = makeSplitter(orient);
		first = &splitter->add(makeLeaf(fake(firstTag), firstFlags));
		sash = &splitter->add(makeLeaf(fake(sashTag), LayoutFlags().Expand()));
		second = &splitter->add(makeLeaf(fake(secondTag), secondFlags));
	}

	void run(const Rect& area)
	{
		engine.measure(*splitter, kLoose);
		engine.arrange(*splitter, area);
	}
};

} // namespace

TEST(splitter_measures_both_panes_plus_the_sash)
{
	Fixture f(Orientation::Horizontal, { 120, 80 }, { 200, 60 });

	const Size desired = f.engine.measure(*f.splitter, kLoose);
	CHECK_EQ(desired.width, 120 + kSash + 200);
	CHECK_EQ(desired.height, 80); // cross axis: the tallest of the three
}

TEST(splitter_position_defaults_to_half_the_pane_space)
{
	// -1 is "half", and it can only be resolved at arrange time, when the area
	// is finally known -- the desired size says nothing about it.
	Fixture f(Orientation::Horizontal, { 120, 80 }, { 200, 60 });
	f.run({ 0, 0, 406, 100 }); // 400 for the panes once the sash is spent

	CHECK_EQ(f.splitter->split.resolved, 200);
	CHECK_EQ(f.first->frame.width, 200);
	CHECK_EQ(f.sash->frame.x, 200);
	CHECK_EQ(f.sash->frame.width, kSash);
	CHECK_EQ(f.second->frame.x, 200 + kSash);
	CHECK_EQ(f.second->frame.width, 200);
}

TEST(splitter_bound_position_places_the_sash)
{
	int position = 90;
	Fixture f(Orientation::Horizontal, { 120, 80 }, { 200, 60 });
	f.splitter->split.position.bind(position);
	f.run({ 0, 0, 406, 100 });

	CHECK_EQ(f.first->frame.width, 90);
	CHECK_EQ(f.sash->frame.x, 90);
	CHECK_EQ(f.second->frame.width, 400 - 90);
	// the engine never writes the caller's int -- it only reads it
	CHECK_EQ(position, 90);
}

TEST(splitter_clamps_to_the_default_min_pane_sizes)
{
	Fixture f(Orientation::Horizontal, { 120, 80 }, { 200, 60 });

	f.splitter->split.position.snapshot(5); // below the first pane's floor
	f.run({ 0, 0, 406, 100 });
	CHECK_EQ(f.first->frame.width, kMinPane);
	CHECK_EQ(f.splitter->split.resolved, kMinPane);

	f.splitter->split.position.snapshot(9000); // past the second pane's floor
	f.run({ 0, 0, 406, 100 });
	CHECK_EQ(f.first->frame.width, 400 - kMinPane);
	CHECK_EQ(f.second->frame.width, kMinPane);
}

TEST(splitter_withminpanesize_overrides_the_defaults)
{
	Fixture f(Orientation::Horizontal, { 120, 80 }, { 200, 60 });
	f.splitter->split.minFirst = 100;
	f.splitter->split.minSecond = 150;

	f.splitter->split.position.snapshot(10);
	f.run({ 0, 0, 406, 100 });
	CHECK_EQ(f.first->frame.width, 100);

	f.splitter->split.position.snapshot(9000);
	f.run({ 0, 0, 406, 100 });
	CHECK_EQ(f.second->frame.width, 150);
}

TEST(splitter_pane_minsize_raises_its_floor)
{
	// R4.5: withMinPaneSize() and a pane's own MinSize say the same thing, so
	// the larger of the two wins -- here MinSize is the larger one.
	Fixture f(Orientation::Horizontal, { 120, 80 }, { 200, 60 },
		LayoutFlags().MinSize({ 130, 0 }));

	f.splitter->split.position.snapshot(10);
	f.run({ 0, 0, 406, 100 });
	CHECK_EQ(f.first->frame.width, 130);
	CHECK_EQ(f.splitter->split.lowerBound, 130);
}

TEST(splitter_publishes_the_drag_bounds_it_clamped_against)
{
	// The sash reads these while dragging, so a drag past a floor cannot bank
	// up an out-of-range value the user then has to drag all the way back.
	Fixture f(Orientation::Horizontal, { 120, 80 }, { 200, 60 });
	f.splitter->split.minFirst = 60;
	f.splitter->split.minSecond = 90;
	f.run({ 0, 0, 406, 100 });

	CHECK_EQ(f.splitter->split.lowerBound, 60);
	CHECK_EQ(f.splitter->split.upperBound, 400 - 90);
	CHECK_EQ(f.splitter->split.resolved, 200);
}

TEST(splitter_too_small_for_both_floors_does_not_overflow)
{
	// 100 px of pane space against two 40 px floors plus a 200 px request:
	// the first pane takes what there is and the second gets the remainder,
	// which is what keeps the sash inside the splitter's own frame.
	Fixture f(Orientation::Horizontal, { 120, 80 }, { 200, 60 });
	f.splitter->split.minFirst = 80;
	f.splitter->split.minSecond = 80;
	f.splitter->split.position.snapshot(200);
	f.run({ 0, 0, 106, 100 });

	CHECK_EQ(f.first->frame.width, 80);
	CHECK_EQ(f.second->frame.width, 20);
	CHECK_EQ(f.second->frame.x + f.second->frame.width, 106);
}

TEST(splitter_vertical_stacks_its_panes)
{
	Fixture f(Orientation::Vertical, { 200, 100 }, { 160, 150 });

	const Size desired = f.engine.measure(*f.splitter, kLoose);
	CHECK_EQ(desired.height, 100 + kSash + 150);
	CHECK_EQ(desired.width, 200);

	f.run({ 0, 0, 300, 306 });
	CHECK_EQ(f.first->frame.height, 150);
	CHECK_EQ(f.sash->frame.y, 150);
	CHECK_EQ(f.sash->frame.height, kSash);
	CHECK_EQ(f.second->frame.y, 150 + kSash);
	CHECK_EQ(f.second->frame.height, 150);
}

TEST(splitter_cross_axis_follows_the_ordinary_crossalign_rules)
{
	// A leaf pane defaults to Start at its intrinsic cross size; Expand()
	// stretches it. The sash carries Expand() from the widget, so it always
	// spans -- otherwise it would be a 6x0 rectangle nobody could grab.
	Fixture f(Orientation::Horizontal, { 120, 40 }, { 200, 60 },
		LayoutFlags{}, LayoutFlags().Expand());
	f.run({ 0, 0, 406, 100 });

	CHECK_EQ(f.first->frame.height, 40);
	CHECK_EQ(f.second->frame.height, 100);
	CHECK_EQ(f.sash->frame.height, 100);
}

TEST(splitter_container_panes_stretch_across_the_cross_axis)
{
	int leftTag = 0;
	int sashTag = 0;
	int rightTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(leftTag), { 100, 30 });
	mock.setSize(fake(sashTag), { kSash, 0 });
	mock.setSize(fake(rightTag), { 100, 30 });

	auto splitter = makeSplitter(Orientation::Horizontal);
	LayoutNode& left = splitter->add(makeBox(Orientation::Vertical));
	left.add(makeLeaf(fake(leftTag)));
	splitter->add(makeLeaf(fake(sashTag), LayoutFlags().Expand()));
	LayoutNode& right = splitter->add(makeBox(Orientation::Vertical));
	right.add(makeLeaf(fake(rightTag)));

	engine.measure(*splitter, kLoose);
	engine.arrange(*splitter, { 0, 0, 406, 120 });

	// containers default to Stretch on the cross axis -- "tallest wins" in a row
	CHECK_EQ(left.frame.height, 120);
	CHECK_EQ(right.frame.height, 120);
}

TEST(splitter_borders_become_gaps_with_no_default_of_their_own)
{
	// A splitter has a 6 px divider already, so unlike a Box it does NOT fall
	// back to kDefaultGap: with no borders the panes sit flush against the sash.
	Fixture flush(Orientation::Horizontal, { 120, 80 }, { 200, 60 });
	CHECK_EQ(flush.engine.measure(*flush.splitter, kLoose).width, 120 + kSash + 200);

	Fixture spaced(Orientation::Horizontal, { 120, 80 }, { 200, 60 },
		LayoutFlags().Border(Side::Right, 10), LayoutFlags().Border(Side::Left, 4));
	// 10 before the sash (widest facing margin), 4 after it
	CHECK_EQ(spaced.engine.measure(*spaced.splitter, kLoose).width,
		120 + 10 + kSash + 4 + 200);

	spaced.run({ 0, 0, 420, 100 });
	CHECK_EQ(spaced.sash->frame.x, spaced.first->frame.width + 10);
	CHECK_EQ(spaced.second->frame.x, spaced.sash->frame.x + kSash + 4);
}

TEST(splitter_disabling_cascades_to_both_panes_and_the_sash)
{
	bool locked = false;
	Fixture f(Orientation::Horizontal, { 120, 80 }, { 200, 60 });
	f.splitter->split.position.snapshot(150);
	f.splitter->disabled.bind(locked);
	f.run({ 0, 0, 406, 100 });

	CHECK(!f.sash->isDisabledEffective());
	locked = true;
	CHECK(f.first->isDisabledEffective());
	CHECK(f.sash->isDisabledEffective());
	CHECK(f.second->isDisabledEffective());
}

TEST(splitter_inside_a_box_takes_the_leftover_when_proportioned)
{
	int firstTag = 0;
	int sashTag = 0;
	int secondTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(firstTag), { 100, 40 });
	mock.setSize(fake(sashTag), { kSash, 0 });
	mock.setSize(fake(secondTag), { 100, 40 });

	auto root = makeBox(Orientation::Vertical);
	LayoutNode& splitter = root->add(makeSplitter(Orientation::Horizontal,
		LayoutFlags().Proportion(1).Expand()));
	splitter.add(makeLeaf(fake(firstTag)));
	splitter.add(makeLeaf(fake(sashTag), LayoutFlags().Expand()));
	splitter.add(makeLeaf(fake(secondTag)));

	engine.run(*root, { 606, 200 });
	CHECK_EQ(splitter.frame.width, 606);
	CHECK_EQ(splitter.split.resolved, 300);
	CHECK_EQ(splitter.children[2]->frame.x + splitter.children[2]->frame.width, 606);
}

TEST(splitter_second_pass_reproduces_identical_frames)
{
	// What wx and Qt do on every resize and every sash drag: the retained tree
	// is measured and arranged again, and must not creep.
	Fixture f(Orientation::Horizontal, { 120, 80 }, { 200, 60 });
	f.splitter->split.position.snapshot(175);
	f.run({ 0, 0, 406, 100 });

	const Rect firstFrame = f.first->frame;
	const Rect sashFrame = f.sash->frame;
	const Rect secondFrame = f.second->frame;

	f.run({ 0, 0, 406, 100 });
	CHECK(f.first->frame == firstFrame);
	CHECK(f.sash->frame == sashFrame);
	CHECK(f.second->frame == secondFrame);
}
