#pragma once

// Observable<T>: a bound value that counts its own changes, so the retained
// backends poll it by comparing one integer rather than the whole value. Pass
// it wherever a widget binds a T&; change it through edit() or set(). See
// frameworks_core/CoreTypes/Observable.hpp.
#include "frameworks_core/CoreTypes/Observable.hpp"
