#pragma once

#include <vector>
#include <algorithm>

#include "convex_hull.hpp"

inline std::pair<int, int> farthest_point_pair(const std::vector<Point>& points) {
    int n = points.size();
    if (n < 2) return { -1,-1 };

    auto [hull, indices] = indexed_convex_hull(points);
    int h = hull.size();
    if (h == 1) return { 0,1 };

    auto comp = [&](const std::pair<int, int>& a, const std::pair<int, int>& b) {
        double da = dist2(hull[a.first], hull[a.second]);
        double db = dist2(hull[b.first], hull[b.second]);
        return da < db;
    };

    std::pair<int, int> result = { 0,1 };
    for (int i = 0, j = 0; i < h; i++) {
        while (j + 1 < h &&
            ccw(hull[i + 1] - hull[i], hull[j + 1] - hull[j], { 0,0 }) >= 0)
            result = std::max(result, { i,j++ }, comp);
        result = std::max(result, { i,j }, comp);
    }

    result = { indices[result.first], indices[result.second] };
    if (result.first > result.second)
        std::swap(result.first, result.second);
    return result;
}