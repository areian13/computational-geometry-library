#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>

#include "../algorithm/minkowski_sum.hpp"
#include "../algorithm/convex_hull.hpp"
#include "../geometry/convex_polygon_intersection.hpp"

namespace {
int checks = 0;
void check(bool condition, const char* message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}
bool near(double a, double b) { return std::abs(a - b) < 1e-7; }
ConvexPolygon polygon(std::initializer_list<Point> points) {
    ConvexPolygon result; result.points = points; return result;
}
ConvexPolygon shifted(ConvexPolygon p, const Point& t) {
    for (auto& v : p.points) v += t;
    return p;
}
double support(const ConvexPolygon& p, const Point& u) {
    double result = -std::numeric_limits<double>::infinity();
    for (const auto& v : p.points) result = std::max(result, v.dot(u));
    return result;
}
ConvexPolygon brute_sum(const ConvexPolygon& a, const ConvexPolygon& b, bool subtract_second) {
    std::vector<Point> sums;
    for (const Point& p : a.points) for (const Point& q : b.points)
        sums.push_back(subtract_second ? p - q : p + q);
    return convex_hull(sums);
}
void compare_hulls(const ConvexPolygon& actual, const ConvexPolygon& expected) {
    check(actual.points == expected.points, "Linear merge differs from pairwise enumeration and hull.");
}
}

int main() {
    try {
        const auto p = polygon({{0,0},{2,0},{2,2},{0,2}});
        const auto q = polygon({{3,1},{6,1},{6,2},{3,2}});
        const auto d = translation_overlap_region(p, q);
        check(d.size() == 4 && near(d.area(), 15) && d.signed_area() > 0, "Rectangle difference area and orientation.");
        for (Point v : {Point{1,-1},Point{6,-1},Point{6,2},Point{1,2}})
            check(locate(d, v) == PointLocation::ON_VERTEX, "Analytic difference corners.");
        check(locate(d, {3,0}) == PointLocation::INSIDE, "Interior overlap translation.");
        check(locate(d, {0,0}) == PointLocation::OUTSIDE, "Outside overlap translation.");
        const auto contact = intersection(shifted(p, {1,-1}), q);
        check(contact.size() == 1 && near(contact.area(), 0), "Boundary includes vertex contact.");
        check(intersection(shifted(p, {1,0}), q).size() == 2, "Boundary includes shared segment contact.");
        check(minkowski_sum(p, q).size() == 4 && near(minkowski_sum(p, q).area(), 15), "Rectangle Minkowski sum.");
        check(minkowski_sum(p, {}).size() == 0 && translation_overlap_region({}, q).size() == 0, "Empty inputs.");
        const auto point = polygon({{2,3}});
        check(minkowski_sum(point, point).points[0] == Point(4,6), "Point sum.");
        check(translation_overlap_region(point, polygon({{5,7}})).points[0] == Point(3,4), "Point difference sign.");
        const auto horizontal = polygon({{-1,0},{1,0}}), vertical = polygon({{0,-2},{0,2}});
        check(near(minkowski_sum(horizontal, vertical).area(), 8), "Transverse segment sum.");
        const auto line = minkowski_sum(horizontal, horizontal);
        check(line.size() == 2 && near(support(line, {1,0}), 2), "Collinear segment sum.");
        const auto redundant = polygon({{0,2},{0,0},{0,0},{1,0},{2,0},{2,2},{0,2}});
        check(near(minkowski_sum(redundant, q).area(), 15), "CCW boundary tolerates cyclic shifts and redundant points.");
        const auto collinear = polygon({{2,0},{-1,0},{0,0},{2,0},{2,0}});
        compare_hulls(minkowski_sum(collinear, vertical), brute_sum(collinear, vertical, false));
        compare_hulls(pairwise_hull(collinear, vertical, true), brute_sum(collinear, vertical, true));
        compare_hulls(minkowski_sum(point, redundant), brute_sum(point, redundant, false));
        compare_hulls(minkowski_sum(redundant, point), brute_sum(redundant, point, false));
        compare_hulls(pairwise_hull(point, redundant, true), brute_sum(point, redundant, true));
        const auto copies = polygon({{2,3},{2,3},{2,3}});
        compare_hulls(minkowski_sum(copies, copies), polygon({{4,6}}));
        compare_hulls(pairwise_hull(copies, copies, true), polygon({{0,0}}));

        std::mt19937 integers(20261029);
        std::uniform_int_distribution<int> integer(-30,30);
        for (int trial = 0; trial < 500; ++trial) {
            std::vector<Point> input_a, input_b;
            for (int i = 0; i < trial % 25; ++i) input_a.emplace_back(integer(integers), integer(integers));
            for (int i = 0; i < (trial * 7) % 25; ++i) input_b.emplace_back(integer(integers), integer(integers));
            auto a = convex_hull(input_a), b = convex_hull(input_b);
            if (!a.points.empty()) std::rotate(a.points.begin(), a.points.begin() + integers() % a.points.size(), a.points.end());
            if (!b.points.empty()) std::rotate(b.points.begin(), b.points.begin() + integers() % b.points.size(), b.points.end());
            compare_hulls(minkowski_sum(a,b), brute_sum(a,b,false));
            compare_hulls(pairwise_hull(a,b,true), brute_sum(a,b,true));
            compare_hulls(translation_overlap_region(a,b), brute_sum(b,a,true));
            if (a.points.size() > 1) a.points.insert(a.points.begin() + 1, a.points[0]);
            if (!b.points.empty()) b.points.push_back(b.points.front());
            compare_hulls(minkowski_sum(a,b), brute_sum(a,b,false));
            compare_hulls(pairwise_hull(a,b,true), brute_sum(a,b,true));
        }

        ConvexPolygon large;
        for (int i = 0; i < 20000; ++i) {
            double angle = 2 * std::acos(-1.) * i / 20000;
            large.points.emplace_back(100 * std::cos(angle), 100 * std::sin(angle));
        }
        auto large_sum = minkowski_sum(large, large);
        check(large_sum.size() == large.size(), "Parallel edge pairs advance together in a large input.");
        check(std::abs(large_sum.area() - 4 * large.area()) < 1e-5, "Large-input area.");
        for (Point u : {Point{1,0}, Point{0,1}, Point{-1,0}, Point{0,-1}, Point{1,2}})
            check(near(support(large_sum,u), 2 * support(large,u)), "Large-input support identity.");

        std::mt19937 random(20261023);
        std::uniform_real_distribution<double> coordinate(-2.,2.), translation(-5.,5.);
        for (int trial = 0; trial < 100; ++trial) {
            std::vector<Point> vertices_a, vertices_b;
            for (int i = 0; i < 12; ++i) {
                vertices_a.push_back({coordinate(random),coordinate(random)});
                vertices_b.push_back({coordinate(random),coordinate(random)});
            }
            const auto a = convex_hull(vertices_a), b = convex_hull(vertices_b);
            const auto sum = minkowski_sum(a,b), reverse_sum = minkowski_sum(b,a);
            const auto region = translation_overlap_region(a,b), reverse_region = translation_overlap_region(b,a);
            const Point sa{coordinate(random),coordinate(random)}, sb{coordinate(random),coordinate(random)};
            const auto moved = translation_overlap_region(shifted(a,sa), shifted(b,sb));
            check(region.is_convex() && region.signed_area() > 0, "Convex CCW region.");
            for (int k = 0; k < 16; ++k) {
                const Point u{coordinate(random),coordinate(random)};
                check(near(support(sum,u), support(a,u)+support(b,u)), "Minkowski support-function identity.");
                check(near(support(sum,u), support(reverse_sum,u)), "Sum commutativity.");
                check(near(support(region,u), support(b,u)+support(a,-u)), "Overlap-region support-function identity.");
                check(near(support(region,u), support(reverse_region,-u)), "Swapping polygons reflects the region.");
                check(near(support(moved,u), support(region,u)+(sb-sa).dot(u)), "Translation covariance.");
            }
            for (int k = 0; k < 30; ++k) {
                const Point t{translation(random),translation(random)};
                const bool belongs = locate(region,t) != PointLocation::OUTSIDE;
                check(belongs == !intersection(shifted(a,t),b).points.empty(), "Region membership matches independent intersection.");
            }
        }
        std::cout << "Minkowski sum: " << checks << " checks passed.\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n'; return 1;
    }
}
