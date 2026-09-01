#pragma once

#include "widgets.hpp"
#include "stacks.hpp"
#include "groupboxes.hpp"
#include "dialog.hpp"
#include "tabpanel.hpp"
#include "messagebox.hpp"

static_assert(NodeBuildable<Button>);
static_assert(NodeBuildable<TextCtrl>);
static_assert(NodeBuildable<StaticText>);
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
static_assert(NodeBuildable<HStack<Button, TextCtrl>>);
static_assert(NodeBuildable<VStack<StaticText, Button>>);

static_assert(TabContent<VStack<Button>>);
static_assert(IsTab<Tab<VStack<Button>>>);
static_assert(NodeBuildable<TabPanel<Tab<VStack<Button>>>>);