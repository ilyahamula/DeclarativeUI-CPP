#include "test_framework.hpp"

// Also proves the include path is wired for the backend-free core headers the
// engine tests (Phase 2) will build on.
#include "frameworks_core/CoreTypes/GeneralTypes.hpp"

TEST(framework_runs)
{
	CHECK(true);
	CHECK_EQ(1 + 1, 2);
}

TEST(core_types_are_backend_free)
{
	Size s { 320, 240 };
	CHECK_EQ(s.width, 320);
	CHECK_EQ(s.height, 240);

	CHECK(Side::All & Side::Left);
	CHECK(!(Side::Top & Side::Bottom));
}

TEST(layout_engine_types)
{
	CHECK(Rect{} == (Rect { 0, 0, 0, 0 }));
	CHECK((Rect { 1, 2, 3, 4 }) == (Rect { 1, 2, 3, 4 }));
	CHECK(!(Rect { 1, 2, 3, 4 } == Rect { 1, 2, 3, 5 }));

	CHECK(EdgeInsets{} == (EdgeInsets { 0, 0, 0, 0 }));
	CHECK((Constraints { 640, 480 }) == (Constraints { 640, 480 }));
	CHECK((Size { 10, 20 }) == (Size { 10, 20 }));

	CHECK(NodeKind::Leaf != NodeKind::Box);
	CHECK(Align::Start != Align::Stretch);
}
