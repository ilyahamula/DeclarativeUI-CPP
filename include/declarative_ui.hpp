#pragma once

#include "widgets.hpp"
#include "stacks.hpp"
#include "grid.hpp"
#include "scrollpanel.hpp"
#include "splitter.hpp"
#include "expander.hpp"
#include "groupboxes.hpp"
#include "dialog.hpp"
#include "window.hpp"
#include "tabpanel.hpp"
#include "messagebox.hpp"

static_assert(NodeBuildable<Button>);
static_assert(NodeBuildable<TextCtrl>);
static_assert(NodeBuildable<StaticText>);
static_assert(NodeBuildable<Spacer>);
static_assert(NodeBuildable<Separator>);
static_assert(NodeBuildable<ReadonlyTextCtrl>);
static_assert(NodeBuildable<ListBox<int>>);
static_assert(NodeBuildable<ListBox<std::string>>);
static_assert(NodeBuildable<ListBox<std::vector<int>>>);
static_assert(NodeBuildable<ListBox<std::vector<std::string>>>);
static_assert(NodeBuildable<TreeView<std::string>>);
static_assert(NodeBuildable<TreeView<std::vector<std::string>>>);
static_assert(NodeBuildable<Table<int>>);
static_assert(NodeBuildable<Table<std::string>>);
static_assert(NodeBuildable<Table<std::vector<int>>>);
static_assert(NodeBuildable<Table<std::vector<std::string>>>);
static_assert(NodeBuildable<Grid<StaticText, TextCtrl>>);
static_assert(NodeBuildable<ScrollPanel<VStack<Button>>>);
static_assert(NodeBuildable<Splitter<VStack<Button>, VStack<Button>>>);
static_assert(NodeBuildable<HSplitter<VStack<Button>, VStack<TextCtrl>>>);
static_assert(NodeBuildable<VSplitter<VStack<Button>, VStack<TextCtrl>>>);
static_assert(NodeBuildable<Expander<VStack<Button>>>);
static_assert(NodeBuildable<Expander<CheckBox>>);
static_assert(NodeBuildable<HStack<Button, TextCtrl>>);
static_assert(NodeBuildable<VStack<StaticText, Button>>);

static_assert(TopLevel<Dialog<VStack<Button>>>);
static_assert(TopLevel<Window<VStack<Button>>>);
static_assert(FlagShowable<Window<VStack<Button>>>);

static_assert(TabContent<VStack<Button>>);
static_assert(IsTab<Tab<VStack<Button>>>);
static_assert(NodeBuildable<TabPanel<Tab<VStack<Button>>>>);