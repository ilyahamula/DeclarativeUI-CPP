#pragma once

#include "widgets.hpp"
#include "stacks.hpp"
#include "groupboxes.hpp"
#include "dialog.hpp"
#include "tabpanel.hpp"

static_assert(CreateAndAddable<Button>);
static_assert(CreateAndAddable<TextCtrl>);
static_assert(CreateAndAddable<StaticText>);
static_assert(CreateAndAddable<HStack<Button, TextCtrl>>);
static_assert(CreateAndAddable<VStack<StaticText, Button>>);