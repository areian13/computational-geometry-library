#pragma once

#include <algorithm>
#include <vector>

#include "../geometry/polygon.hpp"

inline ConvexPolygon pairwise_hull(const ConvexPolygon& a, const ConvexPolygon& b,
                                  bool subtract_second) {
    auto lower = [](const Point& p, const Point& q) {
        return p.y != q.y ? p.y < q.y : p.x < q.x;
    };

    auto normalize = [&](const std::vector<Point>& input, bool negate) {
        std::vector<Point> points;
        points.reserve(input.size());

        for (const Point& p : input) {
            Point q = negate ? -p : p;
            if (points.empty() || !(points.back() == q)) points.push_back(q);
        }

        if (points.size() > 1 && points.front() == points.back()) points.pop_back();
        if (points.size() < 2) return points;

        if (std::all_of(points.begin() + 2, points.end(), [&](const Point& p) {
            return orientation(points[0], points[1], p) == 0;
        })) {
            auto [first, last] = std::minmax_element(points.begin(), points.end(), lower);
            return std::vector<Point>{ *first, *last };
        }

        std::rotate(points.begin(), std::min_element(points.begin(), points.end(), lower), points.end());

        std::vector<Point> boundary;
        boundary.reserve(points.size());

        for (const Point& p : points) {
            while (boundary.size() > 1 && orientation(boundary[boundary.size() - 2], boundary.back(), p) == 0)
                boundary.pop_back();

            boundary.push_back(p);
        }

        while (boundary.size() > 2 && orientation(boundary[boundary.size() - 2], boundary.back(), boundary.front()) == 0)
            boundary.pop_back();

        return boundary;
    };

    auto first = normalize(a.points, false), second = normalize(b.points, subtract_second);
    if (first.empty() || second.empty()) return {};

    std::size_t n = first.size() > 1 ? first.size() : 0;
    std::size_t m = second.size() > 1 ? second.size() : 0;

    ConvexPolygon result;
    if (n == 0 && m == 0) {
        result.points.push_back(first[0] + second[0]);
        return result;
    }

    result.points.reserve(n + m);

    auto half = [](const Point& p) { return p.y > 0 || (p.y == 0 && p.x >= 0) ? 0 : 1; };

    std::size_t i = 0, j = 0;
    while (i < n || j < m) {
        result.points.push_back(first[i % first.size()] + second[j % second.size()]);

        if (i == n) {
            ++j;
            continue;
        }

        if (j == m) {
            ++i;
            continue;
        }

        Point u = first[(i + 1) % first.size()] - first[i];
        Point v = second[(j + 1) % second.size()] - second[j];

        if (half(u) != half(v)) {
            if (half(u) < half(v)) ++i;
            else ++j;
        } else {
            double cross = u.cross(v);

            if (cross >= 0) ++i;
            if (cross <= 0) ++j;
        }
    }

    result.points = normalize(result.points, false);
    return result;
}

inline ConvexPolygon minkowski_sum(const ConvexPolygon& a, const ConvexPolygon& b) {
    return pairwise_hull(a, b, false);
}

inline ConvexPolygon translation_overlap_region(const ConvexPolygon& p, const ConvexPolygon& q) {
    return pairwise_hull(q, p, true);
}
