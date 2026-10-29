#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>

#include "../algorithm/minkowski_sum.hpp"
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
}

int main() {
    try {
        const auto p = polygon({{0,0},{2,0},{2,2},{0,2}});
        const auto q = polygon({{3,1},{6,1},{6,2},{3,2}});
        const auto d = translation_overlap_region_naive(p, q);
        check(d.size() == 4 && near(d.area(), 15) && d.signed_area() > 0, "Rectangle difference area and orientation.");
        for (Point v : {Point{1,-1},Point{6,-1},Point{6,2},Point{1,2}})
            check(locate(d, v) == PointLocation::ON_VERTEX, "Analytic difference corners.");
        check(locate(d, {3,0}) == PointLocation::INSIDE, "Interior overlap translation.");
        check(locate(d, {0,0}) == PointLocation::OUTSIDE, "Outside overlap translation.");
        const auto contact = intersection(shifted(p, {1,-1}), q);
        check(contact.size() == 1 && near(contact.area(), 0), "Boundary includes vertex contact.");
        check(intersection(shifted(p, {1,0}), q).size() == 2, "Boundary includes shared segment contact.");
        check(minkowski_sum_naive(p, q).size() == 4 && near(minkowski_sum_naive(p, q).area(), 15), "Rectangle Minkowski sum.");
        check(minkowski_sum_naive(p, {}).size() == 0 && translation_overlap_region_naive({}, q).size() == 0, "Empty inputs.");
        const auto point = polygon({{2,3}});
        check(minkowski_sum_naive(point, point).points[0] == Point(4,6), "Point sum.");
        check(translation_overlap_region_naive(point, polygon({{5,7}})).points[0] == Point(3,4), "Point difference sign.");
        const auto horizontal = polygon({{-1,0},{1,0}}), vertical = polygon({{0,-2},{0,2}});
        check(near(minkowski_sum_naive(horizontal, vertical).area(), 8), "Transverse segment sum.");
        const auto line = minkowski_sum_naive(horizontal, horizontal);
        check(line.size() == 2 && near(support(line, {1,0}), 2), "Collinear segment sum.");
        const auto redundant = polygon({{0,2},{2,2},{2,0},{1,0},{0,0},{0,2}});
        check(near(minkowski_sum_naive(redundant, q).area(), 15), "Vertex order and redundant points are immaterial.");
        bool rejected = false;
        try { minkowski_sum_naive(polygon({{std::numeric_limits<double>::quiet_NaN(),0}}), p); }
        catch (const std::invalid_argument&) { rejected = true; }
        check(rejected, "Nonfinite input rejected.");
        rejected = false;
        try { minkowski_sum_naive(polygon({{std::numeric_limits<double>::max(),0}}), polygon({{std::numeric_limits<double>::max(),0}})); }
        catch (const std::overflow_error&) { rejected = true; }
        check(rejected, "Coordinate overflow rejected.");

        std::mt19937 random(20261023);
        std::uniform_real_distribution<double> coordinate(-2.,2.), translation(-5.,5.);
        for (int trial = 0; trial < 100; ++trial) {
            std::vector<Point> vertices_a, vertices_b;
            for (int i = 0; i < 12; ++i) {
                vertices_a.push_back({coordinate(random),coordinate(random)});
                vertices_b.push_back({coordinate(random),coordinate(random)});
            }
            const auto a = convex_hull(vertices_a), b = convex_hull(vertices_b);
            const auto sum = minkowski_sum_naive(a,b), reverse_sum = minkowski_sum_naive(b,a);
            const auto region = translation_overlap_region_naive(a,b), reverse_region = translation_overlap_region_naive(b,a);
            const Point sa{coordinate(random),coordinate(random)}, sb{coordinate(random),coordinate(random)};
            const auto moved = translation_overlap_region_naive(shifted(a,sa), shifted(b,sb));
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
