#pragma once

#include "point.hpp"

struct Segment {
    Point a, b;

    Segment() : Segment(Point(), Point()) {}
    Segment(const Point& a, const Point& b) : a(a), b(b) {}

    Point direction() const { return b - a; }
    Point midpoint() const { return (a + b) / 2.0; }

    double length2() const { return dist2(a, b); }
    double length() const { return dist(a, b); }
};

enum class IntersectionType { None, Point, Overlap };

struct SegmentIntersection {
    IntersectionType type = IntersectionType::None;
    Point point;
    Segment overlap;
};

inline bool on_segment(const Point& p, const Segment& s) {
    return ccw(s.a, s.b, p) == 0 && sign((p - s.a).dot(p - s.b)) <= 0;
}
inline bool intersects(const Segment& s1, const Segment& s2) {
    const auto& [a, b] = s1;
    const auto& [c, d] = s2;

    if (ccw(a, b, c) * ccw(a, b, d) < 0 &&
        ccw(c, d, a) * ccw(c, d, b) < 0) return true;

    return on_segment(a, s2) || on_segment(b, s2)
        || on_segment(c, s1) || on_segment(d, s1);
}
inline SegmentIntersection intersection(const Segment& s1, const Segment& s2) {
    if (!intersects(s1, s2)) return {};

    const auto& [a, b] = s1;
    const auto& [c, d] = s2;

    Point r = b - a;
    Point s = d - c;

    double det = r.cross(s);

    if (!is_zero(det)) {
        double t = (c - a).cross(s) / det;
        return { IntersectionType::Point, a + r * t, {} };
    }

    Point points[4];
    int n = 0;

    auto add_point = [&](const Point& p) {
        if (!on_segment(p, s1) || !on_segment(p, s2))
            return;

        for (int i = 0; i < n; ++i) {
            if (points[i] == p)
                return;
        }
        points[n++] = p;
    };

    add_point(a);
    add_point(b);
    add_point(c);
    add_point(d);

    if (n == 0) return {};
    if (n == 1) return { IntersectionType::Point, points[0], {} };

    int u = 0, v = 1;
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (dist2(points[i], points[j]) > dist2(points[u], points[v]))
                u = i, v = j;
        }
    }

    return { IntersectionType::Overlap, {}, Segment(points[u], points[v]) };
}