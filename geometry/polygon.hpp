#pragma once

#include <cassert>
#include <vector>

#include "../numeric.hpp"

#include "point.hpp"
#include "segment.hpp"
#include "bitangent.hpp"
#include "point_location.hpp"

struct Polygon {
    std::vector<Point> points;

    Polygon() = default;
    Polygon(const std::vector<Point>& points) : points(points) {}

    bool is_simple(bool allow_lower_dim = false) const {
        return true; // ±¸Çö ¾ÈµÊ
    }
    bool is_convex(bool allow_lower_dim = false) const {
        int n = points.size();
        if (n < 3) return allow_lower_dim;
        if (!is_simple()) return false;

        int dir = 0;

        for (int i = 0; i < n; ++i) {
            int turn = ccw(
                points[i],
                points[(i + 1) % n],
                points[(i + 2) % n]
            );

            if (turn == 0) continue;

            if (dir == 0) dir = turn;
            else if (dir != turn) return false;
        }

        return dir != 0;
    }

    double signed_area() const {
        int n = points.size();
        double ret = 0.0;
        for (int i = 0; i < n; i++)
            ret += points[i].cross(points[(i + 1) % n]);
        return ret / 2.0;
    }

    Point& operator[](int i) { return points[i]; }
    const Point& operator[](int i) const { return points[i]; }
    int size() const { return points.size(); }
};

struct SimplePolygon : Polygon {
    SimplePolygon() = default;
    SimplePolygon(const std::vector<Point>& points) : Polygon(points) {
        assert(is_simple());
    }

    double area() const { return std::abs(signed_area()); }
};

struct ConvexPolygon : SimplePolygon {
    ConvexPolygon() = default;
    ConvexPolygon(const std::vector<Point>& points) : SimplePolygon(points) {
        assert(this->is_convex());
    }
};

inline PointLocation locate(const SimplePolygon& poly, const Point& p) {
    int n = poly.size();

    int cnt = 0;
    Segment l1 = { p,{ p.x + 1. / EPS,p.y + EPS } };
    for (int i = 0; i < n; i++) {
        if (p == poly[i]) return PointLocation::ON_VERTEX;
        Segment l2 = { poly[i],poly[(i + 1) % n] };
        if (on_segment(p, l2)) return PointLocation::ON_BOUNDARY;
        cnt += intersects(l1, l2);
    }
    return cnt % 2 == 1 ? PointLocation::INSIDE : PointLocation::OUTSIDE;
}
inline PointLocation locate(const ConvexPolygon& poly, const Point& p) {
    int n = poly.size();
    if (n == 0) return PointLocation::OUTSIDE;
    if (n == 1) return p == poly[0]
        ? PointLocation::ON_VERTEX
        : PointLocation::OUTSIDE;
    if (n == 2) {
        if (p == poly[0] || p == poly[1])
            return PointLocation::ON_VERTEX;
        if (on_segment(p, Segment(poly[0], poly[1])))
            return PointLocation::ON_BOUNDARY;
        return PointLocation::OUTSIDE;
    }

    if (p == poly[0])
        return PointLocation::ON_VERTEX;

    int left = ccw(poly[0], poly[1], p);
    int right = ccw(poly[0], poly[n - 1], p);
    if (left < 0 || right > 0) return PointLocation::OUTSIDE;

    if (left == 0) {
        if (p == poly[1])
            return PointLocation::ON_VERTEX;
        return on_segment(p, Segment(poly[0], poly[1]))
            ? PointLocation::ON_BOUNDARY
            : PointLocation::OUTSIDE;
    }

    if (right == 0) {
        if (p == poly[n - 1])
            return PointLocation::ON_VERTEX;
        return on_segment(p, Segment(poly[0], poly[n - 1]))
            ? PointLocation::ON_BOUNDARY
            : PointLocation::OUTSIDE;
    }

    int start = 1, end = n - 1;
    while (start + 1 < end) {
        int mid = (start + end) / 2;

        if (ccw(poly[0], poly[mid], p) > 0)
            start = mid;
        else
            end = mid;
    }

    if (p == poly[start] || p == poly[end])
        return PointLocation::ON_VERTEX;

    int side = ccw(poly[start], poly[end], p);
    if (side > 0)
        return PointLocation::INSIDE;
    if (side == 0)
        return PointLocation::ON_BOUNDARY;
    return PointLocation::OUTSIDE;
}