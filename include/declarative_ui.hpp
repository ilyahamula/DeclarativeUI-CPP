#pragma once

#include <wx/wx.h>

#include "widgets.hpp"
#include "sizers.hpp"

static_assert(CreateAndAddable<Button>);
static_assert(CreateAndAddable<TextCtrl>);
static_assert(CreateAndAddable<StaticText>);
static_assert(CreateAndAddable<HSizer<Button, TextCtrl>>);
static_assert(CreateAndAddable<VSizer<StaticText, Button>>);