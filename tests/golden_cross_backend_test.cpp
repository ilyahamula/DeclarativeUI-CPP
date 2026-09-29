#include "test_framework.hpp"
#include "fake_widget.hpp"

#include "frameworks_core/LayoutEngine.hpp"
#include "mock_layout_backend.hpp"

// The golden cross-backend check (T3.5): the same logical dialog tree is
// measured with wx/Qt/ImGui-shaped intrinsic sizes, and the layout RULES must
// hold identically for every profile — equalization, proportion fill,
// centering, chrome insets, flush-right spacers, auto-fit -- and, since the
// widget catalogue (T5.1), grid bands, scroll viewports behind a scrollbar
// gutter, the splitter's sash and folded expanders. Absolute pixels
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
	// A ScrollPanel's gutter: the native scrollbar thickness, reserved on the
	// cross side of each scrolling axis (style.ScrollbarSize / wxSYS_VSCROLL_X /
	// PM_ScrollBarExtent). All three reserve it; only its width differs.
	int scrollBar;
	// What the backend measures the splitter's sash at. Every backend measures
	// SplitterState::kSashThickness today, but the engine reads the MEASURED
	// width, so the checks below are written against this and not the constant.
	int sash;
};

// intrinsics shaped like each real backend (fonts/paddings differ)
const Profile kImGuiShaped {
	{ 60, 13 }, { 100, 26 }, { 90, 26 }, { 220, 40 }, { 340, 40 },
	{ 8, 8, 19, 8 }, { 2, 2, 27, 2 }, 14, SplitterState::kSashThickness
};
const Profile kWxShaped {
	{ 50, 16 }, { 80, 22 }, { 70, 24 }, { 180, 34 }, { 260, 34 },
	{ 4, 4, 18, 4 }, { 3, 3, 30, 3 }, 16, SplitterState::kSashThickness
};
const Profile kQtShaped {
	{ 55, 14 }, { 120, 24 }, { 80, 28 }, { 200, 38 }, { 300, 38 },
	{ 8, 8, 20, 8 }, { 2, 2, 28, 2 }, 15, SplitterState::kSashThickness
};

struct Golden
{
	int labelTag = 0, inputTag = 0, spacerTag = 0, btn1Tag = 0, btn2Tag = 0;
	int aTag = 0, bTag = 0, p1Tag = 0, p2Tag = 0;
	int gl1Tag = 0, gf1Tag = 0, gl2Tag = 0, gf2Tag = 0;
	int sp1Tag = 0, sashTag = 0, sp2Tag = 0;
	int hdrOpenTag = 0, secOpenTag = 0, hdrShutTag = 0, secShutTag = 0;
	int listTag = 0, noteTag = 0, stripTag = 0;

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
	LayoutNode* form = nullptr;
	LayoutNode* formLabel1 = nullptr;
	LayoutNode* formField1 = nullptr;
	LayoutNode* formLabel2 = nullptr;
	LayoutNode* formField2 = nullptr;
	LayoutNode* panes = nullptr;
	LayoutNode* paneLeft = nullptr;
	LayoutNode* sash = nullptr;
	LayoutNode* paneRight = nullptr;
	LayoutNode* openSection = nullptr;
	LayoutNode* openHeader = nullptr;
	LayoutNode* openBody = nullptr;
	LayoutNode* shutSection = nullptr;
	LayoutNode* shutHeader = nullptr;
	LayoutNode* shutBody = nullptr;
	LayoutNode* listPanel = nullptr;
	LayoutNode* list = nullptr;
	LayoutNode* notePanel = nullptr;
	LayoutNode* note = nullptr;
	LayoutNode* stripPanel = nullptr;
	LayoutNode* strip = nullptr;
	Size window { 0, 0 };

	ControlWrapper* fake(int& tag) { return testfw::fakeWidget(&tag); }

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
		// the grid's first column is driven by the WIDER of the two labels
		mock.setSize(fake(gl1Tag), p.label);
		mock.setSize(fake(gf1Tag), p.input);
		mock.setSize(fake(gl2Tag), Size { p.label.width * 2, p.label.height });
		mock.setSize(fake(gf2Tag), p.input);
		// The splitter's panes: profile-shaped, but deliberately narrow enough
		// that the splitter never becomes the widest thing in the column --
		// what is under test is how it DIVIDES the band, not how it sets it.
		const Size pane { p.contentA.width / 3, 30 };
		mock.setSize(fake(sp1Tag), pane);
		mock.setSize(fake(sashTag), Size { p.sash, 0 });
		mock.setSize(fake(sp2Tag), pane);
		// The two expanders share a header size and a body size, so the only
		// thing separating them is the flag on the node.
		mock.setSize(fake(hdrOpenTag), p.label);
		mock.setSize(fake(hdrShutTag), p.label);
		mock.setSize(fake(secOpenTag), p.contentA);
		mock.setSize(fake(secShutTag), p.contentA);
		// Scroll content: a list far taller than the default viewport, a note
		// shorter than it, and a strip far wider than anything else in the
		// dialog -- the one that would widen an auto-fit window if the panel
		// let it.
		mock.setSize(fake(listTag), Size { p.contentA.width, LayoutEngine::kDefaultScrollViewport * 3 });
		mock.setSize(fake(noteTag), p.contentA);
		mock.setSize(fake(stripTag), Size { p.contentB.width * 3, p.contentA.height });
		mock.chromeFn = [p](const LayoutNode& node) -> EdgeInsets {
			if (node.kind == NodeKind::GroupBox)
				return p.groupChrome;
			if (node.kind == NodeKind::TabPanel)
				return p.tabChrome;
			if (node.kind == NodeKind::ScrollPanel)
				return {
					0,
					scrollsVertically(node.scroll) ? p.scrollBar : 0,
					0,
					scrollsHorizontally(node.scroll) ? p.scrollBar : 0,
				};
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

		// a 2-column form: labels in column 0, Proportion(1) fields in column 1
		form = &root->add(makeGrid(2));
		formLabel1 = &form->add(makeLeaf(fake(gl1Tag), LayoutFlags().CenterVertical()));
		formField1 = &form->add(makeLeaf(fake(gf1Tag), LayoutFlags().Proportion(1).Expand()));
		formLabel2 = &form->add(makeLeaf(fake(gl2Tag), LayoutFlags().CenterVertical()));
		formField2 = &form->add(makeLeaf(fake(gf2Tag), LayoutFlags().Expand()));

		// a splitter with the sash left where it defaults: at half the pane space
		panes = &root->add(makeSplitter(Orientation::Horizontal,
			LayoutFlags().Expand().MinSize({ -1, 60 })));
		paneLeft = &panes->add(makeLeaf(fake(sp1Tag), LayoutFlags().Expand()));
		sash = &panes->add(makeLeaf(fake(sashTag), LayoutFlags().Expand()));
		paneRight = &panes->add(makeLeaf(fake(sp2Tag), LayoutFlags().Expand()));

		// two expanders over identical content: one open, one closed
		openSection = &root->add(makeExpander("Open"));
		openSection->expander.expanded.snapshot(true);
		openHeader = &openSection->add(makeLeaf(fake(hdrOpenTag), LayoutFlags().Expand()));
		openBody = &openSection->add(makeBox(Orientation::Vertical));
		openBody->add(makeLeaf(fake(secOpenTag)));

		shutSection = &root->add(makeExpander("Shut"));
		shutSection->expander.expanded.snapshot(false);
		shutHeader = &shutSection->add(makeLeaf(fake(hdrShutTag), LayoutFlags().Expand()));
		shutBody = &shutSection->add(makeBox(Orientation::Vertical));
		shutBody->add(makeLeaf(fake(secShutTag)));

		// three scroll panels: a long vertical list, a short one that must not
		// be inflated to the viewport, and a wide horizontal strip
		listPanel = &root->add(makeScrollPanel(ScrollAxis::Vertical));
		list = &listPanel->add(makeLeaf(fake(listTag)));
		notePanel = &root->add(makeScrollPanel(ScrollAxis::Vertical));
		note = &notePanel->add(makeLeaf(fake(noteTag)));
		stripPanel = &root->add(makeScrollPanel(ScrollAxis::Horizontal));
		strip = &stripPanel->add(makeLeaf(fake(stripTag)));

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
	CHECK_EQ(g.form->frame.width, band);
	CHECK_EQ(g.panes->frame.width, band);
	CHECK_EQ(g.openSection->frame.width, band);
	CHECK_EQ(g.shutSection->frame.width, band);
	CHECK_EQ(g.listPanel->frame.width, band);
	CHECK_EQ(g.notePanel->frame.width, band);
	CHECK_EQ(g.stripPanel->frame.width, band);
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

	// grid form: the two rows share one column line, so the fields start at the
	// same x however wide each label is; the Proportion(1) column absorbs the
	// leftover out to the grid's right edge; the shorter label is centred in the
	// column band while its row keeps the taller label's height
	CHECK_EQ(g.formField1->frame.x, g.formField2->frame.x);
	CHECK_EQ(g.formLabel1->frame.x, g.formLabel2->frame.x);
	CHECK_EQ(rightEdge(*g.formField1), rightEdge(*g.form));
	CHECK_EQ(rightEdge(*g.formField2), rightEdge(*g.form));
	// column 0 is the widest label; the narrower one keeps its intrinsic width
	CHECK_EQ(g.formLabel1->frame.width, p.label.width);
	CHECK_EQ(g.formLabel2->frame.width, p.label.width * 2);
	CHECK_EQ(g.formField1->frame.x - g.form->frame.x,
		p.label.width * 2 + 8); // widest label + kDefaultGap gutter

	// splitter: the sash is the divider and nothing else separates the panes, so
	// the three frames are flush and span the band exactly. The default position
	// halves whatever the panes have left once the sash is spent, which makes
	// the two panes equal on every profile even though the band is not.
	const int sashWidth = p.sash;
	CHECK_EQ(g.sash->frame.width, sashWidth);
	CHECK_EQ(g.sash->frame.x, rightEdge(*g.paneLeft));
	CHECK_EQ(g.paneRight->frame.x, rightEdge(*g.sash));
	CHECK_EQ(rightEdge(*g.paneRight), rightEdge(*g.panes));
	CHECK_EQ(g.paneLeft->frame.width, (band - sashWidth) / 2);
	CHECK_EQ(g.panes->split.resolved, (band - sashWidth) / 2);
	// Expand() on all three: every one of them spans the splitter's height
	CHECK_EQ(g.paneLeft->frame.height, g.panes->frame.height);
	CHECK_EQ(g.sash->frame.height, g.panes->frame.height);
	CHECK_EQ(g.paneRight->frame.height, g.panes->frame.height);

	// expanders: open, the section is its header plus its body one default gap
	// below; closed, it is EXACTLY its header -- no body, no margins, and no gap
	// where the body would have been -- and the body keeps a zero frame. Both
	// headers span the section, so the whole row is clickable on every backend.
	CHECK_EQ(g.openSection->frame.height,
		p.label.height + LayoutEngine::kDefaultGap + p.contentA.height);
	CHECK_EQ(g.shutSection->frame.height, p.label.height);
	CHECK_EQ(g.openHeader->frame.width, band);
	CHECK_EQ(g.shutHeader->frame.width, band);
	CHECK_EQ(g.openBody->frame.y,
		g.openHeader->frame.y + p.label.height + LayoutEngine::kDefaultGap);
	CHECK_EQ(g.shutBody->frame.width, 0);
	CHECK_EQ(g.shutBody->frame.height, 0);
	CHECK(g.openSection->expander.applied);
	CHECK(!g.shutSection->expander.applied);

	// scroll panels: a scrolling axis is capped at the default viewport and the
	// gutter is reserved beside it whether or not anything overflows. The
	// content gets a VIRTUAL rect at the panel's content origin -- the viewport
	// on the non-scrolling axis (so the list still fills the band minus the
	// gutter), its full desired extent on the scrolling one.
	CHECK_EQ(g.listPanel->frame.height, LayoutEngine::kDefaultScrollViewport);
	CHECK_EQ(g.list->frame.x, g.listPanel->frame.x);
	CHECK_EQ(g.list->frame.y, g.listPanel->frame.y);
	CHECK_EQ(g.list->frame.width, band - p.scrollBar);
	CHECK_EQ(g.list->frame.height, LayoutEngine::kDefaultScrollViewport * 3);
	// short content: the viewport is a min(), not a fixed height -- the panel
	// keeps the note's height, and the gutter is still there
	CHECK_EQ(g.notePanel->frame.height, p.contentA.height);
	CHECK_EQ(g.note->frame.width, band - p.scrollBar);
	CHECK_EQ(g.note->frame.height, p.contentA.height);
	// horizontal: the strip is three times the band, yet the band (checked
	// above) is still set by box B -- the panel caps its width at the viewport
	// and the column stretches it from there. The gutter goes underneath.
	CHECK(g.strip->frame.width == p.contentB.width * 3);
	CHECK_EQ(g.strip->frame.x, g.stripPanel->frame.x);
	CHECK_EQ(g.stripPanel->frame.height, p.contentA.height + p.scrollBar);
	CHECK_EQ(g.strip->frame.height, p.contentA.height);

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
	const Rect formFieldFrame = g.formField1->frame;
	const Rect sashFrame = g.sash->frame;
	const Rect openBodyFrame = g.openBody->frame;
	const Rect listFrame = g.list->frame;
	const Rect stripFrame = g.strip->frame;
	const Size window2 = g.engine.run(*g.root);
	CHECK(window2 == g.window);
	CHECK(g.row->frame == rowFrame);
	CHECK(g.input->frame == inputFrame);
	CHECK(g.boxB->frame == boxBFrame);
	CHECK(g.formField1->frame == formFieldFrame);
	CHECK(g.sash->frame == sashFrame);
	CHECK(g.openBody->frame == openBodyFrame);
	CHECK(g.list->frame == listFrame);
	CHECK(g.strip->frame == stripFrame);
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
