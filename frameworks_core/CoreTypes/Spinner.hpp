#pragma once

#include <cmath>
#include <cstdint>

// The busy indicator's shared numbers (Spinner, SpinnerWrapper), so the three
// backends turn at one speed and default to one size.

// Default edge of the square a spinner measures: native activity indicators
// range from 16 to 32 px, so the engine asks for one size everywhere and
// withSize() changes it -- the same reason Separator measures 1 px itself.
inline constexpr int kDefaultSpinnerSize = 24;

// One revolution per second, and the redraw interval of the retained
// backends' clocks (~30 fps, smooth enough for a rotation and cheap).
inline constexpr int kSpinnerPeriodMs = 1000;
inline constexpr int kSpinnerFrameMs = 33;

// The rotation, in radians, `elapsedMs` into the animation.
inline double spinnerAngle(std::int64_t elapsedMs)
{
	constexpr double kTwoPi = 6.283185307179586;
	return static_cast<double>(elapsedMs % kSpinnerPeriodMs) / kSpinnerPeriodMs * kTwoPi;
}
