#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>

#include "../geometry/convex_polygon_intersection.hpp"
#include "../algorithm/convex_hull.hpp"

namespace {
int checks = 0;
void check(bool condition, const char* message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}
bool near(double a, double b) { return std::abs(a - b) < 1e-7; }
ConvexPolygon polygon(std::initializer_list<Point> points) {
    ConvexPolygon p; p.points = points; return p;
}
ConvexPolygon shifted(ConvexPolygon p, Point t) {
    for (auto& v : p.points) v += t;
    return p;
}
}

int main() {
    try {
        const auto square = polygon({{0,0},{2,0},{2,2},{0,2}});
        check(near(intersection_area(square, square), 4), "Identical polygons.");
        const auto overlap = intersection(square, shifted(square, {1,1}));
        check(near(overlap.area(), 1) && overlap.size() == 4 && overlap.signed_area() > 0, "Area and CCW output.");
        check(intersection(square, shifted(square, {3,0})).size() == 0, "Disjoint polygons.");
        check(intersection(square, shifted(square, {2,0})).size() == 2, "Shared edge preserved as a segment.");
        check(intersection(square, shifted(square, {2,2})).size() == 1, "Shared vertex preserved as a point.");
        const auto inner = polygon({{.5,.5},{1.5,.5},{1.5,1.5},{.5,1.5}});
        check(near(intersection_area(square, inner), 1), "Containment.");
        check(intersection(square, ConvexPolygon()).size() == 0, "Empty input.");
        check(intersection(square, polygon({{1,1}})).size() == 1, "Point inside.");
        check(intersection(square, polygon({{3,3}})).size() == 0, "Point outside.");
        check(intersection(square, polygon({{-1,1},{3,1}})).size() == 2, "Clipped segment.");
        check(intersection(polygon({{0,0},{2,0}}), polygon({{1,0},{3,0}})).size() == 2, "Collinear segment overlap.");
        check(intersection(polygon({{0,0},{2,2}}), polygon({{0,2},{2,0}})).size() == 1, "Crossing segments.");
        const auto redundant = polygon({{0,0},{1,0},{2,0},{2,2},{0,2},{0,0}});
        check(intersection(redundant, square).size() == 4, "Redundant vertices and closing duplicate.");
        check(intersection(polygon({{0,0},{1,0},{2,0}}), square).size() == 2, "Collinear polygon input.");
        bool rejected = false;
        try { intersection(square, polygon({{0,0},{0,2},{2,2},{2,0}})); }
        catch (const std::invalid_argument&) { rejected = true; }
        check(rejected, "Clockwise input rejected.");
        rejected = false;
        try { intersection(square, polygon({{std::numeric_limits<double>::infinity(),0}})); }
        catch (const std::invalid_argument&) { rejected = true; }
        check(rejected, "Nonfinite input rejected.");
        for (int ix = -25; ix <= 25; ++ix) for (int iy = -25; iy <= 25; ++iy) {
            const double x = ix / 10., y = iy / 10.;
            const double expected = std::max(0., 2-std::abs(x)) * std::max(0., 2-std::abs(y));
            check(near(intersection_area(square, shifted(square, {x,y})), expected), "Analytic rectangle translation formula.");
        }
        const auto triangle = polygon({{0,0},{2,0},{0,2}});
        for (int ix = 0; ix <= 20; ++ix) for (int iy = 0; iy <= 20; ++iy) {
            const double x = ix / 10., y = iy / 10., side = std::max(0., 2-x-y);
            check(near(intersection_area(triangle, shifted(triangle, {x,y})), side*side/2), "Analytic triangle translation formula.");
        }
        std::mt19937 random(20261023);
        std::uniform_real_distribution<double> coordinate(-2., 2.);
        for (int trial = 0; trial < 200; ++trial) {
            std::vector<Point> points_a, points_b;
            for (int i = 0; i < 14; ++i) { points_a.push_back({coordinate(random),coordinate(random)}); points_b.push_back({coordinate(random),coordinate(random)}); }
            const auto a = convex_hull(points_a), b = convex_hull(points_b);
            const auto hit = intersection(a, b);
            check(near(hit.area(), intersection_area(b, a)), "Intersection symmetry.");
            check(hit.area() <= std::min(a.area(), b.area()) + 1e-7, "Intersection bounded by both areas.");
            for (const auto& p : hit.points)
                check(convex_polygon_intersection_detail::contains(a.points, p) &&
                      convex_polygon_intersection_detail::contains(b.points, p), "Result vertices belong to both polygons.");
            const Point t{coordinate(random),coordinate(random)};
            check(near(hit.area(), intersection_area(shifted(a,t),shifted(b,t))), "Common translation invariance.");
        }
        std::cout << "Polygon intersection: " << checks << " checks passed.\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n'; return 1;
    }
}
