#pragma once

#include <cmath>
#include <limits>

inline constexpr double EPS = 1e-9;
inline constexpr double INF = std::numeric_limits<double>::infinity();

inline bool is_zero(double x) { return std::abs(x) < EPS; }
inline bool approx_eq(double a, double b) { return is_zero(a - b); }
inline int sign(double x) { return is_zero(x) ? 0 : (x > 0 ? +1 : -1); }