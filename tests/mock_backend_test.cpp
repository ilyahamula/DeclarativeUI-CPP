#include "test_framework.hpp"

#include "mock_layout_backend.hpp"

static ControlWrapper* fake(int& tag)
{
	return reinterpret_cast<ControlWrapper*>(&tag);
}

TEST(mock_backend_scripted_sizes)
{
	int aTag = 0, bTag = 0, cTag = 0;
	MockLayoutBackend mock;
	mock.setSize(fake(aTag), { 80, 20 });

	auto leafA = makeLeaf(fake(aTag));
	auto leafB = makeLeaf(fake(bTag));

	// fixed scripted size, constraints ignored
	CHECK(mock.measure(*leafA, { 640, 480 }) == (Size { 80, 20 }));
	// unscripted widget falls back to defaultSize
	CHECK(mock.measure(*leafB, { 640, 480 }) == (Size { 0, 0 }));
	mock.defaultSize = { 10, 10 };
	CHECK(mock.measure(*leafB, { 640, 480 }) == (Size { 10, 10 }));

	// width-dependent height (wrapping text): narrower constraint -> taller
	mock.setMeasure(fake(cTag), [](const Constraints& c) {
		const int area = 6000;
		const int width = c.maxWidth < 300 ? c.maxWidth : 300;
		return Size { width, area / width };
	});
	auto leafC = makeLeaf(fake(cTag));
	CHECK(mock.measure(*leafC, { 300, 1000 }) == (Size { 300, 20 }));
	CHECK(mock.measure(*leafC, { 100, 1000 }) == (Size { 100, 60 }));

	// every call was recorded with its constraints
	CHECK_EQ(mock.measureCalls.size(), (size_t)5);
	CHECK(mock.measureCalls[0].widget == fake(aTag));
	CHECK(mock.measureCalls[0].constraints == (Constraints { 640, 480 }));
	CHECK(mock.measureCalls[4].constraints == (Constraints { 100, 1000 }));
}

TEST(mock_backend_records_placement_and_chrome)
{
	int aTag = 0;
	MockLayoutBackend mock;

	auto box = makeGroupBox(Orientation::Vertical, "Credentials");
	auto& leaf = box->add(makeLeaf(fake(aTag)));

	mock.beginContainer(*box, { 10, 10, 200, 100 });
	mock.place(leaf, { 15, 25, 190, 30 });
	mock.endContainer(*box);

	CHECK(mock.containerFrames[box.get()] == (Rect { 10, 10, 200, 100 }));
	CHECK(mock.frameOf(fake(aTag)) == (Rect { 15, 25, 190, 30 }));

	// traversal order is observable: begin -> place -> end
	CHECK_EQ(mock.callLog.size(), (size_t)3);
	CHECK_EQ(mock.callLog[0], std::string("begin:Credentials"));
	CHECK_EQ(mock.callLog[1], std::string("place"));
	CHECK_EQ(mock.callLog[2], std::string("end:Credentials"));
}
