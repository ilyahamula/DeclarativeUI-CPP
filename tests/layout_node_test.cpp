#include "test_framework.hpp"

#include "frameworks_core/LayoutNode.hpp"

// LayoutNode stores ControlWrapper* as an opaque back-pointer and never
// dereferences it, so distinct fake addresses are enough for structure tests
// (instantiating a real ControlWrapper would pull backend LayoutWrapper
// symbols into this backend-free target).
static ControlWrapper* fakeWidget(int& tag)
{
	return reinterpret_cast<ControlWrapper*>(&tag);
}

// Mirrors the worked example from architecture.md:
//   VStack {
//     HStack { StaticText{"Password:"}.CenterVertical().Border(Right,5),
//              PasswordInput{...}.Proportion(1).Expand() },
//     VGroupBox { "Credentials", <leaf> },
//     VGroupBox { "Options", <leaf> },
//   }
TEST(node_tree_structure)
{
	int labelTag = 0, inputTag = 0, boxTag1 = 0, boxTag2 = 0;
	ControlWrapper* labelWidget = fakeWidget(labelTag);
	ControlWrapper* inputWidget = fakeWidget(inputTag);

	auto root = makeBox(Orientation::Vertical, LayoutFlags().Border(Side::All, 10));

	auto& row = root->add(makeBox(Orientation::Horizontal));
	row.add(makeLeaf(labelWidget, LayoutFlags().CenterVertical().Border(Side::Right, 5)));
	row.add(makeLeaf(inputWidget, LayoutFlags().Proportion(1).Expand()));

	auto& credentials = root->add(makeGroupBox(Orientation::Vertical, "Credentials"));
	credentials.add(makeLeaf(fakeWidget(boxTag1)));

	auto& options = root->add(makeGroupBox(Orientation::Vertical, "Options"));
	options.add(makeLeaf(fakeWidget(boxTag2)));

	// root
	CHECK(root->kind == NodeKind::Box);
	CHECK(root->orientation == Orientation::Vertical);
	CHECK(!root->isLeaf());
	CHECK_EQ(root->children.size(), (size_t)3);
	CHECK(root->flags.border() == (EdgeInsets { 10, 10, 10, 10 }));
	CHECK(root->widget == nullptr);

	// row
	CHECK(row.kind == NodeKind::Box);
	CHECK(row.orientation == Orientation::Horizontal);
	CHECK_EQ(row.children.size(), (size_t)2);

	// leaves keep their widget back-pointer and flags
	const LayoutNode& label = *row.children[0];
	const LayoutNode& input = *row.children[1];
	CHECK(label.isLeaf());
	CHECK(label.widget == labelWidget);
	CHECK_EQ(label.flags.border().right, 5);
	CHECK(label.flags.crossAlign(NodeKind::Leaf, Orientation::Horizontal) == Align::Center);
	CHECK(input.isLeaf());
	CHECK(input.widget == inputWidget);
	CHECK_EQ(input.flags.proportion(), 1);
	CHECK(input.flags.crossAlign(NodeKind::Leaf, Orientation::Horizontal) == Align::Stretch);

	// group boxes carry their labels
	CHECK(credentials.kind == NodeKind::GroupBox);
	CHECK_EQ(credentials.label, std::string("Credentials"));
	CHECK(options.kind == NodeKind::GroupBox);
	CHECK_EQ(options.label, std::string("Options"));

	// engine outputs start zeroed on every node
	CHECK(root->desired == (Size { 0, 0 }));
	CHECK(root->frame == Rect{});
	CHECK(input.desired == (Size { 0, 0 }));
	CHECK(input.frame == Rect{});
}

TEST(node_tree_tab_panel)
{
	auto tabs = makeTabPanel();
	auto& notes = tabs->add(makeBox(Orientation::Vertical));
	notes.label = "Notes";
	auto& about = tabs->add(makeBox(Orientation::Vertical));
	about.label = "About";

	CHECK(tabs->kind == NodeKind::TabPanel);
	CHECK_EQ(tabs->children.size(), (size_t)2);
	CHECK_EQ(tabs->children[0]->label, std::string("Notes"));
	CHECK_EQ(tabs->children[1]->label, std::string("About"));
}
