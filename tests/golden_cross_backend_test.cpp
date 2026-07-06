#include "test_framework.hpp"

#include "frameworks_core/LayoutEngine.hpp"
#include "mock_layout_backend.hpp"

// The golden cross-backend check (T3.5): the same logical dialog tree is
// measured with wx/Qt/ImGui-shaped intrinsic sizes, and the layout RULES must
// hold identically for every profile — equalization, proportion fill,
// centering, chrome insets, flush-right spacers, auto-fit. Absolute pixels
// legitimately differ per profile ("identical rules, not identical pixels").

namespace
{

struct Profile
{
	Size label;
	Size input;
	Size button;
	Size contentA;
	Size contentB; // wider than contentA: drives the column width
	EdgeInsets groupChrome;
	EdgeInsets tabChrome;
};

// intrinsics shaped like each real backend (fonts/paddings differ)
const Profile kImGuiShaped {
	{ 60, 13 }, { 100, 26 }, { 90, 26 }, { 220, 40 }, { 340, 40 },
	{ 8, 8, 19, 8 }, { 2, 2, 27, 2 }
};
const Profile kWxShaped {
	{ 50, 16 }, { 80, 22 }, { 70, 24 }, { 180, 34 }, { 260, 34 },
	{ 4, 4, 18, 4 }, { 3, 3, 30, 3 }
};
const Profile kQtShaped {
	{ 55, 14 }, { 120, 24 }, { 80, 28 }, { 200, 38 }, { 300, 38 },
	{ 8, 8, 20, 8 }, { 2, 2, 28, 2 }
};

struct Golden
{
	int labelTag = 0, inputTag = 0, spacerTag = 0, btn1Tag = 0, btn2Tag = 0;
	int aTag = 0, bTag = 0, p1Tag = 0, p2Tag = 0;

	MockLayoutBackend mock;
	LayoutEngine engine { mock };
	std::unique_ptr<LayoutNode> root;
	LayoutNode* row = nullptr;
	LayoutNode* label = nullptr;
	LayoutNode* input = nullptr;
	LayoutNode* boxA = nullptr;
	LayoutNode* boxB = nullptr;
	LayoutNode* leafA = nullptr;
	LayoutNode* leafB = nullptr;
	LayoutNode* tabs = nullptr;
	LayoutNode* page1 = nullptr;
	LayoutNode* page2 = nullptr;
	LayoutNode* buttons = nullptr;
	LayoutNode* spacer = nullptr;
	LayoutNode* btn1 = nullptr;
	LayoutNode* btn2 = nullptr;
	Size window { 0, 0 };

	ControlWrapper* fake(int& tag) { return reinterpret_cast<ControlWrapper*>(&tag); }

	void build(const Profile& p)
	{
		mock.setSize(fake(labelTag), p.label);
		mock.setSize(fake(inputTag), p.input);
		mock.setSize(fake(spacerTag), { 0, 0 });
		mock.setSize(fake(btn1Tag), p.button);
		mock.setSize(fake(btn2Tag), p.button);
		mock.setSize(fake(aTag), p.contentA);
		mock.setSize(fake(bTag), p.contentB);
		mock.setSize(fake(p1Tag), p.contentA);
		mock.setSize(fake(p2Tag), p.contentA);
		mock.chromeFn = [p](const LayoutNode& node) -> EdgeInsets {
			if (node.kind == NodeKind::GroupBox)
				return p.groupChrome;
			if (node.kind == NodeKind::TabPanel)
				return p.tabChrome;
			return {};
		};

		// the demo-shaped logical tree, identical for every profile
		root = makeBox(Orientation::Vertical, LayoutFlags().Border(Side::All, 10));

		row = &root->add(makeBox(Orientation::Horizontal));
		label = &row->add(makeLeaf(fake(labelTag), LayoutFlags().CenterVertical().Border(Side::Right, 5)));
		input = &row->add(makeLeaf(fake(inputTag), LayoutFlags().Proportion(1).Expand()));

		boxA = &root->add(makeGroupBox(Orientation::Vertical, "A"));
		leafA = &boxA->add(makeLeaf(fake(aTag)));
		boxB = &root->add(makeGroupBox(Orientation::Vertical, "B"));
		leafB = &boxB->add(makeLeaf(fake(bTag)));

		tabs = &root->add(makeTabPanel());
		page1 = &tabs->add(makeBox(Orientation::Vertical));
		page1->label = "Notes";
		page1->add(makeLeaf(fake(p1Tag)));
		page2 = &tabs->add(makeBox(Orientation::Vertical));
		page2->label = "About";
		page2->add(makeLeaf(fake(p2Tag)));

		buttons = &root->add(makeBox(Orientation::Horizontal));
		spacer = &buttons->add(makeLeaf(fake(spacerTag), LayoutFlags().Proportion(1)));
		btn1 = &buttons->add(makeLeaf(fake(btn1Tag)));
		btn2 = &buttons->add(makeLeaf(fake(btn2Tag), LayoutFlags().Border(Side::Left, 8)));

		window = engine.run(*root);
	}
};

int rightEdge(const LayoutNode& node) { return node.frame.x + node.frame.width; }

// The rules that must hold on every backend profile.
void checkGoldenInvariants(const Profile& p)
{
	Golden g;
	g.build(p);

	// auto-fit: window = root desired + the root's own margins
	CHECK(g.window == (Size { g.root->desired.width + 20, g.root->desired.height + 20 }));
	CHECK_EQ(g.root->frame.x, 10);
	CHECK_EQ(g.root->frame.y, 10);

	// widest-wins equalization: every container in the column fills the band
	const int band = g.root->frame.width;
	CHECK_EQ(g.boxA->frame.width, band);
	CHECK_EQ(g.boxB->frame.width, band);
	CHECK_EQ(g.tabs->frame.width, band);
	CHECK_EQ(g.row->frame.width, band);
	CHECK_EQ(g.buttons->frame.width, band);
	// ...and the band is driven by the widest content (box B) plus chrome
	CHECK_EQ(band, p.contentB.width + p.groupChrome.left + p.groupChrome.right);

	// group-box chrome: content leaf sits inside the chrome insets and keeps
	// its intrinsic size (leaf default = Start)
	CHECK_EQ(g.leafA->frame.x, g.boxA->frame.x + p.groupChrome.left);
	CHECK_EQ(g.leafA->frame.y, g.boxA->frame.y + p.groupChrome.top);
	CHECK(g.leafA->frame.width == p.contentA.width);

	// label+input row: explicit 5px gap, Proportion(1) input fills to the
	// row's right edge, Expand stretches it to the full band height,
	// CenterVertical centers the label in that band
	CHECK_EQ(g.input->frame.x, rightEdge(*g.label) + 5);
	CHECK_EQ(rightEdge(*g.input), rightEdge(*g.row));
	CHECK_EQ(g.input->frame.height, g.row->frame.height);
	CHECK_EQ(g.label->frame.y - g.row->frame.y,
		(g.row->frame.height - g.label->frame.height) / 2);

	// spacer row: buttons keep intrinsic sizes, the Proportion(1) spacer
	// absorbs all leftover, pushing the last button flush right
	CHECK(g.btn1->frame.width == p.button.width);
	CHECK(g.btn2->frame.width == p.button.width);
	CHECK_EQ(rightEdge(*g.btn2), rightEdge(*g.buttons));
	CHECK_EQ(g.btn2->frame.x, rightEdge(*g.btn1) + 8); // explicit border gap

	// tab pages overlap: both pages get the panel frame inset by the chrome
	const Rect pageArea {
		g.tabs->frame.x + p.tabChrome.left,
		g.tabs->frame.y + p.tabChrome.top,
		g.tabs->frame.width - p.tabChrome.left - p.tabChrome.right,
		g.tabs->frame.height - p.tabChrome.top - p.tabChrome.bottom,
	};
	CHECK(g.page1->frame == pageArea);
	CHECK(g.page2->frame == pageArea);

	// determinism: a second full pass over the retained tree (what wx/Qt do
	// on every resize/relayout) must reproduce identical frames
	const Rect rowFrame = g.row->frame;
	const Rect inputFrame = g.input->frame;
	const Rect boxBFrame = g.boxB->frame;
	const Size window2 = g.engine.run(*g.root);
	CHECK(window2 == g.window);
	CHECK(g.row->frame == rowFrame);
	CHECK(g.input->frame == inputFrame);
	CHECK(g.boxB->frame == boxBFrame);
}

} // unnamed namespace

TEST(golden_rules_imgui_shaped)
{
	checkGoldenInvariants(kImGuiShaped);
}

TEST(golden_rules_wx_shaped)
{
	checkGoldenInvariants(kWxShaped);
}

TEST(golden_rules_qt_shaped)
{
	checkGoldenInvariants(kQtShaped);
}
