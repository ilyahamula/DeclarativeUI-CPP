#include "test_framework.hpp"

#include "frameworks_core/LayoutEngine.hpp"
#include "mock_layout_backend.hpp"

// Expander measure/arrange/traverse (requirements.md R5). The header is an
// ordinary leaf here -- a mock one -- which is exactly how the backends see it:
// nothing in the engine knows the header can be clicked, only that an
// Expander's second child comes and goes with the flag on the node.

namespace
{

ControlWrapper* fake(int& tag)
{
	return reinterpret_cast<ControlWrapper*>(&tag);
}

const Constraints kLoose { 10000, 10000 };
constexpr int kGap = LayoutEngine::kDefaultGap; // 8

// The tree every test below starts from: a header leaf and a one-leaf content
// box, which is the shape Expander::buildNode() emits (the content always hangs
// from a container, so the backends have a scope to hide).
struct Fixture
{
	int headerTag = 0;
	int contentTag = 0;

	MockLayoutBackend mock;
	LayoutEngine engine { mock };
	std::unique_ptr<LayoutNode> expander;
	LayoutNode* header = nullptr;
	LayoutNode* content = nullptr;
	LayoutNode* inner = nullptr;

	Fixture(Size headerSize, Size contentSize, bool expanded = false)
	{
		mock.setSize(fake(headerTag), headerSize);
		mock.setSize(fake(contentTag), contentSize);

		expander = makeExpander("Advanced");
		expander->expander.expanded.snapshot(expanded);
		header = &expander->add(makeLeaf(fake(headerTag), LayoutFlags().Expand()));
		content = &expander->add(makeBox(Orientation::Vertical));
		content->label = "content";
		inner = &content->add(makeLeaf(fake(contentTag)));
	}

	void run(const Rect& area)
	{
		engine.measure(*expander, kLoose);
		engine.arrange(*expander, area);
	}
};

} // namespace

TEST(expander_collapsed_measures_the_header_only)
{
	Fixture f({ 120, 24 }, { 300, 90 });

	const Size desired = f.engine.measure(*f.expander, kLoose);
	CHECK_EQ(desired.height, 24);  // no content, and no gap above it either
	CHECK_EQ(desired.width, 120);  // the wide content does not set the width
	CHECK(f.content->desired == (Size { 0, 0 }));
}

TEST(expander_expanded_measures_header_plus_content)
{
	Fixture f({ 120, 24 }, { 300, 90 }, /*expanded*/ true);

	const Size desired = f.engine.measure(*f.expander, kLoose);
	CHECK_EQ(desired.height, 24 + kGap + 90);
	CHECK_EQ(desired.width, 300); // widest wins, exactly as in a vertical Box
}

TEST(expander_content_is_never_measured_while_collapsed)
{
	Fixture f({ 120, 24 }, { 300, 90 });
	f.engine.measure(*f.expander, kLoose);

	for (const auto& call : f.mock.measureCalls)
		CHECK(call.widget != fake(f.contentTag));
	CHECK_EQ((int)f.mock.measureCalls.size(), 1); // the header, and nothing else
}

TEST(expander_arranges_the_content_below_the_header)
{
	Fixture f({ 120, 24 }, { 300, 90 }, /*expanded*/ true);
	f.run({ 0, 0, 300, 122 });

	CHECK_EQ(f.header->frame.y, 0);
	CHECK_EQ(f.header->frame.height, 24);
	CHECK_EQ(f.header->frame.width, 300); // Expand(): the whole row is clickable
	CHECK_EQ(f.content->frame.y, 24 + kGap);
	CHECK_EQ(f.content->frame.height, 90);
}

TEST(expander_collapsed_gives_the_content_a_zero_frame)
{
	// A stale rectangle is a rectangle something could still be drawn at, so
	// the fold leaves the content with nothing.
	Fixture f({ 120, 24 }, { 300, 90 });
	f.run({ 0, 0, 300, 24 });

	CHECK_EQ(f.header->frame.height, 24);
	CHECK_EQ(f.content->frame.width, 0);
	CHECK_EQ(f.content->frame.height, 0);
}

TEST(expander_traverse_skips_the_collapsed_content)
{
	Fixture f({ 120, 24 }, { 300, 90 });
	f.mock.hiddenContainers.insert(f.content); // what a backend does when collapsed
	f.engine.run(*f.expander);

	// the header is placed; the content box reports invisible and its subtree
	// is never entered -- the inactive-tab path
	CHECK_EQ(f.mock.callLog[0], std::string("begin:Advanced")); // the expander itself
	CHECK_EQ(f.mock.callLog[1], std::string("place"));          // the header leaf
	CHECK_EQ(f.mock.callLog[2], std::string("skip:content"));   // the folded content
	CHECK_EQ(f.mock.callLog[3], std::string("end:Advanced"));
	CHECK_EQ((int)f.mock.callLog.size(), 4);
	CHECK_EQ((int)f.mock.placed.size(), 1);
}

TEST(expander_traverse_enters_the_open_content)
{
	Fixture f({ 120, 24 }, { 300, 90 }, /*expanded*/ true);
	f.engine.run(*f.expander);

	CHECK_EQ(f.mock.callLog[2], std::string("begin:content"));
	CHECK_EQ(f.mock.callLog[3], std::string("place")); // the content leaf
	CHECK_EQ(f.mock.callLog[4], std::string("end:content"));
	CHECK_EQ((int)f.mock.placed.size(), 2);
}

TEST(expander_follows_a_bound_flag_on_the_next_pass)
{
	bool open = false;

	Fixture f({ 120, 24 }, { 300, 90 });
	f.expander->expander.expanded.bind(open);

	CHECK_EQ(f.engine.measure(*f.expander, kLoose).height, 24);
	open = true;
	CHECK_EQ(f.engine.measure(*f.expander, kLoose).height, 24 + kGap + 90);
	open = false;
	CHECK_EQ(f.engine.measure(*f.expander, kLoose).height, 24);
}

TEST(expander_latches_the_state_the_pass_laid_out)
{
	// `applied` is what arrange and the backends read: the live flag can be
	// flipped by a header click after measure has already decided the sizes
	// (ImGui toggles during render), and drawing content into rectangles that
	// were never budgeted for is exactly what the latch prevents.
	bool open = true;

	Fixture f({ 120, 24 }, { 300, 90 });
	f.expander->expander.expanded.bind(open);
	f.engine.measure(*f.expander, kLoose);
	CHECK(f.expander->expander.applied);

	open = false; // flipped after the measure pass
	f.engine.arrange(*f.expander, { 0, 0, 300, 122 });
	CHECK_EQ(f.content->frame.height, 90); // still the layout measure produced
}

TEST(expander_auto_fit_root_shrinks_to_the_header_when_closed)
{
	// R5.2: the dialog follows the section. Same tree, same content, one flag.
	Fixture f({ 120, 24 }, { 300, 90 }, /*expanded*/ true);
	const Size openSize = f.engine.resolve(*f.expander);

	f.expander->expander.expanded.snapshot(false);
	const Size closedSize = f.engine.resolve(*f.expander);

	CHECK_EQ(openSize.height, 24 + kGap + 90);
	CHECK_EQ(closedSize.height, 24);
	CHECK_EQ(closedSize.width, 120);
}

TEST(expander_disabled_cascades_into_header_and_content)
{
	// Real wrappers here: isDisabledEffective() asks the leaf's own widget as
	// well as the chain above it, which the tagged stand-ins cannot answer.
	ControlWrapper headerWidget;
	ControlWrapper contentWidget;
	bool locked = false;

	MockLayoutBackend mock;
	auto expander = makeExpander("Advanced");
	expander->disabled.bind(locked);
	auto& header = expander->add(makeLeaf(&headerWidget, LayoutFlags().Expand()));
	auto& content = expander->add(makeBox(Orientation::Vertical));
	auto& inner = content.add(makeLeaf(&contentWidget));

	CHECK(!header.isDisabledEffective());
	CHECK(!inner.isDisabledEffective());
	locked = true;
	CHECK(header.isDisabledEffective());
	CHECK(inner.isDisabledEffective());
}

TEST(expander_honours_the_content_margin_when_open)
{
	Fixture f({ 120, 24 }, { 300, 90 }, /*expanded*/ true);
	f.content->flags = LayoutFlags().Border(Side::Top, 20).Border(Side::Left, 10);
	f.run({ 0, 0, 310, 200 });

	// The facing margins collapse via max() exactly as in a Box: 20 beats the
	// default gap, and the left margin holds the content off the section's edge.
	CHECK_EQ(f.content->frame.y, 24 + 20);
	CHECK_EQ(f.content->frame.x, 10);
}

TEST(expander_with_no_children_measures_nothing)
{
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	auto expander = makeExpander("Empty");
	CHECK(engine.measure(*expander, kLoose) == (Size { 0, 0 }));
	engine.arrange(*expander, { 0, 0, 100, 100 }); // must not fall over
	CHECK(expander->children.empty());
}
