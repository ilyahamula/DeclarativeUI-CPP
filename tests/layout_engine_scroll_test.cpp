#include "test_framework.hpp"

#include "frameworks_core/LayoutEngine.hpp"
#include "mock_layout_backend.hpp"

// ScrollPanel measure/arrange (requirements.md R3), engine half only: the panel
// decides how big the viewport is and how big a virtual rect the content gets
// inside it. Turning that into a real scrolling scope is T1.4b; nothing here
// depends on a backend beyond the scrollbar gutter, which arrives through
// containerInsets like any other chrome.

namespace
{

ControlWrapper* fake(int& tag)
{
	return reinterpret_cast<ControlWrapper*>(&tag);
}

const Constraints kLoose { 10000, 10000 };
constexpr int kViewport = LayoutEngine::kDefaultScrollViewport; // 240

} // namespace

TEST(scroll_caps_the_scrolling_axis_at_the_default_viewport)
{
	int content = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(content), { 300, 600 });

	auto panel = makeScrollPanel(ScrollAxis::Vertical);
	panel->add(makeLeaf(fake(content)));

	const Size desired = engine.measure(*panel, kLoose);
	// vertical scrolls -> capped; horizontal does not -> content width
	CHECK_EQ(desired.height, kViewport);
	CHECK_EQ(desired.width, 300);
}

TEST(scroll_short_content_keeps_its_own_size)
{
	// R3.1 is a min(), not a fixed height: content smaller than the viewport
	// does not inflate the panel to 240.
	int content = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(content), { 200, 90 });

	auto panel = makeScrollPanel(ScrollAxis::Vertical);
	panel->add(makeLeaf(fake(content)));

	CHECK(engine.measure(*panel, kLoose) == (Size { 200, 90 }));
}

TEST(scroll_maxsize_overrides_the_default_viewport)
{
	int content = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(content), { 300, 600 });

	auto panel = makeScrollPanel(ScrollAxis::Vertical, LayoutFlags().MaxSize({ 400, 120 }));
	panel->add(makeLeaf(fake(content)));

	const Size desired = engine.measure(*panel, kLoose);
	CHECK_EQ(desired.height, 120);
	CHECK_EQ(desired.width, 300);
}

TEST(scroll_measures_the_child_unbounded_on_scrolling_axes_only)
{
	// The content is measured as if it had all the room it wants where it will
	// scroll, and against the real constraint where it will not -- that is what
	// keeps wrapping content wrapping to the available width.
	int content = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setMeasure(fake(content), [](const Constraints& c) {
		return Size { c.maxWidth, c.maxHeight };
	});

	auto panel = makeScrollPanel(ScrollAxis::Vertical);
	panel->add(makeLeaf(fake(content)));
	engine.measure(*panel, Constraints { 500, 400 });

	CHECK_EQ(mock.measureCalls.size(), (size_t)1);
	const Constraints& offered = mock.measureCalls.front().constraints;
	CHECK_EQ(offered.maxWidth, 500);         // non-scrolling: the panel's own
	CHECK(offered.maxHeight > 1000000);      // scrolling: effectively unbounded
}

TEST(scroll_arranges_content_at_its_full_extent_in_virtual_space)
{
	// R3.2: the viewport is 240 tall but the content is arranged at 600, so
	// there is something to scroll over.
	int content = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(content), { 300, 600 });

	auto panel = makeScrollPanel(ScrollAxis::Vertical);
	auto& child = panel->add(makeLeaf(fake(content)));

	const Size desired = engine.measure(*panel, kLoose);
	engine.arrange(*panel, { 0, 0, desired.width, desired.height });

	CHECK_EQ(panel->frame.height, kViewport);
	CHECK_EQ(child.frame.height, 600);       // the virtual extent
	CHECK_EQ(child.frame.width, 300);
	CHECK_EQ(child.frame.y, 0);              // origin is the panel's content origin
}

TEST(scroll_content_fills_the_viewport_on_the_non_scrolling_axis)
{
	// R3.3: a narrow list still spans the panel's width.
	int content = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(content), { 100, 600 });

	auto panel = makeScrollPanel(ScrollAxis::Vertical);
	auto& child = panel->add(makeLeaf(fake(content)));

	engine.measure(*panel, kLoose);
	engine.arrange(*panel, { 0, 0, 400, kViewport });
	CHECK_EQ(child.frame.width, 400);
	// ...and short content still fills the scrolling axis rather than leaving a hole
	CHECK_EQ(child.frame.height, 600);
}

TEST(scroll_short_content_fills_the_scrolling_axis_too)
{
	int content = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(content), { 100, 50 });

	auto panel = makeScrollPanel(ScrollAxis::Vertical);
	auto& child = panel->add(makeLeaf(fake(content)));

	engine.measure(*panel, kLoose);
	engine.arrange(*panel, { 0, 0, 400, 300 });
	// max(viewport, desired) -> the viewport, so nothing scrolls and no hole is left
	CHECK_EQ(child.frame.height, 300);
	CHECK_EQ(child.frame.width, 400);
}

TEST(scroll_horizontal_axis_caps_and_scrolls_the_other_way)
{
	// R3.4: the same rules, per axis.
	int content = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(content), { 900, 80 });

	auto panel = makeScrollPanel(ScrollAxis::Horizontal);
	auto& child = panel->add(makeLeaf(fake(content)));

	const Size desired = engine.measure(*panel, kLoose);
	CHECK_EQ(desired.width, kViewport);  // capped
	CHECK_EQ(desired.height, 80);        // content height, like a Box

	engine.arrange(*panel, { 0, 0, desired.width, desired.height });
	CHECK_EQ(child.frame.width, 900);    // virtual extent
	CHECK_EQ(child.frame.height, 80);
}

TEST(scroll_both_axes_cap_and_scroll_independently)
{
	int content = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(content), { 900, 600 });

	auto panel = makeScrollPanel(ScrollAxis::Both, LayoutFlags().MaxSize({ 300, 150 }));
	auto& child = panel->add(makeLeaf(fake(content)));

	const Size desired = engine.measure(*panel, kLoose);
	CHECK(desired == (Size { 300, 150 }));

	engine.arrange(*panel, { 0, 0, desired.width, desired.height });
	CHECK_EQ(child.frame.width, 900);
	CHECK_EQ(child.frame.height, 600);
}

TEST(scroll_chrome_is_a_gutter_taken_out_of_the_viewport)
{
	// The scrollbar gutter arrives as container chrome (0 on ImGui, where the
	// bar overlays; a native width on wx/Qt). It grows the panel's desired size
	// and shrinks the space the content is arranged in.
	int content = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(content), { 300, 600 });
	mock.chromeFn = [](const LayoutNode& node) -> EdgeInsets {
		return node.kind == NodeKind::ScrollPanel ? EdgeInsets { 0, 15, 0, 0 } : EdgeInsets{};
	};

	auto panel = makeScrollPanel(ScrollAxis::Vertical);
	auto& child = panel->add(makeLeaf(fake(content)));

	const Size desired = engine.measure(*panel, kLoose);
	CHECK_EQ(desired.width, 300 + 15);   // content plus the gutter
	CHECK_EQ(desired.height, kViewport);

	engine.arrange(*panel, { 0, 0, desired.width, desired.height });
	// the content is arranged inside the gutter, not under it
	CHECK_EQ(child.frame.width, 300);
	CHECK_EQ(child.frame.x, 0);
}

TEST(scroll_content_margins_are_spent_inside_the_viewport)
{
	int content = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(content), { 100, 50 });

	auto panel = makeScrollPanel(ScrollAxis::Vertical);
	auto& child = panel->add(makeLeaf(fake(content), LayoutFlags().Border(Side::All, 10)));

	const Size desired = engine.measure(*panel, kLoose);
	CHECK(desired == (Size { 120, 70 })); // content plus its own margins

	engine.arrange(*panel, { 0, 0, desired.width, desired.height });
	CHECK_EQ(child.frame.x, 10);
	CHECK_EQ(child.frame.y, 10);
	CHECK_EQ(child.frame.width, 100);
}

TEST(scroll_panel_does_not_widen_an_autofit_window)
{
	// The whole point of the panel: 600px of options inside a dialog that stays
	// the size of everything else.
	int header = 0, options = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(header), { 200, 20 });
	mock.setSize(fake(options), { 180, 600 });

	auto root = makeBox(Orientation::Vertical);
	root->add(makeLeaf(fake(header)));
	auto& panel = root->add(makeScrollPanel(ScrollAxis::Vertical));
	panel.add(makeLeaf(fake(options)));

	const Size window = engine.resolve(*root);
	// 20 (header) + 8 (default gap) + 240 (capped viewport) -- not 600
	CHECK_EQ(window.height, 20 + LayoutEngine::kDefaultGap + kViewport);
	CHECK_EQ(window.width, 200);
}

TEST(scroll_empty_panel_measures_nothing)
{
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	auto panel = makeScrollPanel(ScrollAxis::Both);
	CHECK(engine.measure(*panel, kLoose) == (Size { 0, 0 }));
	engine.arrange(*panel, { 0, 0, 100, 100 }); // must not fall over
	CHECK(panel->children.empty());
}

TEST(scroll_disabled_cascades_into_the_content)
{
	ControlWrapper widget;
	bool locked = false;

	auto panel = makeScrollPanel(ScrollAxis::Vertical);
	panel->disabled.bind(locked);
	auto& child = panel->add(makeLeaf(&widget));

	CHECK(!child.isDisabledEffective());
	locked = true;
	CHECK(child.isDisabledEffective());
}
