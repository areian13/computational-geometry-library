#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <numeric>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

#include "../geometry/point.hpp"

struct DCEL {
    struct Vertex {
        Point point;
        int halfedge = -1;
        int face = -1;
    };
    struct HalfEdge {
        int origin = -1;
        int twin = -1;
        int next = -1;
        int prev = -1;
        int face = -1;
    };
    struct Face {
        int halfedge = -1;
        std::vector<int> inner_halfedges;
        std::vector<int> isolated_vertices;
    };

    std::vector<Vertex> vertices;
    std::vector<HalfEdge> halfedges;
    std::vector<Face> faces{ Face{} };
    int outer_face = 0;

    DCEL() = default;
    DCEL(const std::vector<Point>& points, const std::vector<std::pair<int, int>>& edges) {
        build(points, edges);
    }

    int target(int e) const {
        check_index(e, halfedges.size());
        check_index(halfedges[e].twin, halfedges.size());
        return halfedges[halfedges[e].twin].origin;
    }
    int left_face(int e) const {
        check_index(e, halfedges.size());
        return halfedges[e].face;
    }
    int right_face(int e) const {
        check_index(e, halfedges.size());
        check_index(halfedges[e].twin, halfedges.size());
        return halfedges[halfedges[e].twin].face;
    }

    std::vector<int> vertex_halfedges(int v) const {
        check_index(v, vertices.size());
        std::vector<int> result;
        int start = vertices[v].halfedge;
        if (start == -1) return result;
        int e = start;
        do {
            check_index(e, halfedges.size());
            if (halfedges[e].origin != v || result.size() >= halfedges.size())
                throw std::logic_error("Invalid DCEL vertex circulation.");
            result.push_back(e);
            check_index(halfedges[e].twin, halfedges.size());
            e = halfedges[halfedges[e].twin].next;
        } while (e != start);
        return result;
    }
    std::vector<int> boundary_halfedges(int start) const {
        std::vector<int> result;
        if (start == -1) return result;
        int e = start;
        do {
            check_index(e, halfedges.size());
            if (result.size() >= halfedges.size())
                throw std::logic_error("Invalid DCEL boundary circulation.");
            result.push_back(e);
            e = halfedges[e].next;
        } while (e != start);
        return result;
    }
    std::vector<int> face_boundaries(int f) const {
        check_index(f, faces.size());
        std::vector<int> result;
        if (faces[f].halfedge != -1) result.push_back(faces[f].halfedge);
        result.insert(result.end(), faces[f].inner_halfedges.begin(), faces[f].inner_halfedges.end());
        return result;
    }
    std::vector<int> face_halfedges(int f) const {
        std::vector<int> result;
        for (int start : face_boundaries(f)) {
            auto boundary = boundary_halfedges(start);
            result.insert(result.end(), boundary.begin(), boundary.end());
        }
        return result;
    }
    std::vector<int> face_vertices(int f) const {
        std::vector<int> result;
        for (int e : face_halfedges(f)) result.push_back(halfedges[e].origin);
        return result;
    }
    double face_signed_area(int f) const {
        long double area = 0;
        for (int start : face_boundaries(f)) area += boundary_area(start);
        return static_cast<double>(area);
    }

    void build(const std::vector<Point>& points, const std::vector<std::pair<int, int>>& edges) {
        if (points.size() > static_cast<size_t>(std::numeric_limits<int>::max()) ||
            edges.size() > static_cast<size_t>(std::numeric_limits<int>::max() / 2))
            throw std::length_error("DCEL uses int IDs.");
        if (!valid_embedding(points, edges))
            throw std::invalid_argument("DCEL requires finite distinct points and nonzero, nonoverlapping edges meeting only at shared endpoints.");
        DCEL result;
        result.build_embedding(points, edges);
        if (!result.is_valid())
            throw std::invalid_argument("Inconsistent planar embedding; degenerate or numerically unstable input.");
        *this = std::move(result);
    }

    bool is_valid() const {
        if (vertices.size() > static_cast<size_t>(std::numeric_limits<int>::max()) ||
            halfedges.size() > static_cast<size_t>(std::numeric_limits<int>::max()) ||
            faces.size() > static_cast<size_t>(std::numeric_limits<int>::max())) return false;
        int n = static_cast<int>(vertices.size());
        int m = static_cast<int>(halfedges.size());
        int f = static_cast<int>(faces.size());
        auto valid_vertex = [n](int v) { return 0 <= v && v < n; };
        auto valid_edge = [m](int e) { return 0 <= e && e < m; };
        auto valid_face = [f](int i) { return 0 <= i && i < f; };
        if (!valid_face(outer_face) || m % 2 != 0 || faces[outer_face].halfedge != -1) return false;

        std::vector<int> degree(n, 0), parent(n);
        std::iota(parent.begin(), parent.end(), 0);
        auto root = [&](int v) {
            while (parent[v] != v) { parent[v] = parent[parent[v]]; v = parent[v]; }
            return v;
        };
        for (const auto& h : halfedges) {
            if (!valid_vertex(h.origin) || !valid_edge(h.twin) || !valid_edge(h.next) ||
                !valid_edge(h.prev) || !valid_face(h.face)) return false;
            ++degree[h.origin];
        }
        for (int e = 0; e < m; ++e) {
            const auto& h = halfedges[e];
            if (h.twin == e || halfedges[h.twin].twin != e) return false;
            if (halfedges[h.twin].origin == h.origin) return false;
            if (halfedges[h.next].prev != e || halfedges[h.prev].next != e) return false;
            if (halfedges[h.next].origin != halfedges[h.twin].origin || halfedges[h.next].face != h.face) return false;
            parent[root(h.origin)] = root(halfedges[h.twin].origin);
        }

        std::vector<bool> seen_at_vertex(m, false), seen_on_face(m, false), seen_isolated(n, false);
        int components = 0;
        for (int v = 0; v < n; ++v) {
            components += root(v) == v;
            int start = vertices[v].halfedge;
            if (degree[v] == 0) {
                if (start != -1 || !valid_face(vertices[v].face)) return false;
                continue;
            }
            if (!valid_edge(start) || vertices[v].face != -1) return false;
            int e = start, count = 0;
            do {
                if (!valid_edge(e) || halfedges[e].origin != v || seen_at_vertex[e]) return false;
                seen_at_vertex[e] = true;
                e = halfedges[halfedges[e].twin].next;
                if (++count > degree[v]) return false;
            } while (e != start);
            if (count != degree[v]) return false;
        }

        for (int i = 0; i < f; ++i) {
            if (i != outer_face && !valid_edge(faces[i].halfedge)) return false;
            auto visit_boundary = [&](int start) {
                if (!valid_edge(start)) return false;
                int e = start;
                do {
                    if (!valid_edge(e) || halfedges[e].face != i || seen_on_face[e]) return false;
                    seen_on_face[e] = true;
                    e = halfedges[e].next;
                } while (e != start);
                return true;
            };
            if (faces[i].halfedge != -1 && !visit_boundary(faces[i].halfedge)) return false;
            for (int start : faces[i].inner_halfedges)
                if (!visit_boundary(start)) return false;
            for (int v : faces[i].isolated_vertices) {
                if (!valid_vertex(v) || degree[v] != 0 || vertices[v].face != i || seen_isolated[v]) return false;
                seen_isolated[v] = true;
            }
        }
        for (int e = 0; e < m; ++e)
            if (!seen_at_vertex[e] || !seen_on_face[e]) return false;
        for (int v = 0; v < n; ++v)
            if (degree[v] == 0 && !seen_isolated[v]) return false;
        return static_cast<long long>(n) - m / 2 + f == 1LL + components;
    }

    bool is_valid_geometry() const {
        if (!is_valid()) return false;
        std::vector<Point> points;
        std::vector<std::pair<int, int>> edges;
        std::vector<int> original;
        for (const auto& v : vertices) points.push_back(v.point);
        for (int e = 0; e < static_cast<int>(halfedges.size()); ++e)
            if (e < halfedges[e].twin) { edges.emplace_back(halfedges[e].origin, target(e)); original.push_back(e); }
        try {
            DCEL rebuilt(points, edges);
            if (rebuilt.faces.size() != faces.size()) return false;
            std::vector<int> edge_map(halfedges.size()), face_map(faces.size(), -1), reverse_map(faces.size(), -1);
            for (int i = 0; i < static_cast<int>(original.size()); ++i) {
                edge_map[original[i]] = 2 * i;
                edge_map[halfedges[original[i]].twin] = 2 * i + 1;
            }
            auto match_face = [&](int a, int b) {
                if (face_map[a] != -1) return face_map[a] == b;
                if (reverse_map[b] != -1) return false;
                face_map[a] = b; reverse_map[b] = a;
                return true;
            };
            if (!match_face(outer_face, rebuilt.outer_face)) return false;
            for (int e = 0; e < static_cast<int>(halfedges.size()); ++e) {
                const auto& a = halfedges[e];
                const auto& b = rebuilt.halfedges[edge_map[e]];
                if (edge_map[a.next] != b.next || edge_map[a.prev] != b.prev || !match_face(a.face, b.face)) return false;
            }
            for (int v = 0; v < static_cast<int>(vertices.size()); ++v)
                if (vertices[v].halfedge == -1 && !match_face(vertices[v].face, rebuilt.vertices[v].face)) return false;
            for (int i = 0; i < static_cast<int>(faces.size()); ++i) if (i != outer_face) {
                auto boundary = rebuilt.boundary_halfedges(rebuilt.faces[face_map[i]].halfedge);
                if (std::find(boundary.begin(), boundary.end(), edge_map[faces[i].halfedge]) == boundary.end()) return false;
            }
            return true;
        } catch (const std::invalid_argument&) { return false; }
    }

    int find_halfedge(int u, int v) const {
        check_index(v, vertices.size());
        for (int e : vertex_halfedges(u)) if (target(e) == v) return e;
        return -1;
    }
    int find_halfedge_on_face(int f, int v) const {
        check_index(v, vertices.size());
        for (int e : face_halfedges(f)) if (halfedges[e].origin == v) return e;
        return -1;
    }

    int split_face(int f, int u, int v) {
        check_index(f, faces.size());
        check_index(u, vertices.size());
        check_index(v, vertices.size());
        if (f == outer_face || u == v || find_halfedge(u, v) != -1)
            throw std::invalid_argument("A face split needs distinct nonadjacent vertices in a bounded face.");
        int eu = corner_toward(u, vertices[v].point);
        int ev = corner_toward(v, vertices[u].point);
        if (eu == -1 || ev == -1 || halfedges[eu].face != f || halfedges[ev].face != f)
            throw std::invalid_argument("The diagonal does not enter the requested face at both endpoints.");
        return split_face_at_corners(eu, ev);
    }

    int split_face_at_corners(int eu, int ev) {
        check_index(eu, halfedges.size());
        check_index(ev, halfedges.size());
        int f = halfedges[eu].face;
        check_index(f, faces.size());
        int u = halfedges[eu].origin, v = halfedges[ev].origin;
        if (f == outer_face || halfedges[ev].face != f || u == v || find_halfedge(u, v) != -1)
            throw std::invalid_argument("Invalid face-split endpoints.");
        auto boundary = boundary_halfedges(faces[f].halfedge);
        if (std::find(boundary.begin(), boundary.end(), eu) == boundary.end() ||
            std::find(boundary.begin(), boundary.end(), ev) == boundary.end())
            throw std::invalid_argument("Face splitting currently requires two corners on the outer boundary.");
        if (corner_toward(u, vertices[v].point) != eu || corner_toward(v, vertices[u].point) != ev || !clear_diagonal(u, v))
            throw std::invalid_argument("The diagonal overlaps, crosses, or touches existing geometry, or uses the wrong corner.");
        check_capacity(0, 2, 1);

        int pu = halfedges[eu].prev, pv = halfedges[ev].prev;
        int e = static_cast<int>(halfedges.size()), t = e + 1;
        int nf = static_cast<int>(faces.size());
        reserve_more(halfedges, 2);
        reserve_more(faces, 1);
        Face new_face;
        new_face.halfedge = e;
        new_face.inner_halfedges.reserve(faces[f].inner_halfedges.size());
        new_face.isolated_vertices.reserve(faces[f].isolated_vertices.size());
        halfedges.push_back({ u, t, ev, pu, nf });
        halfedges.push_back({ v, e, eu, pv, f });
        halfedges[pu].next = e; halfedges[ev].prev = e;
        halfedges[pv].next = t; halfedges[eu].prev = t;
        faces[f].halfedge = eu;
        faces.push_back(std::move(new_face));
        int cur = e;
        do { halfedges[cur].face = nf; cur = halfedges[cur].next; } while (cur != e);

        auto& holes = faces[f].inner_halfedges;
        for (size_t i = 0; i < holes.size();) {
            int start = holes[i];
            if (boundary_contains(e, vertices[halfedges[start].origin].point)) {
                faces[nf].inner_halfedges.push_back(start);
                cur = start;
                do { halfedges[cur].face = nf; cur = halfedges[cur].next; } while (cur != start);
                holes.erase(holes.begin() + i);
            } else ++i;
        }
        auto& isolated = faces[f].isolated_vertices;
        for (size_t i = 0; i < isolated.size();) {
            int w = isolated[i];
            if (boundary_contains(e, vertices[w].point)) {
                faces[nf].isolated_vertices.push_back(w);
                vertices[w].face = nf;
                isolated.erase(isolated.begin() + i);
            } else ++i;
        }
        assert(is_valid());
        return e;
    }

    int split_edge(int e, const Point& p) {
        check_index(e, halfedges.size());
        int t = halfedges[e].twin;
        check_index(t, halfedges.size());
        int u = halfedges[e].origin, v = halfedges[t].origin;
        Point point = p;
        if (!finite(point) || point == vertices[u].point || point == vertices[v].point ||
            !point_on_segment(point, vertices[u].point, vertices[v].point))
            throw std::invalid_argument("The split point must lie strictly inside the edge.");
        for (const auto& vertex : vertices)
            if (vertex.point == point) throw std::invalid_argument("The split point already exists.");
        check_capacity(1, 2, 0);
        int ne = halfedges[e].next, nt = halfedges[t].next;
        int lf = halfedges[e].face, rf = halfedges[t].face;
        int w = static_cast<int>(vertices.size());
        int e2 = static_cast<int>(halfedges.size()), t2 = e2 + 1;
        reserve_more(vertices, 1);
        reserve_more(halfedges, 2);
        vertices.push_back({ point, e2, -1 });
        halfedges.push_back({ w, t, ne, e, lf });
        halfedges.push_back({ w, e, nt, t, rf });
        halfedges[e].twin = t2; halfedges[t].twin = e2;
        halfedges[e].next = e2; halfedges[ne].prev = e2;
        halfedges[t].next = t2; halfedges[nt].prev = t2;
        assert(is_valid());
        return w;
    }

private:
    template <typename T>
    static void reserve_more(std::vector<T>& values, size_t count) {
        size_t required = values.size() + count;
        if (required <= values.capacity()) return;
        size_t limit = static_cast<size_t>(std::numeric_limits<int>::max());
        size_t grown = values.capacity() > limit / 2 ? limit : 2 * values.capacity();
        values.reserve(std::max(required, grown));
    }
    static void check_index(int index, size_t size) {
        if (index < 0 || static_cast<size_t>(index) >= size) throw std::out_of_range("Invalid DCEL ID.");
    }
    void check_capacity(size_t v, size_t e, size_t f) const {
        auto limit = static_cast<size_t>(std::numeric_limits<int>::max());
        if (vertices.size() > limit - v || halfedges.size() > limit - e || faces.size() > limit - f)
            throw std::length_error("DCEL uses int IDs.");
    }
    static bool finite(const Point& p) { return std::isfinite(p.x) && std::isfinite(p.y); }
    static long double turn(const Point& a, const Point& b, const Point& c) {
        return (static_cast<long double>(b.x) - a.x) * (static_cast<long double>(c.y) - a.y) -
            (static_cast<long double>(b.y) - a.y) * (static_cast<long double>(c.x) - a.x);
    }
    static bool point_on_segment(const Point& p, const Point& a, const Point& b) {
        return turn(a, b, p) == 0 && std::min(a.x, b.x) <= p.x && p.x <= std::max(a.x, b.x) &&
            std::min(a.y, b.y) <= p.y && p.y <= std::max(a.y, b.y);
    }
    static bool angle_less(const Point& a, const Point& b) {
        auto upper = [](const Point& d) { return d.y > 0 || (d.y == 0 && d.x >= 0); };
        if (upper(a) != upper(b)) return upper(a) > upper(b);
        return turn({}, a, b) > 0;
    }
    static bool segments_conflict(const Point& a, const Point& b, const Point& c, const Point& d) {
        if (a == c || a == d || b == c || b == d) {
            Point s = (a == c || a == d) ? a : b;
            Point x = s == a ? b : a, y = s == c ? d : c;
            return turn(s, x, y) == 0 &&
                (static_cast<long double>(x.x) - s.x) * (static_cast<long double>(y.x) - s.x) +
                (static_cast<long double>(x.y) - s.y) * (static_cast<long double>(y.y) - s.y) > 0;
        }
        auto opposite = [](long double x, long double y) { return (x < 0 && y > 0) || (x > 0 && y < 0); };
        if (opposite(turn(a, b, c), turn(a, b, d)) && opposite(turn(c, d, a), turn(c, d, b))) return true;
        return point_on_segment(c, a, b) || point_on_segment(d, a, b) ||
            point_on_segment(a, c, d) || point_on_segment(b, c, d);
    }
    static bool valid_embedding(const std::vector<Point>& points, const std::vector<std::pair<int, int>>& edges) {
        std::set<std::pair<double, double>> positions;
        for (const auto& p : points)
            if (!finite(p) || !positions.emplace(p.x, p.y).second) return false;
        for (size_t i = 0; i < edges.size(); ++i) {
            auto [u, v] = edges[i];
            if (u < 0 || v < 0 || static_cast<size_t>(u) >= points.size() || static_cast<size_t>(v) >= points.size() || u == v) return false;
            if (!finite(points[v] - points[u])) return false;
            for (size_t w = 0; w < points.size(); ++w)
                if (!std::isfinite(turn(points[u], points[v], points[w])) ||
                    (w != static_cast<size_t>(u) && w != static_cast<size_t>(v) && point_on_segment(points[w], points[u], points[v]))) return false;
            for (size_t j = 0; j < i; ++j) {
                auto [a, b] = edges[j];
                if (segments_conflict(points[u], points[v], points[a], points[b])) return false;
            }
        }
        return true;
    }
    bool clear_diagonal(int u, int v) const {
        const Point& a = vertices[u].point;
        const Point& b = vertices[v].point;
        for (int w = 0; w < static_cast<int>(vertices.size()); ++w)
            if (w != u && w != v && point_on_segment(vertices[w].point, a, b)) return false;
        for (int e = 0; e < static_cast<int>(halfedges.size()); ++e) if (e < halfedges[e].twin)
            if (segments_conflict(a, b, vertices[halfedges[e].origin].point, vertices[target(e)].point)) return false;
        return true;
    }
    int corner_toward(int v, const Point& p) const {
        auto outgoing = vertex_halfedges(v);
        if (outgoing.empty()) return -1;
        const Point origin = vertices[v].point;
        std::sort(outgoing.begin(), outgoing.end(), [&](int a, int b) {
            return angle_less(vertices[target(a)].point - origin, vertices[target(b)].point - origin);
        });
        Point direction = p - origin;
        auto it = std::lower_bound(outgoing.begin(), outgoing.end(), direction, [&](int e, const Point& d) {
            return angle_less(vertices[target(e)].point - origin, d);
        });
        if (it == outgoing.begin()) it = outgoing.end();
        return *--it;
    }
    long double boundary_area(int start) const {
        if (start == -1) return 0;
        Point base = vertices[halfedges[start].origin].point;
        long double area = 0;
        for (int e : boundary_halfedges(start))
            area += turn(base, vertices[halfedges[e].origin].point, vertices[target(e)].point);
        return area / 2;
    }
    bool boundary_contains(int start, const Point& p) const {
        int winding = 0;
        for (int e : boundary_halfedges(start)) {
            const Point& a = vertices[halfedges[e].origin].point;
            const Point& b = vertices[target(e)].point;
            if (point_on_segment(p, a, b)) return true;
            if (a.y <= p.y && p.y < b.y && turn(a, b, p) > 0) ++winding;
            if (b.y <= p.y && p.y < a.y && turn(a, b, p) < 0) --winding;
        }
        return winding != 0;
    }

    void build_embedding(const std::vector<Point>& points, const std::vector<std::pair<int, int>>& edges) {
        int n = static_cast<int>(points.size());
        std::vector<std::vector<int>> outgoing(n);
        std::vector<int> parent(n);
        std::iota(parent.begin(), parent.end(), 0);
        auto root = [&](int v) {
            while (parent[v] != v) { parent[v] = parent[parent[v]]; v = parent[v]; }
            return v;
        };
        for (const auto& p : points) vertices.push_back({ p, -1, -1 });
        halfedges.reserve(2 * edges.size());
        for (auto [u, v] : edges) {
            int e = static_cast<int>(halfedges.size());
            halfedges.push_back({ u, e + 1, -1, -1, -1 });
            halfedges.push_back({ v, e, -1, -1, -1 });
            outgoing[u].push_back(e); outgoing[v].push_back(e + 1);
            vertices[u].halfedge = e; vertices[v].halfedge = e + 1;
            parent[root(u)] = root(v);
        }
        std::vector<int> position(halfedges.size());
        for (int v = 0; v < n; ++v) {
            std::sort(outgoing[v].begin(), outgoing[v].end(), [&](int a, int b) {
                return angle_less(points[target(a)] - points[v], points[target(b)] - points[v]);
            });
            for (int i = 0; i < static_cast<int>(outgoing[v].size()); ++i) position[outgoing[v][i]] = i;
        }
        for (int e = 0; e < static_cast<int>(halfedges.size()); ++e) {
            const auto& out = outgoing[target(e)];
            int pos = position[halfedges[e].twin];
            halfedges[e].next = out[(pos + out.size() - 1) % out.size()];
        }
        for (int e = 0; e < static_cast<int>(halfedges.size()); ++e) halfedges[halfedges[e].next].prev = e;

        std::vector<int> starts, cycle(halfedges.size(), -1);
        for (int e = 0; e < static_cast<int>(halfedges.size()); ++e) if (cycle[e] == -1) {
            int id = static_cast<int>(starts.size());
            starts.push_back(e);
            int cur = e;
            do { cycle[cur] = id; cur = halfedges[cur].next; } while (cur != e);
        }
        std::vector<int> lowest(n, -1);
        for (int v = 0; v < n; ++v) if (!outgoing[v].empty()) {
            int r = root(v), a = lowest[r];
            if (a == -1 || points[v].y < points[a].y || (points[v].y == points[a].y && points[v].x < points[a].x)) lowest[r] = v;
        }
        std::vector<bool> exterior(starts.size(), false);
        for (int v : lowest) if (v != -1) exterior[cycle[outgoing[v].back()]] = true;
        std::vector<int> owner(starts.size(), outer_face);
        for (int i = 0; i < static_cast<int>(starts.size()); ++i) if (!exterior[i]) {
            long double area = boundary_area(starts[i]);
            if (!(area > 0) || !std::isfinite(area)) throw std::invalid_argument("Degenerate or numerically unstable DCEL boundary.");
            owner[i] = static_cast<int>(faces.size());
            faces.push_back({ starts[i], {}, {} });
        }
        auto containing_face = [&](const Point& p, int excluded_component) {
            int result = outer_face;
            long double smallest = std::numeric_limits<long double>::infinity();
            for (int f = 1; f < static_cast<int>(faces.size()); ++f) {
                int start = faces[f].halfedge;
                if (root(halfedges[start].origin) == excluded_component) continue;
                long double area = boundary_area(start);
                if (area < smallest && boundary_contains(start, p)) { result = f; smallest = area; }
            }
            return result;
        };
        for (int i = 0; i < static_cast<int>(starts.size()); ++i) if (exterior[i]) {
            int start = starts[i], v = halfedges[start].origin;
            owner[i] = containing_face(points[v], root(v));
            faces[owner[i]].inner_halfedges.push_back(start);
        }
        for (int e = 0; e < static_cast<int>(halfedges.size()); ++e) halfedges[e].face = owner[cycle[e]];
        for (int v = 0; v < n; ++v) if (outgoing[v].empty()) {
            int f = containing_face(points[v], -1);
            vertices[v].face = f;
            faces[f].isolated_vertices.push_back(v);
        }
    }
};
