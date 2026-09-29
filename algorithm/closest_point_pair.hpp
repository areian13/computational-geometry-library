#pragma once

#include <vector>
#include <algorithm>
#include <cmath>
#include <tuple>
#include <set>
#include <utility>

#include "../numeric.hpp"
#include "../geometry/point.hpp"

inline std::pair<int, int> closest_point_pair(const std::vector<Point>& points) {
    int n = points.size();
    if (n < 2) return { -1,-1 };

    std::vector<int> order(n);
    for (int i = 0; i < n; i++) order[i] = i;

    std::sort(order.begin(), order.end(),
        [&](int i, int j) {
        if (points[i].x != points[j].x) return points[i].x < points[j].x;
        if (points[i].y != points[j].y) return points[i].y < points[j].y;
        return i < j;
    });

    std::pair<int, int> result = { order[0], order[1] };
    double best = dist2(points[result.first], points[result.second]);

    if (best == 0.0) return result;

    std::set<std::tuple<double, double, int>> active;
    int l = 0;
    for (int r = 0; r < n; r++) {
        int i = order[r];
        const Point& p = points[i];

        while (l < r) {
            int j = order[l];
            double dx = p.x - points[j].x;

            if (dx * dx <= best) break;

            active.erase({ points[j].y, points[j].x, j });
            l++;
        }

        double d = std::sqrt(best);

        auto it = active.lower_bound({ p.y - d,-INF,-1 });
        while (it != active.end() && std::get<0>(*it) <= p.y + d) {
            int j = std::get<2>(*it);
            double cur = dist2(p, points[j]);

            if (cur < best) {
                best = cur;
                result = { i,j };

                if (best == 0.0) goto BEST;
            }
            it++;
        }

        active.insert({ p.y,p.x,i });
    }

BEST:
    if (result.first > result.second)
        std::swap(result.first, result.second);
    return result;
}