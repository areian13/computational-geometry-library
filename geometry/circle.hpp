#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

#include "point.hpp"
#include "segment.hpp"
#include "bitangent.hpp"
#include "point_location.hpp"

struct Circle {
    Point center;
    double radius;

    Circle() : Circle(Point(), 0.0) {}
    Circle(const Point& center, double radius) : center(center), radius(radius) {}
};

inline BitangentType bitangent_type(const Segment& s, const Circle& a, const Circle& b) {
    bool left_a = ccw(s.a, s.b, a.center) > 0;
    bool left_b = ccw(s.a, s.b, b.center) > 0;

    if (left_a) return left_b ? BitangentType::LL : BitangentType::LR;
    return left_b ? BitangentType::RL : BitangentType::RR;
}
inline std::vector<Bitangent> bitangents(const Circle& a, const Circle& b) {
    std::vector<Bitangent> result;
    result.reserve(4);

    Point d = b.center - a.center;
    Point perp(-d.y, d.x);
    double d2 = d.norm2();

    if (is_zero(d2)) return result;

    for (int side : {1, -1}) {
        double dr = a.radius - side * b.radius;
        double h2 = d2 - dr * dr;

        if (sign(h2) < 0) continue;

        double h = std::sqrt(std::max(0.0, h2));

        for (int turn : {-1, 1}) {
            if (turn == 1 && is_zero(h)) continue;

            Point n = (d * dr + perp * (turn * h)) / d2;

            Point p = a.center + n * a.radius;
            Point q = b.center + n * (side * b.radius);

            Segment s(p, q);
            result.emplace_back(s, bitangent_type(s, a, b));
        }
    }

    return result;
}

inline PointLocation locate(const Circle& cir, const Point& p) {
    int s = sign(dist(cir.center, p) - cir.radius);
    if (s == 0) return PointLocation::ON_BOUNDARY;
    return s < 0 ? PointLocation::INSIDE : PointLocation::OUTSIDE;
}