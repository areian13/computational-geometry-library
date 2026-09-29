#pragma once

#include <cmath>

#include "../numeric.hpp"

struct Point {
    double x, y;

    Point() : Point(0.0, 0.0) {}
    Point(double x, double y) : x(x), y(y) {}

    Point& operator+=(const Point& p) {
        x += p.x;
        y += p.y;
        return *this;
    }
    Point& operator-=(const Point& p) {
        x -= p.x;
        y -= p.y;
        return *this;
    }

    Point& operator*=(double d) {
        x *= d;
        y *= d;
        return *this;
    }
    Point& operator/=(double d) {
        x /= d;
        y /= d;
        return *this;
    }

    Point operator+() const { return *this; }
    Point operator-() const { return Point(-x, -y); }

    friend Point operator+(Point a, const Point& b) {
        a += b;
        return a;
    }
    friend Point operator-(Point a, const Point& b) {
        a -= b;
        return a;
    }

    friend Point operator*(Point p, double d) {
        p *= d;
        return p;
    }
    friend Point operator*(double d, Point p) {
        p *= d;
        return p;
    }
    friend Point operator/(Point p, double d) {
        p /= d;
        return p;
    }

    double dot(const Point& p) const { return x * p.x + y * p.y; }
    double cross(const Point& p) const { return x * p.y - y * p.x; }
    double norm2() const { return dot(*this); }
    double norm() const { return std::sqrt(norm2()); }

    friend bool operator==(const Point& a, const Point& b) {
        return a.x == b.x && a.y == b.y;
    }
};

inline double orientation(const Point& a, const Point& b, const Point& c) { return (b - a).cross(c - a); }
inline int ccw(const Point& a, const Point& b, const Point& c) { return sign(orientation(a, b, c)); }

inline double dist2(const Point& a, const Point& b) { return (b - a).norm2(); }
inline double dist(const Point& a, const Point& b) { return (b - a).norm(); }