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

TEST(sizegroup_equalizes_across_parents)
{
	int aTag = 0, bTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(aTag), { 100, 30 });
	mock.setSize(fake(bTag), { 160, 40 });

	// the grouped leaves live under different parents
	auto root = makeBox(Orientation::Vertical);
	auto& left = root->add(makeBox(Orientation::Vertical));
	auto& a = left.add(makeLeaf(fake(aTag), LayoutFlags().SizeGroup(1)));
	auto& right = root->add(makeBox(Orientation::Vertical));
	auto& b = right.add(makeLeaf(fake(bTag), LayoutFlags().SizeGroup(1)));

	engine.measure(*root, kLoose);
	CHECK(a.desired == (Size { 100, 30 }));

	engine.reconcileSizeGroups(*root);
	// both members raised to the per-axis group maximum
	CHECK(a.desired == (Size { 160, 40 }));
	CHECK(b.desired == (Size { 160, 40 }));
	// their parents were re-summed too
	CHECK(left.desired == (Size { 160, 40 }));
}

TEST(sizegroup_repropagates_to_autofit_root)
{
	// The stale-ancestor case from architecture.md: raising a grouped node
	// must grow every ancestor, or an auto-fit dialog sizes to stale numbers.
	int aTag = 0, xTag = 0, bTag = 0, yTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(aTag), { 100, 20 }); // grouped, will be raised
	mock.setSize(fake(xTag), { 50, 20 });
	mock.setSize(fake(bTag), { 160, 20 }); // grouped, sets the max
	mock.setSize(fake(yTag), { 10, 20 });

	auto root = makeBox(Orientation::Vertical);
	auto& row1 = root->add(makeBox(Orientation::Horizontal));
	row1.add(makeLeaf(fake(aTag), LayoutFlags().SizeGroup(7)));
	row1.add(makeLeaf(fake(xTag)));
	auto& row2 = root->add(makeBox(Orientation::Horizontal));
	row2.add(makeLeaf(fake(bTag), LayoutFlags().SizeGroup(7)));
	row2.add(makeLeaf(fake(yTag)));

	const int gap = LayoutEngine::kDefaultGap;
	engine.measure(*root, kLoose);
	// pre-reconcile: row2 is the widest, root cross = 178
	CHECK(row1.desired == (Size { 100 + gap + 50, 20 }));
	CHECK_EQ(root->desired.width, 160 + gap + 10);

	engine.reconcileSizeGroups(*root);
	// a raised to 160 -> row1 grows to 218 -> root must follow
	CHECK(row1.desired == (Size { 160 + gap + 50, 20 }));
	CHECK_EQ(root->desired.width, 160 + gap + 50);
}

TEST(sizegroup_respects_member_max_size)
{
	int aTag = 0, bTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(aTag), { 100, 30 });
	mock.setSize(fake(bTag), { 200, 30 });

	auto root = makeBox(Orientation::Vertical);
	auto& a = root->add(makeLeaf(fake(aTag), LayoutFlags().SizeGroup(1).MaxSize({ 150, 30 })));
	auto& b = root->add(makeLeaf(fake(bTag), LayoutFlags().SizeGroup(1)));

	engine.measure(*root, kLoose);
	engine.reconcileSizeGroups(*root);

	// raised toward 200 but clamped by its own MaxSize
	CHECK(a.desired == (Size { 150, 30 }));
	CHECK(b.desired == (Size { 200, 30 }));
}

TEST(sizegroup_grouped_containers_equalize)
{
	int aTag = 0, bTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(aTag), { 120, 30 });
	mock.setSize(fake(bTag), { 220, 60 });

	// two group boxes in different columns, equalized by group id
	auto root = makeBox(Orientation::Horizontal);
	auto& colLeft = root->add(makeBox(Orientation::Vertical));
	auto& boxA = colLeft.add(makeGroupBox(Orientation::Vertical, "A", LayoutFlags().SizeGroup(3)));
	boxA.add(makeLeaf(fake(aTag)));
	auto& colRight = root->add(makeBox(Orientation::Vertical));
	auto& boxB = colRight.add(makeGroupBox(Orientation::Vertical, "B", LayoutFlags().SizeGroup(3)));
	boxB.add(makeLeaf(fake(bTag)));

	engine.measure(*root, kLoose);
	CHECK(boxA.desired == (Size { 120, 30 }));

	engine.reconcileSizeGroups(*root);
	CHECK(boxA.desired == (Size { 220, 60 }));
	CHECK(boxB.desired == (Size { 220, 60 }));
	CHECK(colLeft.desired == (Size { 220, 60 }));
	CHECK_EQ(root->desired.width, 220 + LayoutEngine::kDefaultGap + 220);
}

TEST(sizegroup_no_groups_is_noop)
{
	int aTag = 0;
	MockLayoutBackend mock;
	LayoutEngine engine(mock);
	mock.setSize(fake(aTag), { 80, 20 });

	auto root = makeBox(Orientation::Vertical);
	root->add(makeLeaf(fake(aTag)));

	engine.measure(*root, kLoose);
	const Size before = root->desired;
	engine.reconcileSizeGroups(*root);
	CHECK(root->desired == before);
}
