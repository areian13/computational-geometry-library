#pragma once

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

#include "polygon.hpp"

namespace convex_polygon_intersection_detail {

inline bool same_point(const Point& a, const Point& b) {
    return approx_eq(a.x, b.x) && approx_eq(a.y, b.y);
}

inline double side(const Point& a, const Point& b, const Point& p) {
    const double value = orientation(a, b, p);
    if (!std::isfinite(value))
        throw std::overflow_error("Convex polygon intersection: orientation overflow.");
    return value;
}

inline void append(std::vector<Point>& points, const Point& p) {
    if (points.empty() || !same_point(points.back(), p)) points.push_back(p);
}

inline std::vector<Point> endpoints(const std::vector<Point>& points) {
    if (points.empty()) return {};
    const auto [first, last] = std::minmax_element(points.begin(), points.end(),
        [](const Point& a, const Point& b) {
            return a.x != b.x ? a.x < b.x : a.y < b.y;
        });
    if (same_point(*first, *last)) return {*first};
    return {*first, *last};
}

inline std::vector<Point> canonicalize(const std::vector<Point>& input) {
    std::vector<Point> points;
    for (const Point& p : input) {
        if (!std::isfinite(p.x) || !std::isfinite(p.y))
            throw std::invalid_argument("Convex polygon intersection: coordinates must be finite.");
        append(points, p);
    }
    if (points.size() > 1 && same_point(points.front(), points.back())) points.pop_back();
    if (points.size() <= 2) return endpoints(points);
    const double area = Polygon(points).signed_area();
    if (!std::isfinite(area))
        throw std::overflow_error("Convex polygon intersection: area overflow.");
    if (sign(area) < 0)
        throw std::invalid_argument("Convex polygon intersection requires CCW polygons.");
    std::vector<Point> corners;
    for (std::size_t i = 0; i < points.size(); ++i) {
        const int turn = sign(side(points[(i + points.size() - 1) % points.size()],
                                   points[i], points[(i + 1) % points.size()]));
        if (turn < 0)
            throw std::invalid_argument("Convex polygon intersection requires convex polygons.");
        if (turn > 0) corners.push_back(points[i]);
    }
    return corners.empty() ? endpoints(points) : corners;
}

inline std::vector<Point> clip(std::vector<Point> subject, const std::vector<Point>& boundary) {
    for (std::size_t i = 0; i < boundary.size() && !subject.empty(); ++i) {
        const Point& a = boundary[i];
        const Point& b = boundary[(i + 1) % boundary.size()];
        std::vector<Point> next;
        Point previous = subject.back();
        double previous_side = side(a, b, previous);
        for (const Point& current : subject) {
            const double current_side = side(a, b, current);
            const int ps = sign(previous_side), cs = sign(current_side);
            if ((ps >= 0) != (cs >= 0)) {
                const Point hit = ps == 0 ? previous : cs == 0 ? current
                    : previous + (current - previous) * (previous_side / (previous_side - current_side));
                append(next, hit);
            }
            if (cs >= 0) append(next, current);
            previous = current;
            previous_side = current_side;
        }
        if (next.size() > 1 && same_point(next.front(), next.back())) next.pop_back();
        subject = std::move(next);
    }
    return canonicalize(subject);
}

inline bool contains(const std::vector<Point>& polygon, const Point& p) {
    if (polygon.empty()) return false;
    if (polygon.size() == 1) return same_point(polygon[0], p);
    if (polygon.size() == 2) return on_segment(p, Segment(polygon[0], polygon[1]));
    for (std::size_t i = 0; i < polygon.size(); ++i)
        if (sign(side(polygon[i], polygon[(i + 1) % polygon.size()], p)) < 0) return false;
    return true;
}

}

inline ConvexPolygon intersection(const ConvexPolygon& a, const ConvexPolygon& b) {
    namespace detail = convex_polygon_intersection_detail;
    auto p = detail::canonicalize(a.points);
    auto q = detail::canonicalize(b.points);
    ConvexPolygon result;
    if (p.empty() || q.empty()) return result;
    if (p.size() == 1 || q.size() == 1) {
        const Point point = p.size() == 1 ? p[0] : q[0];
        if (detail::contains(p.size() == 1 ? q : p, point)) result.points = {point};
    } else if (p.size() == 2 && q.size() == 2) {
        const auto hit = intersection(Segment(p[0], p[1]), Segment(q[0], q[1]));
        if (hit.type == IntersectionType::Point) result.points = {hit.point};
        if (hit.type == IntersectionType::Overlap) result.points = detail::endpoints({hit.overlap.a, hit.overlap.b});
    } else if (p.size() == 2 || q.size() == 2) {
        result.points = p.size() == 2 ? detail::clip(p, q) : detail::clip(q, p);
    } else {
        result.points = p.size() >= q.size() ? detail::clip(p, q) : detail::clip(q, p);
    }
    return result;
}

inline double intersection_area(const ConvexPolygon& a, const ConvexPolygon& b) {
    return intersection(a, b).area();
}
