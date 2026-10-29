#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>

#include "../structure/triangulation.hpp"
#include "../algorithm/convex_hull.hpp"

namespace {
int checks = 0, flips = 0;
using Type = Triangulation::LocationType;

void check(bool condition, const char* message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}
void verify(const Triangulation& t) {
    check(t.is_valid(), "Invalid triangulation structure.");
    check(t.is_valid_geometry(), "Invalid triangulation geometry.");
}
template <class Operation>
void rejected(Operation operation) {
    bool failed = false;
    try { operation(); }
    catch (const std::invalid_argument&) { failed = true; }
    catch (const std::out_of_range&) { failed = true; }
    check(failed, "Invalid input was accepted.");
}
double area(const Triangulation& t) {
    double sum = 0;
    for (int f : t.triangle_faces()) sum += t.triangle_area(f);
    return sum;
}
std::string snapshot(const Triangulation& t) {
    std::ostringstream out;
    const auto& d = t.dcel();
    out.precision(17);
    for (const auto& v : d.vertices) out << v.point.x << ',' << v.point.y << ',' << v.halfedge << ',' << v.face << ';';
    for (int e = 0; e < static_cast<int>(d.halfedges.size()); ++e) {
        const auto& h = d.halfedges[e];
        out << h.origin << ',' << h.twin << ',' << h.next << ',' << h.prev << ',' << h.face << ',' << t.is_constrained(e) << ';';
    }
    for (int f = 0; f < static_cast<int>(d.faces.size()); ++f) {
        out << d.faces[f].halfedge << ',' << t.is_triangle(f) << ':';
        for (int e : d.faces[f].inner_halfedges) out << e << ',';
        for (int v : d.faces[f].isolated_vertices) out << v << ',';
    }
    for (int f : t.triangle_faces()) out << f << ',';
    return out.str();
}
Triangulation square() {
    return Triangulation::from_triangles({{0,0},{2,0},{2,2},{0,2}}, {{0,1,2},{0,2,3}});
}

void basic_queries_and_import() {
    Triangulation empty;
    verify(empty);
    check(empty.triangle_faces().empty() && empty.locate({0,0}).type == Type::OUTSIDE, "Empty domain.");
    auto t = square();
    verify(t);
    check(t.triangle_faces().size() == 2 && area(t) == 4, "Square triangulation.");
    check(!t.is_triangle(t.dcel().outer_face), "Unbounded face cannot be a triangle.");
    int diagonal = t.dcel().find_halfedge(0, 2);
    int boundary = t.dcel().find_halfedge(0, 1);
    check(t.is_interior_edge(diagonal) && !t.is_boundary_edge(diagonal), "Internal edge classification.");
    check(t.is_boundary_edge(boundary) && !t.is_interior_edge(boundary), "Boundary edge classification.");
    check(t.opposite_vertex(diagonal) == 3, "Opposite vertex on left side.");
    check(t.opposite_vertex(t.dcel().halfedges[diagonal].twin) == 1, "Opposite vertex on right side.");
    check(t.incident_triangles(0).size() == 2 && t.incident_triangles(1).size() == 1, "Vertex incidence.");
    for (int f : t.triangle_faces()) {
        auto edges = t.triangle_halfedges(f);
        auto vertices = t.triangle_vertices(f);
        auto neighbors = t.triangle_neighbors(f);
        int count = 0;
        for (int i = 0; i < 3; ++i) {
            check(t.dcel().halfedges[edges[i]].origin == vertices[i], "Aligned triangle query order.");
            if (neighbors[i] != -1) {
                ++count;
                check(neighbors[i] == t.dcel().right_face(edges[i]) && neighbors[i] != f, "Neighbor across edge.");
            }
        }
        check(count == 1 && t.triangle_area(f) == 2, "Triangle area and neighbor count.");
    }
    check(locate(t, {1.5,0.5}).type == Type::FACE, "Triangle interior location.");
    auto internal = t.locate({1,1});
    check(internal.type == Type::EDGE && !t.is_boundary_edge(internal.halfedge), "Internal edge location.");
    auto border = t.locate({1,0});
    check(border.type == Type::EDGE && t.is_boundary_edge(border.halfedge), "Domain boundary location.");
    auto vertex = t.locate({0,0});
    check(vertex.type == Type::VERTEX && vertex.vertex == 0 && vertex.halfedge == -1, "Vertex location.");
    check(t.locate({-0.1,0}).type == Type::OUTSIDE && t.locate({3,3}).face == -1, "Outside location.");

    auto reversed = Triangulation::from_triangles({{0,0},{2,0},{2,2},{0,2}}, {{2,1,0},{3,2,0}});
    verify(reversed);
    check(area(reversed) == 4, "CW input triangles.");
    DCEL d({{0,0},{2,0},{2,2},{0,2}}, {{0,1},{1,2},{2,3},{3,0}});
    d.split_face(d.left_face(d.find_halfedge(0,1)), 0, 2);
    Triangulation imported(d);
    verify(imported);
    check(imported.triangle_faces().size() == 2, "DCEL import.");
    Triangulation selected(d, {d.left_face(d.find_halfedge(0,1))});
    verify(selected);
    check(selected.is_boundary_edge(selected.dcel().find_halfedge(0,2)), "Subdomain diagonal boundary.");
    check(selected.locate({0.5,1.5}).type == Type::OUTSIDE && area(selected) == 2, "Selected domain only.");

    rejected([&] { t.triangle_vertices(t.dcel().outer_face); });
    rejected([&] { t.opposite_vertex(t.dcel().halfedges[boundary].twin); });
    rejected([&] { t.can_flip(-1); });
    auto unchanged = snapshot(t);
    rejected([&] { t.flip_edge(-1); });
    check(snapshot(t) == unchanged, "Invalid flip ID changed state.");
    rejected([&] { t.is_triangle(999); });
    rejected([&] { t.incident_triangles(999); });
    rejected([&] { t.set_constrained(999); });
    rejected([&] { t.locate({std::numeric_limits<double>::quiet_NaN(),0}); });
    rejected([&] { t.locate({0,std::numeric_limits<double>::infinity()}); });
    rejected([&] { Triangulation(d, {d.outer_face}); });
    rejected([&] { Triangulation(d, {1,1}); });
    rejected([&] { Triangulation(d, {-1}); });
    rejected([&] { Triangulation(DCEL({{0,0},{2,0},{2,2},{0,2}}, {{0,1},{1,2},{2,3},{3,0}})); });
    rejected([&] { Triangulation::from_triangles({{0,0},{1,0},{2,0}}, {{0,1,2}}); });
    rejected([&] { Triangulation::from_triangles({{0,0},{1,0},{0,1}}, {{0,0,1}}); });
    rejected([&] { Triangulation::from_triangles({{0,0},{1,0},{0,1}}, {{0,1,9}}); });
    rejected([&] { Triangulation::from_triangles({{0,0},{1,0},{0,1}}, {{0,1,2},{2,1,0}}); });
    rejected([&] { Triangulation::from_triangles({{0,0},{4,0},{0,4},{1,-1},{1,3},{3,1}}, {{0,1,2},{3,4,5}}); });
    rejected([&] { Triangulation::from_triangles({{0,0},{8,0},{0,8},{1,1},{2,1},{1,2}}, {{0,1,2},{3,4,5}}); });
    rejected([&] { Triangulation::from_triangles({{0,0},{2,0},{0,2},{0.5,0.5}}, {{0,1,2}}); });
    auto malformed = d;
    malformed.halfedges[0].next = -1;
    rejected([&] { (void)Triangulation(malformed); });
    malformed = d;
    malformed.vertices[2].point = {-1,-1};
    rejected([&] { (void)Triangulation(malformed); });
    auto tiny = Triangulation::from_triangles({{0,0},{2e-12,0},{2e-12,2e-12},{0,2e-12}}, {{0,1,2},{0,2,3}});
    check(tiny.flip_edge(tiny.dcel().find_halfedge(0,2)), "Small triangles must not collapse under a fixed EPS.");
    ++flips;
    verify(tiny);
    check(tiny.locate({1e-12,1e-12}).type == Type::EDGE, "Small triangle location.");
}

void flip_and_protection() {
    for (bool use_twin : {false, true}) {
        auto t = square();
        int e = t.dcel().find_halfedge(0,2);
        if (use_twin) e = t.dcel().halfedges[e].twin;
        int twin = t.dcel().halfedges[e].twin;
        auto before = snapshot(t);
        t.set_constrained(e);
        check(t.is_constrained(e) && t.is_constrained(twin), "Constraint shared by twins.");
        auto protected_state = snapshot(t);
        check(!t.can_flip(e) && !t.flip_edge(twin), "Constrained diagonal cannot flip.");
        check(snapshot(t) == protected_state, "Rejected flip changed state.");
        t.set_constrained(twin, false);
        check(snapshot(t) == before, "Constraint removal.");
        check(t.can_flip(e) && t.can_flip(twin) && t.flip_edge(e), "Convex edge flip.");
        ++flips;
        verify(t);
        check(t.dcel().find_halfedge(0,2) == -1 && t.dcel().find_halfedge(1,3) != -1, "Flipped endpoints.");
        check(t.dcel().vertices.size() == 4 && t.dcel().halfedges.size() == 10 && t.dcel().faces.size() == 3,
            "Flip preserves element counts.");
        check(t.dcel().halfedges[e].twin == twin && t.triangle_faces().size() == 2 && area(t) == 4,
            "Flip preserves IDs, triangle count and area.");
        check(t.incident_triangles(0).size() == 1 && t.incident_triangles(1).size() == 2, "Updated vertex incidence.");
        check(t.locate({1,1}).type == Type::EDGE && t.locate({0.5,0.5}).type == Type::FACE, "Updated location.");
        check(t.flip_edge(twin), "Inverse flip.");
        ++flips;
        verify(t);
        check(t.dcel().find_halfedge(0,2) != -1 && area(t) == 4, "Inverse flip restores geometry.");
        int boundary = t.dcel().find_halfedge(0,1);
        before = snapshot(t);
        check(!t.can_flip(boundary) && !t.flip_edge(boundary), "Boundary cannot flip.");
        check(snapshot(t) == before, "Boundary flip rejection changed state.");
        t.set_constrained(boundary);
        verify(t);
    }
    auto concave = Triangulation::from_triangles({{0,0},{4,0},{1,1},{0,4}}, {{0,1,2},{0,2,3}});
    int e = concave.dcel().find_halfedge(0,2);
    auto before = snapshot(concave);
    check(!concave.can_flip(e) && !concave.flip_edge(e) && snapshot(concave) == before, "Concave flip rejection.");
    verify(concave);
    auto collinear = Triangulation::from_triangles({{0,0},{2,0},{1,1},{0,2}}, {{0,1,2},{0,2,3}});
    e = collinear.dcel().find_halfedge(0,2);
    check(!collinear.can_flip(e), "Collinear flip rejection.");
    verify(collinear);
}

void holes_and_disconnected_domains() {
    std::vector<Point> points{{0,0},{6,0},{6,6},{0,6},{2,2},{4,2},{4,4},{2,4}};
    auto ring = Triangulation::from_triangles(points,
        {{0,1,5},{0,5,4},{1,2,6},{1,6,5},{2,3,7},{2,7,6},{3,0,4},{3,4,7}});
    verify(ring);
    check(ring.triangle_faces().size() == 8 && area(ring) == 32, "Ring area and triangle count.");
    int hole = ring.dcel().left_face(ring.dcel().find_halfedge(4,5));
    check(hole != ring.dcel().outer_face && !ring.is_triangle(hole), "Finite hole is outside domain.");
    check(ring.locate({3,3}).type == Type::OUTSIDE && ring.locate({7,3}).type == Type::OUTSIDE, "Hole/exterior location.");
    auto hole_boundary = ring.locate({3,2});
    check(hole_boundary.type == Type::EDGE && ring.is_boundary_edge(hole_boundary.halfedge), "Hole boundary location.");
    check(ring.locate({3,1}).type == Type::FACE, "Ring interior location.");
    int boundary = ring.dcel().find_halfedge(4,5);
    check(!ring.can_flip(boundary), "Hole boundary cannot flip.");
    rejected([&] { Triangulation(ring.dcel()); });
    Triangulation imported(ring.dcel(), ring.triangle_faces());
    verify(imported);
    check(imported.locate({3,3}).type == Type::OUTSIDE, "DCEL domain selection preserves holes.");
    int protected_edge = ring.dcel().find_halfedge(0,5);
    check(ring.can_flip(protected_edge), "Internal ring diagonal initially flippable.");
    ring.set_constrained(protected_edge);
    for (int e = 0; e < static_cast<int>(ring.dcel().halfedges.size()); ++e) {
        if (ring.can_flip(e)) {
            check(ring.flip_edge(e), "Ring edge flip.");
            ++flips;
            verify(ring);
            check(area(ring) == 32 && ring.locate({3,3}).type == Type::OUTSIDE, "Flips preserve ring domain.");
            check(ring.is_constrained(protected_edge) && ring.dcel().find_halfedge(0,5) == protected_edge &&
                !ring.can_flip(protected_edge), "Other flips preserve constrained segment.");
        }
    }
    auto triangular_hole = Triangulation::from_triangles({{0,0},{6,0},{0,6},{1,1},{3,1},{1,3}},
        {{0,1,4},{0,4,3},{1,2,5},{1,5,4},{2,0,3},{2,3,5}});
    verify(triangular_hole);
    int excluded = triangular_hole.dcel().left_face(triangular_hole.dcel().find_halfedge(3,4));
    check(!triangular_hole.is_triangle(excluded) && triangular_hole.dcel().face_vertices(excluded).size() == 3,
        "Triangular hole must not be inferred as domain.");
    check(triangular_hole.locate({1.5,1.5}).type == Type::OUTSIDE && area(triangular_hole) == 16, "Triangular hole location and area.");
    Triangulation filled(triangular_hole.dcel());
    verify(filled);
    check(filled.locate({1.5,1.5}).type == Type::FACE && area(filled) == 18, "All bounded faces explicitly fills triangular hole.");
    auto disconnected = Triangulation::from_triangles({{0,0},{1,0},{0,1},{3,0},{4,0},{3,1},{8,8}}, {{0,1,2},{3,4,5}});
    verify(disconnected);
    check(disconnected.triangle_faces().size() == 2 && area(disconnected) == 1, "Disconnected domain.");
    check(disconnected.locate({2,0.1}).type == Type::OUTSIDE, "Between components.");
    check(disconnected.incident_triangles(6).empty() && disconnected.locate({8,8}).type == Type::OUTSIDE,
        "Unused vertices are outside the triangular domain.");
    auto empty = Triangulation::from_triangles({{0,0},{1,0}}, {});
    verify(empty);
    check(empty.triangle_faces().empty() && empty.locate({0,0}).type == Type::OUTSIDE, "No triangles means empty domain.");
}

void random_flip_sequences() {
    std::mt19937 random(20261006);
    std::uniform_int_distribution<int> coordinate(-100, 100);
    for (int trial = 0; trial < 200; ++trial) {
        std::vector<Point> sample;
        for (int i = 0; i < 40; ++i) sample.emplace_back(coordinate(random), coordinate(random));
        auto hull = convex_hull(sample);
        auto points = hull.points;
        std::vector<std::array<int, 3>> triangles;
        for (int i = 1; i + 1 < static_cast<int>(points.size()); ++i) triangles.push_back({0,i,i+1});
        auto t = Triangulation::from_triangles(points, triangles);
        double original_area = area(t);
        verify(t);
        for (int mutation = 0; mutation < 50; ++mutation) {
            std::vector<int> candidates;
            for (int e = 0; e < static_cast<int>(t.dcel().halfedges.size()); ++e)
                if (t.can_flip(e)) candidates.push_back(e);
            if (candidates.empty()) break;
            int e = candidates[random() % candidates.size()];
            int twin = t.dcel().halfedges[e].twin;
            check(t.can_flip(twin) && t.flip_edge(e), "Random flip rejected.");
            ++flips;
            verify(t);
            check(area(t) == original_area && t.triangle_faces().size() + 2 == points.size(), "Random area/count conservation.");
            for (int v = 0; v < static_cast<int>(points.size()); ++v) {
                auto location = t.locate(points[v]);
                check(location.type == Type::VERTEX && location.vertex == v, "Random vertex location after flip.");
            }
        }
    }
}
}

int main() {
    try {
        basic_queries_and_import();
        flip_and_protection();
        holes_and_disconnected_domains();
        random_flip_sequences();
        std::cout << "Triangulation: " << checks << " checks passed; " << flips << " flips verified.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Triangulation test failure after " << checks << " checks: " << error.what() << '\n';
        return 1;
    }
}
