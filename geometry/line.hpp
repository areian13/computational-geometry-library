#pragma once

#include "point.hpp"
#include "segment.hpp"

struct Line {
    Point p;
    Point dir;

    Line(const Point& p, const Point& dir) : p(p), dir(dir) {}
    Line(const Segment& s) : p(s.a), dir(s.direction()) {}
};