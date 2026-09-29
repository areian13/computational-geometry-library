#pragma once

#include <algorithm>
#include <utility>
#include <vector>

#include "../geometry/polygon.hpp"

inline std::pair<ConvexPolygon, std::vector<int>>
indexed_convex_hull(const std::vector<Point>& points) {
    int n = points.size();
    if (n == 0) return { ConvexPolygon(), {} };

    int p0 = std::min_element(points.begin(), points.end(),
        [](const Point& a, const Point& b) {
        if (a.y != b.y) return a.y < b.y;
        return a.x < b.x;
    }) - points.begin();

    std::vector<int> order(n);
    for (int i = 0; i < n; i++) order[i] = i;

    std::sort(order.begin(), order.end(),
        [&](int i, int j) {
        double turn = orientation(points[p0], points[i], points[j]);
        if (turn != 0.0) return turn > 0.0;

        double di = dist2(points[p0], points[i]);
        double dj = dist2(points[p0], points[j]);

        if (di != dj) return di < dj;
        return i < j;
    });

    order.erase(std::unique(order.begin(), order.end(),
        [&](int i, int j) {
        return points[i] == points[j];
    }), order.end());

    std::vector<int> indices;

    for (int c : order) {
        while (indices.size() >= 2) {
            int b = indices[indices.size() - 1];
            int a = indices[indices.size() - 2];

            if (ccw(points[a], points[b], points[c]) > 0) break;
            indices.pop_back();
        }

        indices.push_back(c);
    }

    ConvexPolygon hull;
    hull.points.reserve(indices.size());

    for (int i : indices)
        hull.points.push_back(points[i]);

    return { std::move(hull), std::move(indices) };
}

inline ConvexPolygon convex_hull(const std::vector<Point>& points) {
    return indexed_convex_hull(points).first;
}