#pragma once

#include "widgets.hpp"
#include "stacks.hpp"
#include "groupboxes.hpp"
#include "dialog.hpp"
#include "tabpanel.hpp"
#include "messagebox.hpp"

static_assert(CreateAndAddable<Button>);
static_assert(CreateAndAddable<TextCtrl>);
static_assert(CreateAndAddable<StaticText>);
static_assert(CreateAndAddable<HStack<Button, TextCtrl>>);
static_assert(CreateAndAddable<VStack<StaticText, Button>>);

static_assert(TabContent<VStack<Button>>);
static_assert(IsTab<Tab<VStack<Button>>>);
static_assert(CreateAndAddable<TabPanel<Tab<VStack<Button>>>>);
static_assert(FittableLayout<TabPanel<Tab<VStack<Button>>>>);