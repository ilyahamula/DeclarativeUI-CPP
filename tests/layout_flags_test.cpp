#include "test_framework.hpp"

#include "frameworks_core/LayoutFlags.hpp"

TEST(flags_defaults)
{
	LayoutFlags f;
	CHECK_EQ(f.proportion(), 0);
	CHECK(f.border() == EdgeInsets{});
	CHECK(!f.minSize());
	CHECK(!f.maxSize());
	CHECK(!f.sizeGroup());
	CHECK(!f.autoGrow());
}

TEST(kind_default_cross_align)
{
	LayoutFlags f;
	CHECK(f.crossAlign(NodeKind::Leaf, Orientation::Horizontal) == Align::Start);
	CHECK(f.crossAlign(NodeKind::Leaf, Orientation::Vertical) == Align::Start);
	CHECK(f.crossAlign(NodeKind::Box, Orientation::Horizontal) == Align::Stretch);
	CHECK(f.crossAlign(NodeKind::GroupBox, Orientation::Vertical) == Align::Stretch);
	CHECK(f.crossAlign(NodeKind::TabPanel, Orientation::Vertical) == Align::Stretch);
}

TEST(expand_resolves_to_stretch)
{
	LayoutFlags leaf;
	leaf.Expand();
	CHECK(leaf.crossAlign(NodeKind::Leaf, Orientation::Vertical) == Align::Stretch);
	CHECK(leaf.crossAlign(NodeKind::Leaf, Orientation::Horizontal) == Align::Stretch);

	// Expand on a container is a no-op: containers stretch by default.
	LayoutFlags expanded;
	expanded.Expand();
	LayoutFlags plain;
	CHECK(expanded.crossAlign(NodeKind::GroupBox, Orientation::Vertical)
		== plain.crossAlign(NodeKind::GroupBox, Orientation::Vertical));
}

TEST(center_resolution_per_orientation)
{
	// CenterVertical addresses the cross axis of a row only.
	LayoutFlags v;
	v.CenterVertical();
	CHECK(v.crossAlign(NodeKind::Leaf, Orientation::Horizontal) == Align::Center);
	CHECK(v.crossAlign(NodeKind::Leaf, Orientation::Vertical) == Align::Start);

	// CenterHorizontal addresses the cross axis of a column only.
	LayoutFlags h;
	h.CenterHorizontal();
	CHECK(h.crossAlign(NodeKind::Leaf, Orientation::Vertical) == Align::Center);
	CHECK(h.crossAlign(NodeKind::Leaf, Orientation::Horizontal) == Align::Start);

	LayoutFlags both;
	both.Center();
	CHECK(both.crossAlign(NodeKind::Leaf, Orientation::Horizontal) == Align::Center);
	CHECK(both.crossAlign(NodeKind::Leaf, Orientation::Vertical) == Align::Center);
}

TEST(center_wins_over_expand)
{
	LayoutFlags f;
	f.Expand().CenterVertical();
	CHECK(f.crossAlign(NodeKind::Leaf, Orientation::Horizontal) == Align::Center);
	// CenterVertical does not apply in a column; Expand still does.
	CHECK(f.crossAlign(NodeKind::Leaf, Orientation::Vertical) == Align::Stretch);
}

TEST(border_accumulates_per_side)
{
	LayoutFlags f;
	f.Border(Side::All, 5);
	CHECK(f.border() == (EdgeInsets { 5, 5, 5, 5 }));

	f.Border(Side::Left, 3);
	f.Border(Side::Right | Side::Top, 7);
	CHECK(f.border() == (EdgeInsets { 3, 7, 7, 5 }));

	// legacy accessors read the same storage
	CHECK_EQ(f.borderLeft(), 3);
	CHECK_EQ(f.borderRight(), 7);
	CHECK_EQ(f.borderTop(), 7);
	CHECK_EQ(f.borderBottom(), 5);
}

TEST(builder_setters)
{
	LayoutFlags f;
	f.Proportion(2).MinSize({ 10, 20 }).MaxSize({ 100, 200 }).SizeGroup(7).AutoGrow();
	CHECK_EQ(f.proportion(), 2);
	CHECK(f.minSize() && *f.minSize() == (Size { 10, 20 }));
	CHECK(f.maxSize() && *f.maxSize() == (Size { 100, 200 }));
	CHECK(f.sizeGroup() && *f.sizeGroup() == 7);
	CHECK(f.autoGrow());
}
