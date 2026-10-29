#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>

#include "../structure/dcel.hpp"
#include "../algorithm/convex_hull.hpp"

namespace {
int checks = 0;
int mutations = 0;

void check(bool condition, const char* message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}
void verify(const DCEL& d) {
    check(d.is_valid(), "Structural validation failed.");
    check(d.is_valid_geometry(), "Geometry/rebuild validation failed.");
}
std::string snapshot(const DCEL& d) {
    std::ostringstream out;
    out.precision(17);
    out << d.outer_face << ':';
    for (const auto& v : d.vertices) out << v.point.x << ',' << v.point.y << ',' << v.halfedge << ',' << v.face << ';';
    out << '|';
    for (const auto& e : d.halfedges) out << e.origin << ',' << e.twin << ',' << e.next << ',' << e.prev << ',' << e.face << ';';
    out << '|';
    for (const auto& f : d.faces) {
        out << f.halfedge << ':';
        for (int e : f.inner_halfedges) out << e << ',';
        out << ':';
        for (int v : f.isolated_vertices) out << v << ',';
        out << ';';
    }
    return out.str();
}
template <class Operation>
void rejected_without_change(DCEL& d, Operation operation) {
    auto before = snapshot(d);
    bool rejected = false;
    try { operation(); }
    catch (const std::invalid_argument&) { rejected = true; }
    catch (const std::out_of_range&) { rejected = true; }
    check(rejected, "Invalid operation was accepted.");
    check(before == snapshot(d), "Rejected operation changed the DCEL.");
    verify(d);
}
DCEL square() {
    return DCEL({{0,0},{2,0},{2,2},{0,2}}, {{0,1},{1,2},{2,3},{3,0}});
}

void basic_and_invalid_cases() {
    DCEL empty;
    verify(empty);
    check(empty.faces.size() == 1 && empty.face_halfedges(empty.outer_face).empty(), "Empty subdivision.");
    DCEL isolated({{0,0},{1,1}}, {});
    verify(isolated);
    check(isolated.faces[isolated.outer_face].isolated_vertices.size() == 2, "Isolated ownership.");

    auto d = square();
    int f = d.left_face(d.find_halfedge(0,1));
    check(d.face_signed_area(f) == 4 && d.face_signed_area(d.outer_face) == -4, "Square boundary areas.");
    rejected_without_change(d, [&] { d.split_face(f,0,1); });
    rejected_without_change(d, [&] { d.split_face(f,0,0); });
    rejected_without_change(d, [&] { d.split_face(d.outer_face,0,2); });
    rejected_without_change(d, [&] { d.split_face(-1,0,2); });
    rejected_without_change(d, [&] { d.split_edge(-1,{1,0}); });
    rejected_without_change(d, [&] { d.split_edge(d.find_halfedge(0,1),{1,3}); });
    rejected_without_change(d, [&] { d.split_edge(d.find_halfedge(0,1),{0,0}); });
    rejected_without_change(d, [&] { d.split_edge(d.find_halfedge(0,1),{2,0}); });
    rejected_without_change(d, [&] { d.split_edge(d.find_halfedge(0,1),{std::numeric_limits<double>::quiet_NaN(),0}); });

    auto missing = d;
    missing.vertices[0].halfedge = -1;
    check(!missing.is_valid(), "Missing representative was accepted.");
    auto bad_twin = d;
    bad_twin.halfedges[0].twin = 0;
    check(!bad_twin.is_valid(), "Self twin was accepted.");
    auto bad_next = d;
    bad_next.halfedges[0].next = static_cast<int>(bad_next.halfedges.size());
    check(!bad_next.is_valid(), "Bad next ID was accepted.");
    auto wrong_geometry = d;
    wrong_geometry.vertices[1].point = {-1,1};
    check(wrong_geometry.is_valid() && !wrong_geometry.is_valid_geometry(), "Crossing geometry escaped validation.");
    auto missing_boundary = d;
    missing_boundary.faces[missing_boundary.outer_face].inner_halfedges.clear();
    check(!missing_boundary.is_valid(), "Missing exterior boundary was accepted.");

    rejected_without_change(d, [&] { d.build({{0,0}}, {{0,1}}); });
    rejected_without_change(d, [&] { d.build({{0,0}}, {{-1,0}}); });
    rejected_without_change(d, [&] { d.build({{0,0}}, {{0,0}}); });
    rejected_without_change(d, [&] { d.build({{0,0},{0,0}}, {}); });
    rejected_without_change(d, [&] { d.build({{0,0},{2,0}}, {{0,1},{1,0}}); });
    rejected_without_change(d, [&] { d.build({{0,0},{2,0},{1,0}}, {{0,1}}); });
    rejected_without_change(d, [&] { d.build({{0,0},{2,2},{0,2},{2,0}}, {{0,1},{2,3}}); });
    rejected_without_change(d, [&] { d.build({{std::numeric_limits<double>::infinity(),0}}, {}); });
    rejected_without_change(d, [&] { d.build({{-1e308,0},{1e308,0}}, {{0,1}}); });

    DCEL concave({{0,0},{3,0},{3,3},{2,3},{2,1},{1,1},{1,3},{0,3}},
                 {{0,1},{1,2},{2,3},{3,4},{4,5},{5,6},{6,7},{7,0}});
    int interior = concave.left_face(concave.find_halfedge(0,1));
    rejected_without_change(concave, [&] { concave.split_face(interior,3,6); });
    rejected_without_change(concave, [&] { concave.split_face(interior,4,7); });

    DCEL bridge({{0,0},{2,0}}, {{0,1}});
    for (int i = 0; i < 8; ++i) {
        int e = bridge.find_halfedge(0, i == 0 ? 1 : i + 1);
        Point p = (bridge.vertices[0].point + bridge.vertices[bridge.target(e)].point) / 2;
        bridge.split_edge(e,p);
        verify(bridge);
    }
}

void repeated_boundary_vertex() {
    auto make = [] {
        return DCEL({{0,0},{2,0},{2,2},{0,2},{0.5,1}}, {{0,4},{0,1},{1,2},{2,3},{3,0}});
    };
    auto d = make();
    int f = d.left_face(d.find_halfedge(0,1));
    int wrong = d.find_halfedge_on_face(f,0);
    check(d.target(wrong) == 4, "Regression input no longer has the first-occurrence ambiguity.");
    rejected_without_change(d, [&] { d.split_face_at_corners(wrong,d.find_halfedge(2,3)); });
    rejected_without_change(d, [&] { d.split_face(f,0,1); });
    int e = d.split_face(f,0,2);
    check(d.halfedges[e].origin == 0 && d.target(e) == 2, "Diagonal return value.");
    verify(d);
    check(d.faces.size() == 3, "Repeated-vertex face split count.");
    auto explicit_corners = make();
    explicit_corners.split_face_at_corners(explicit_corners.find_halfedge(0,1),explicit_corners.find_halfedge(2,3));
    verify(explicit_corners);

    DCEL two_branches({{0,0},{2,0},{2,2},{0,2},{0.5,1},{1,0.5}},
                      {{0,4},{0,5},{0,1},{1,2},{2,3},{3,0}});
    int interior = two_branches.left_face(two_branches.find_halfedge(0,1));
    int diagonal = two_branches.split_face(interior,0,2);
    verify(two_branches);
    check(two_branches.left_face(two_branches.find_halfedge(0,4)) == two_branches.left_face(diagonal),
          "Upper branch belongs to the wrong split region.");
    check(two_branches.left_face(two_branches.find_halfedge(0,5)) == two_branches.right_face(diagonal),
          "Lower branch belongs to the wrong split region.");
}

void disconnected_and_nested_faces() {
    DCEL triangles({{0,0},{2,0},{0,2},{5,0},{7,0},{5,2}}, {{0,1},{1,2},{2,0},{3,4},{4,5},{5,3}});
    verify(triangles);
    check(triangles.faces.size() == 3 && triangles.faces[triangles.outer_face].inner_halfedges.size() == 2,
          "Disjoint triangles need one shared exterior face.");
    DCEL trees({{0,0},{2,0},{0,5},{2,5}}, {{0,1},{2,3}});
    verify(trees);
    check(trees.faces.size() == 1 && trees.face_boundaries(trees.outer_face).size() == 2, "Disjoint trees.");

    DCEL nested({{0,0},{10,0},{10,10},{0,10},{1,1},{3,1},{3,3},{1,3},{2,2},{5,5},{20,20}},
                {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4}});
    verify(nested);
    int annulus = nested.left_face(nested.find_halfedge(0,1));
    int inner = nested.left_face(nested.find_halfedge(4,5));
    check(nested.faces.size() == 3 && nested.face_signed_area(annulus) == 96 && nested.face_signed_area(inner) == 4,
          "Nested boundary ownership or area.");
    check(nested.vertices[8].face == inner && nested.vertices[9].face == annulus && nested.vertices[10].face == nested.outer_face,
          "Nested isolated vertices belong to the wrong face.");
    rejected_without_change(nested, [&] { nested.split_face(annulus,0,4); });

    DCEL connected_hole({{0,0},{10,0},{10,10},{0,10},{1,1},{3,1},{3,3},{1,3}},
                        {{0,4},{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4}});
    verify(connected_hole);
    int connected_annulus = connected_hole.left_face(connected_hole.find_halfedge(0,1));
    check(connected_hole.face_signed_area(connected_annulus) == 96, "Bridge-connected annulus area.");
    int connector = connected_hole.split_face(connected_annulus,2,6);
    verify(connected_hole);
    check(connected_hole.face_signed_area(connected_hole.left_face(connector)) == 48 &&
          connected_hole.face_signed_area(connected_hole.right_face(connector)) == 48,
          "Repeated-boundary annulus split areas.");
    connected_hole.split_edge(connected_hole.find_halfedge(0,4),{0.5,0.5});
    verify(connected_hole);

    DCEL holes({{0,0},{10,0},{10,10},{0,10},
                {6,1},{8,1},{8,3},{6,3},{1,6},{3,6},{3,8},{1,8},
                {8,5},{5,8},{4,7},{4,8},{7,4},{8,4}},
               {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},
                {8,9},{9,10},{10,11},{11,8},{14,15},{16,17}});
    verify(holes);
    int old_face = holes.left_face(holes.find_halfedge(0,1));
    check(holes.face_signed_area(old_face) == 92, "Area before hole redistribution.");
    int diagonal = holes.split_face(old_face,0,2);
    int new_face = holes.left_face(diagonal);
    verify(holes);
    check(holes.face_signed_area(old_face) == 46 && holes.face_signed_area(new_face) == 46, "Split areas with holes.");
    check(holes.faces[old_face].inner_halfedges.size() == 2 && holes.faces[new_face].inner_halfedges.size() == 2,
          "Holes/trees were not redistributed.");
    check(holes.vertices[12].face == old_face && holes.vertices[13].face == new_face, "Isolated redistribution.");
    check(holes.right_face(holes.find_halfedge(4,5)) == old_face && holes.right_face(holes.find_halfedge(8,9)) == new_face,
          "Hole halfedge face IDs were not updated.");
    holes.split_edge(holes.find_halfedge(4,5),{7,1});
    holes.split_edge(holes.find_halfedge(14,15),{4,7.5});
    verify(holes);
    check(holes.face_signed_area(old_face) == 46 && holes.face_signed_area(new_face) == 46, "Boundary subdivision changed area.");
}

void nested_permutations_and_grid() {
    std::vector<Point> points = {{0,0},{12,0},{12,12},{0,12},
                                {2,2},{10,2},{10,10},{2,10},
                                {4,4},{6,4},{6,6},{4,6},{5,5},{3,3},{1,1}};
    std::vector<std::pair<int,int>> edges = {{0,1},{1,2},{2,3},{3,0},
                                            {4,5},{5,6},{6,7},{7,4},
                                            {8,9},{9,10},{10,11},{11,8}};
    std::mt19937 rng(734);
    for (int trial = 0; trial < 50; ++trial) {
        std::shuffle(edges.begin(),edges.end(),rng);
        for (auto& edge : edges) if (rng()%2) std::swap(edge.first,edge.second);
        DCEL d(points,edges);
        verify(d);
        int outer_ring = d.left_face(d.find_halfedge(0,1));
        int middle_ring = d.left_face(d.find_halfedge(4,5));
        int inner = d.left_face(d.find_halfedge(8,9));
        check(d.faces.size() == 4 && d.face_signed_area(outer_ring) == 80 &&
              d.face_signed_area(middle_ring) == 60 && d.face_signed_area(inner) == 4,
              "Three-level nesting depends on input ordering.");
        check(d.vertices[12].face == inner && d.vertices[13].face == middle_ring && d.vertices[14].face == outer_ring,
              "Three-level isolated ownership.");
    }

    std::vector<Point> grid;
    std::vector<std::pair<int,int>> grid_edges;
    for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) {
        grid.emplace_back(2*x,2*y);
        if (x < 2) grid_edges.emplace_back(3*y+x,3*y+x+1);
        if (y < 2) grid_edges.emplace_back(3*y+x,3*(y+1)+x);
    }
    DCEL d(grid,grid_edges);
    verify(d);
    check(d.faces.size() == 5, "Grid face count.");
    for (int f = 1; f <= 4; ++f) {
        auto vs = d.face_vertices(f);
        check(vs.size() == 4 && d.face_signed_area(f) == 4, "Grid square boundary.");
        d.split_face(f,vs[0],vs[2]);
        verify(d);
    }
    check(d.faces.size() == 9, "Grid triangulation face count.");
}

void random_mutations() {
    std::mt19937 rng(20261006);
    for (int trial = 0; trial < 500; ++trial) {
        std::vector<Point> cloud;
        for (int i = 0; i < 40; ++i)
            cloud.emplace_back(static_cast<int>(rng()%2001)-1000,static_cast<int>(rng()%2001)-1000);
        auto hull = convex_hull(cloud);
        check(hull.size() >= 3, "Unexpected degenerate random hull.");
        std::vector<std::pair<int,int>> edges;
        for (int i = 0; i < hull.size(); ++i) edges.emplace_back(i,(i+1)%hull.size());
        DCEL d(hull.points,edges);
        verify(d);
        double original_area = d.face_signed_area(d.left_face(d.find_halfedge(0,1)));
        for (;;) {
            int f = -1;
            std::vector<int> vs;
            for (int i = 0; i < static_cast<int>(d.faces.size()); ++i) if (i != d.outer_face) {
                auto boundary = d.face_vertices(i);
                if (boundary.size() > 3) { f = i; vs = std::move(boundary); break; }
            }
            if (f == -1) break;
            size_t start = rng()%vs.size(), offset = 2 + rng()%(vs.size()-3);
            auto before_v = d.vertices.size(), before_e = d.halfedges.size(), before_f = d.faces.size();
            d.split_face(f,vs[start],vs[(start+offset)%vs.size()]);
            check(d.vertices.size() == before_v && d.halfedges.size() == before_e+2 && d.faces.size() == before_f+1,
                  "Face split counts.");
            verify(d);
            ++mutations;
        }
        double area = 0;
        for (int f = 0; f < static_cast<int>(d.faces.size()); ++f) if (f != d.outer_face) area += d.face_signed_area(f);
        check(area == original_area, "Triangulation failed to conserve area.");
        check(d.faces.size() == hull.points.size()-1, "Convex n-gon should yield n-2 bounded triangles.");
        for (int i = 0; i < 20; ++i) {
            int e = static_cast<int>(rng()%d.halfedges.size());
            Point p = (d.vertices[d.halfedges[e].origin].point + d.vertices[d.target(e)].point)/2;
            auto before_v = d.vertices.size(), before_e = d.halfedges.size(), before_f = d.faces.size();
            d.split_edge(e,p);
            check(d.vertices.size() == before_v+1 && d.halfedges.size() == before_e+2 && d.faces.size() == before_f,
                  "Edge split counts.");
            verify(d);
            ++mutations;
        }
    }
    for (int trial = 0; trial < 100; ++trial) {
        DCEL d({{0,0},{2,0},{0,2},{-2,0},{0,-2}}, {{0,1},{0,2},{0,3},{0,4}});
        for (int i = 0; i < 20; ++i) {
            int e = static_cast<int>(rng()%d.halfedges.size());
            Point p = (d.vertices[d.halfedges[e].origin].point + d.vertices[d.target(e)].point)/2;
            d.split_edge(e,p);
            verify(d);
            check(d.faces.size() == 1, "Tree subdivision created a face.");
            ++mutations;
        }
    }
}
}

int main() {
    try {
        basic_and_invalid_cases();
        repeated_boundary_vertex();
        disconnected_and_nested_faces();
        nested_permutations_and_grid();
        random_mutations();
        std::cout << "DCEL tests passed: " << checks << " checks, " << mutations << " randomized mutations.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "DCEL test failure: " << error.what() << '\n';
        return 1;
    }
}
