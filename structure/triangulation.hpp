#pragma once

#include <array>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

#include "dcel.hpp"

class Triangulation {
public:
    enum class LocationType { VERTEX, EDGE, FACE, OUTSIDE };
    struct Location {
        LocationType type = LocationType::OUTSIDE;
        int face = -1;
        int halfedge = -1;
        int vertex = -1;
    };

    Triangulation() = default;

    explicit Triangulation(DCEL subdivision) : subdivision_(std::move(subdivision)) {
        in_domain_.assign(subdivision_.faces.size(), false);
        for (int f = 0; f < static_cast<int>(subdivision_.faces.size()); ++f)
            if (f != subdivision_.outer_face) in_domain_[f] = true;
        finish_construction();
    }

    Triangulation(DCEL subdivision, const std::vector<int>& triangle_faces)
        : subdivision_(std::move(subdivision)) {
        in_domain_.assign(subdivision_.faces.size(), false);
        for (int f : triangle_faces) {
            check_index(f, in_domain_.size());
            if (f == subdivision_.outer_face || in_domain_[f])
                throw std::invalid_argument("Domain faces must be distinct bounded triangles.");
            in_domain_[f] = true;
        }
        finish_construction();
    }

    static Triangulation from_triangles(const std::vector<Point>& points,
        const std::vector<std::array<int, 3>>& triangles) {
        std::set<std::pair<int, int>> edges;
        for (auto vertices : triangles) {
            for (int v : vertices) check_index(v, points.size());

            auto area = turn(points[vertices[0]], points[vertices[1]], points[vertices[2]]);
            if (!std::isfinite(area) || area == 0)
                throw std::invalid_argument("Triangles must have finite, nonzero area.");

            for (int i = 0; i < 3; ++i)
                edges.emplace(std::min(vertices[i], vertices[(i + 1) % 3]),
                    std::max(vertices[i], vertices[(i + 1) % 3]));
        }

        DCEL subdivision(points, std::vector<std::pair<int, int>>(edges.begin(), edges.end()));
        std::vector<int> selected;
        for (auto vertices : triangles) {
            if (turn(points[vertices[0]], points[vertices[1]], points[vertices[2]]) < 0)
                std::swap(vertices[1], vertices[2]);

            int e = subdivision.find_halfedge(vertices[0], vertices[1]);
            int f = subdivision.left_face(e);
            auto actual = subdivision.face_vertices(f);
            std::sort(actual.begin(), actual.end());
            std::sort(vertices.begin(), vertices.end());

            if (f == subdivision.outer_face || actual.size() != 3 ||
                !std::equal(actual.begin(), actual.end(), vertices.begin()))
                throw std::invalid_argument("Input triangles must be faces of a planar subdivision.");

            selected.push_back(f);
        }

        return Triangulation(std::move(subdivision), selected);
    }

    const DCEL& dcel() const { return subdivision_; }
    const std::vector<int>& triangle_faces() const { return triangle_faces_; }
    bool is_triangle(int f) const {
        check_index(f, in_domain_.size());
        return in_domain_[f];
    }
    std::array<int, 3> triangle_halfedges(int f) const {
        require_triangle(f);
        int e = subdivision_.faces[f].halfedge;
        int next = subdivision_.halfedges[e].next;
        return { e, next, subdivision_.halfedges[next].next };
    }
    std::array<int, 3> triangle_vertices(int f) const {
        auto edges = triangle_halfedges(f);
        return { subdivision_.halfedges[edges[0]].origin, subdivision_.halfedges[edges[1]].origin,
            subdivision_.halfedges[edges[2]].origin };
    }
    std::array<int, 3> triangle_neighbors(int f) const {
        auto edges = triangle_halfedges(f);
        std::array<int, 3> result;
        for (int i = 0; i < 3; ++i) {
            int neighbor = subdivision_.right_face(edges[i]);
            result[i] = in_domain_[neighbor] ? neighbor : -1;
        }
        return result;
    }
    std::vector<int> incident_triangles(int v) const {
        std::vector<int> result;
        for (int e : subdivision_.vertex_halfedges(v)) {
            int f = subdivision_.left_face(e);
            if (in_domain_[f]) result.push_back(f);
        }
        return result;
    }
    int opposite_vertex(int e) const {
        int f = subdivision_.left_face(e);
        require_triangle(f);
        return subdivision_.halfedges[subdivision_.halfedges[e].prev].origin;
    }
    double triangle_area(int f) const {
        auto v = triangle_vertices(f);
        return static_cast<double>(turn(point(v[0]), point(v[1]), point(v[2])) / 2);
    }
    bool is_boundary_edge(int e) const {
        return in_domain_[subdivision_.left_face(e)] != in_domain_[subdivision_.right_face(e)];
    }
    bool is_interior_edge(int e) const {
        return in_domain_[subdivision_.left_face(e)] && in_domain_[subdivision_.right_face(e)];
    }
    bool is_constrained(int e) const {
        check_index(e, constrained_.size());
        return constrained_[e];
    }
    void set_constrained(int e, bool constrained = true) {
        check_index(e, constrained_.size());
        if (!in_domain_[subdivision_.left_face(e)] && !in_domain_[subdivision_.right_face(e)])
            throw std::invalid_argument("Constraints must belong to the triangulated domain.");
        constrained_[e] = constrained_[subdivision_.halfedges[e].twin] = constrained;
    }

    bool can_flip(int e) const {
        check_index(e, subdivision_.halfedges.size());
        if (!is_interior_edge(e) || constrained_[e]) return false;

        int t = subdivision_.halfedges[e].twin;
        int a = subdivision_.halfedges[e].origin, b = subdivision_.halfedges[t].origin;
        int c = opposite_vertex(e), d = opposite_vertex(t);
        if (a == b || c == d || subdivision_.find_halfedge(c, d) != -1) return false;

        auto left = turn(point(c), point(d), point(b));
        auto right = turn(point(d), point(c), point(a));
        return std::isfinite(left) && std::isfinite(right) && left > 0 && right > 0;
    }
    bool flip_edge(int e) {
        if (!can_flip(e)) return false;

        auto& h = subdivision_.halfedges;
        int t = h[e].twin, en = h[e].next, ep = h[e].prev;
        int tn = h[t].next, tp = h[t].prev;
        int a = h[e].origin, b = h[t].origin, c = h[ep].origin, d = h[tp].origin;
        int f = h[e].face, g = h[t].face;
        h[e].origin = c;
        h[t].origin = d;

        auto link_triangle = [&](int face, int first, int second, int third) {
            h[first].next = second; h[second].next = third; h[third].next = first;
            h[first].prev = third; h[second].prev = first; h[third].prev = second;
            h[first].face = h[second].face = h[third].face = face;
            subdivision_.faces[face].halfedge = first;
        };
        link_triangle(f, e, tp, en);
        link_triangle(g, t, ep, tn);

        if (subdivision_.vertices[a].halfedge == e) subdivision_.vertices[a].halfedge = tn;
        if (subdivision_.vertices[b].halfedge == t) subdivision_.vertices[b].halfedge = en;
        assert(is_valid());
        return true;
    }

    Location locate(const Point& p) const {
        if (!std::isfinite(p.x) || !std::isfinite(p.y))
            throw std::invalid_argument("Point-location queries require finite coordinates.");
        for (int f : triangle_faces_) {
            auto edges = triangle_halfedges(f);
            for (int e : edges) {
                int v = subdivision_.halfedges[e].origin;
                if (point(v) == p) return { LocationType::VERTEX, f, -1, v };
            }
            bool inside = true;
            int boundary = -1;
            for (int e : edges) {
                auto side = turn(point(subdivision_.halfedges[e].origin), point(subdivision_.target(e)), p);
                if (!std::isfinite(side))
                    throw std::overflow_error("Point-location predicate overflow.");
                if (side < 0) inside = false;
                if (side == 0) boundary = e;
            }
            if (inside) return boundary == -1 ? Location{ LocationType::FACE, f, -1, -1 }
                : Location{ LocationType::EDGE, f, boundary, -1 };
        }
        return {};
    }

    bool is_valid() const {
        if (!subdivision_.is_valid() || in_domain_.size() != subdivision_.faces.size() ||
            constrained_.size() != subdivision_.halfedges.size() || in_domain_[subdivision_.outer_face]) return false;

        std::vector<bool> seen(in_domain_.size(), false);
        for (int f : triangle_faces_) {
            if (f < 0 || static_cast<size_t>(f) >= seen.size() || seen[f] || !in_domain_[f]) return false;
            seen[f] = true;
        }

        if (seen != in_domain_) return false;

        for (int f : triangle_faces_) {
            const auto& face = subdivision_.faces[f];
            if (face.halfedge == -1 || !face.inner_halfedges.empty() || !face.isolated_vertices.empty()) return false;

            int a = face.halfedge, b = subdivision_.halfedges[a].next, c = subdivision_.halfedges[b].next;
            if (a == b || b == c || c == a || subdivision_.halfedges[c].next != a) return false;

            auto v = triangle_vertices(f);
            auto area = turn(point(v[0]), point(v[1]), point(v[2]));
            if (!std::isfinite(area) || area <= 0) return false;
        }
        for (int e = 0; e < static_cast<int>(constrained_.size()); ++e) {
            if (constrained_[e] != constrained_[subdivision_.halfedges[e].twin]) return false;
            if (constrained_[e] && !in_domain_[subdivision_.left_face(e)] && !in_domain_[subdivision_.right_face(e)])
                return false;
        }
        return true;
    }
    bool is_valid_geometry() const { return is_valid() && subdivision_.is_valid_geometry(); }

private:
    DCEL subdivision_;
    std::vector<bool> in_domain_{ false };
    std::vector<int> triangle_faces_;
    std::vector<bool> constrained_;

    static void check_index(int index, size_t size) {
        if (index < 0 || static_cast<size_t>(index) >= size) throw std::out_of_range("Invalid triangulation ID.");
    }
    void require_triangle(int f) const {
        if (!is_triangle(f)) throw std::invalid_argument("Face is outside the triangulated domain.");
    }
    const Point& point(int v) const { return subdivision_.vertices[v].point; }
    static long double turn(const Point& a, const Point& b, const Point& c) {
        long double x1 = static_cast<long double>(b.x) - a.x, y1 = static_cast<long double>(b.y) - a.y;
        long double x2 = static_cast<long double>(c.x) - a.x, y2 = static_cast<long double>(c.y) - a.y;
        return x1 * y2 - y1 * x2;
    }
    void finish_construction() {
        constrained_.assign(subdivision_.halfedges.size(), false);
        for (int f = 0; f < static_cast<int>(in_domain_.size()); ++f)
            if (in_domain_[f]) triangle_faces_.push_back(f);
        if (!is_valid_geometry())
            throw std::invalid_argument("Triangulation requires a valid planar DCEL with triangular domain faces.");
    }
};

inline Triangulation::Location locate(const Triangulation& triangulation, const Point& p) {
    return triangulation.locate(p);
}
