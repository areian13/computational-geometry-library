#pragma once

#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

#include "convex_hull.hpp"

namespace minkowski_sum_detail {

inline void check_finite(const ConvexPolygon& polygon) {
    for (const Point& p : polygon.points)
        if (!std::isfinite(p.x) || !std::isfinite(p.y))
            throw std::invalid_argument("Minkowski sum: coordinates must be finite.");
}

inline ConvexPolygon pairwise_hull(const ConvexPolygon& a, const ConvexPolygon& b,
                                  bool subtract_second) {
    check_finite(a);
    check_finite(b);
    if (a.points.empty() || b.points.empty()) return {};
    std::vector<Point> candidates;
    const std::size_t limit = std::min(candidates.max_size(),
        static_cast<std::size_t>(std::numeric_limits<int>::max()));
    if (a.points.size() > limit / b.points.size())
        throw std::length_error("Minkowski sum: too many vertex pairs.");
    candidates.reserve(a.points.size() * b.points.size());
    for (const Point& p : a.points) for (const Point& q : b.points) {
        const Point v = subtract_second ? p - q : p + q;
        if (!std::isfinite(v.x) || !std::isfinite(v.y))
            throw std::overflow_error("Minkowski sum: vertex coordinate overflow.");
        candidates.push_back(v);
    }
    return convex_hull(candidates);
}

}

inline ConvexPolygon minkowski_sum_naive(const ConvexPolygon& a, const ConvexPolygon& b) {
    return minkowski_sum_detail::pairwise_hull(a, b, false);
}

inline ConvexPolygon translation_overlap_region_naive(const ConvexPolygon& p,
                                                      const ConvexPolygon& q) {
    return minkowski_sum_detail::pairwise_hull(q, p, true);
}
