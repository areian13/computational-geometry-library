#pragma once

#include <cassert>

#include <vector>
#include <algorithm>

#include "../geometry/point.hpp"

struct DCEL {
    struct Vertex {
        Point point;
        int halfedge = -1;
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
    };

    std::vector<Vertex> vertices;
    std::vector<HalfEdge> halfedges;
    std::vector<Face> faces;

    int outer_face = -1;

    DCEL() = default;
    DCEL(const std::vector<Point>& points, const std::vector<std::pair<int, int>>& edges) {
        build(points, edges);
    }

    int target(int e) const {
        return halfedges[halfedges[e].twin].origin;
    }
    int left_face(int e) const {
        return halfedges[e].face;
    }
    int right_face(int e) const {
        return halfedges[halfedges[e].twin].face;
    }

    std::vector<int> vertex_halfedges(int v) const {
        std::vector<int> result;

        int start = vertices[v].halfedge;
        if (start == -1) return result;

        int e = start;
        do {
            result.push_back(e);
            e = halfedges[halfedges[e].twin].next;
        } while (e != start);

        return result;
    }
    std::vector<int> face_halfedges(int f) const {
        std::vector<int> result;

        int start = faces[f].halfedge;
        if (start == -1) return result;

        int e = start;
        do {
            result.push_back(e);
            e = halfedges[e].next;
        } while (e != start);

        return result;
    }
    std::vector<int> face_vertices(int f) const {
        std::vector<int> result;

        for (int e : face_halfedges(f))
            result.push_back(halfedges[e].origin);

        return result;
    }

    double face_signed_area(int f) const {
        int start = faces[f].halfedge;
        if (start == -1) return 0.0;

        double area = 0.0;

        int e = start;
        do {
            const Point& a = vertices[halfedges[e].origin].point;
            const Point& b = vertices[target(e)].point;

            area += a.cross(b);
            e = halfedges[e].next;
        } while (e != start);

        return area / 2.0;
    }

    void build(const std::vector<Point>& points, const std::vector<std::pair<int, int>>& edges) {
        vertices.clear();
        halfedges.clear();
        faces.clear();
        outer_face = -1;

        int n = points.size();

        vertices.reserve(n);
        for (const Point& p : points)
            vertices.push_back({ p, -1 });

        std::vector<std::vector<int>> outgoing(n);

        for (auto [u, v] : edges) {
            int e = halfedges.size();

            halfedges.push_back({ u, e + 1, -1, -1, -1 });
            halfedges.push_back({ v, e, -1, -1, -1 });

            outgoing[u].push_back(e);
            outgoing[v].push_back(e + 1);

            if (vertices[u].halfedge == -1)
                vertices[u].halfedge = e;

            if (vertices[v].halfedge == -1)
                vertices[v].halfedge = e + 1;
        }

        if (halfedges.empty()) {
            faces.push_back({ -1 });
            outer_face = 0;
            return;
        }

        auto upper = [](const Point& d) {
            return d.y > 0 || (d.y == 0 && d.x >= 0);
        };

        for (int v = 0; v < n; v++) {
            std::sort(outgoing[v].begin(), outgoing[v].end(),
                [&](int e1, int e2) {
                Point d1 = vertices[target(e1)].point - vertices[v].point;
                Point d2 = vertices[target(e2)].point - vertices[v].point;

                bool h1 = upper(d1);
                bool h2 = upper(d2);

                if (h1 != h2) return h1 > h2;

                double cr = d1.cross(d2);
                if (cr != 0.0) return cr > 0;

                double n1 = d1.norm2();
                double n2 = d2.norm2();

                if (n1 != n2) return n1 < n2;
                return e1 < e2;
            }
            );
        }

        std::vector<int> position(halfedges.size());

        for (int v = 0; v < n; v++)
            for (int i = 0; i < outgoing[v].size(); i++)
                position[outgoing[v][i]] = i;

        for (int e = 0; e < halfedges.size(); e++) {
            int v = target(e);
            int twin = halfedges[e].twin;

            const auto& out = outgoing[v];
            int pos = position[twin];

            halfedges[e].next = out[(pos - 1 + out.size()) % out.size()];
        }

        for (int e = 0; e < halfedges.size(); e++)
            halfedges[halfedges[e].next].prev = e;

        for (int e = 0; e < halfedges.size(); e++) {
            if (halfedges[e].face != -1) continue;

            int f = faces.size();
            faces.push_back({ e });

            int cur = e;
            do {
                halfedges[cur].face = f;
                cur = halfedges[cur].next;
            } while (cur != e);
        }

        outer_face = 0;
        for (int f = 1; f < faces.size(); f++)
            if (face_signed_area(f) < face_signed_area(outer_face))
                outer_face = f;
    }
    bool is_valid() const {
        int n = vertices.size();
        int m = halfedges.size();
        int f = faces.size();

        if (outer_face < 0 || outer_face >= f)
            return false;

        auto valid_vertex = [&](int v) { return 0 <= v && v < n; };
        auto valid_halfedge = [&](int e) { return 0 <= e && e < m; };
        auto valid_face = [&](int x) { return 0 <= x && x < f; };

        for (int v = 0; v < n; v++) {
            int e = vertices[v].halfedge;

            if (e != -1) {
                if (!valid_halfedge(e)) return false;
                if (halfedges[e].origin != v) return false;
            }
        }

        for (int e = 0; e < m; e++) {
            const HalfEdge& h = halfedges[e];

            if (!valid_vertex(h.origin)) return false;
            if (!valid_halfedge(h.twin)) return false;
            if (!valid_halfedge(h.next)) return false;
            if (!valid_halfedge(h.prev)) return false;
            if (!valid_face(h.face)) return false;

            if (halfedges[h.twin].twin != e) return false;
            if (halfedges[h.next].prev != e) return false;
            if (halfedges[h.prev].next != e) return false;

            if (halfedges[h.next].origin != target(e)) return false;
            if (halfedges[h.next].face != h.face) return false;
        }

        if (m == 0)
            return f == 1 && faces[0].halfedge == -1;

        std::vector<bool> visited(m, false);

        for (int i = 0; i < f; i++) {
            int start = faces[i].halfedge;

            if (!valid_halfedge(start))
                return false;

            if (halfedges[start].face != i)
                return false;

            int e = start;
            int count = 0;

            do {
                if (!valid_halfedge(e)) return false;
                if (halfedges[e].face != i) return false;
                if (visited[e]) return false;

                visited[e] = true;
                e = halfedges[e].next;

                if (++count > m) return false;
            } while (e != start);
        }

        for (bool v : visited)
            if (!v) return false;

        return true;
    }

    int find_halfedge(int u, int v) const {
        int start = vertices[u].halfedge;
        if (start == -1) return -1;

        int e = start;
        do {
            if (target(e) == v) return e;
            e = halfedges[halfedges[e].twin].next;
        } while (e != start);

        return -1;
    }
    int find_halfedge_on_face(int f, int v) const {
        int start = faces[f].halfedge;
        if (start == -1) return -1;

        int e = start;
        do {
            if (halfedges[e].origin == v) return e;
            e = halfedges[e].next;
        } while (e != start);

        return -1;
    }
    
    int split_face(int f, int u, int v) {
        assert(f != outer_face);
        assert(u != v);

        int eu = find_halfedge_on_face(f, u);
        int ev = find_halfedge_on_face(f, v);

        assert(eu != -1);
        assert(ev != -1);

        // 이미 boundary edge로 연결되어 있으면 face를 split할 수 없음.
        assert(target(eu) != v);
        assert(target(ev) != u);

        int pu = halfedges[eu].prev;
        int pv = halfedges[ev].prev;

        int e = halfedges.size();
        int t = e + 1;
        int nf = faces.size();

        // e : u -> v
        // t : v -> u
        halfedges.push_back({
            u,      // origin
            t,      // twin
            ev,     // next
            pu,     // prev
            nf      // face
            });

        halfedges.push_back({
            v,
            e,
            eu,
            pv,
            f
            });

        // 새 face 쪽:
        //
        // ... -> pu -> e(u->v) -> ev -> ...
        halfedges[pu].next = e;
        halfedges[ev].prev = e;

        // 기존 face 쪽:
        //
        // ... -> pv -> t(v->u) -> eu -> ...
        halfedges[pv].next = t;
        halfedges[eu].prev = t;

        faces[f].halfedge = eu;
        faces.push_back({ e });

        // e가 속한 cycle 전체를 새 face로 변경.
        int cur = e;
        do {
            halfedges[cur].face = nf;
            cur = halfedges[cur].next;
        } while (cur != e);

        assert(this->is_valid());
        return e;
    }
    int split_edge(int e, const Point& p) {
        int t = halfedges[e].twin;

        int u = halfedges[e].origin;
        int v = halfedges[t].origin;

        int ne = halfedges[e].next;
        int nt = halfedges[t].next;

        int lf = halfedges[e].face;
        int rf = halfedges[t].face;

        int w = vertices.size();
        vertices.push_back({ p, -1 });

        int e2 = halfedges.size();
        int t2 = e2 + 1;

        // 기존:
        //
        // e : u -> v
        // t : v -> u
        //
        // 변경:
        //
        // e  : u -> w
        // e2 : w -> v
        //
        // t  : v -> w
        // t2 : w -> u

        halfedges.push_back({
            w,      // e2 : w -> v
            t,
            ne,
            e,
            lf
            });

        halfedges.push_back({
            w,      // t2 : w -> u
            e,
            nt,
            t,
            rf
            });

        // twin 관계 변경
        halfedges[e].twin = t2;
        halfedges[t].twin = e2;

        // left face:
        //
        // e -> e2 -> ne
        halfedges[e].next = e2;
        halfedges[ne].prev = e2;

        // right face:
        //
        // t -> t2 -> nt
        halfedges[t].next = t2;
        halfedges[nt].prev = t2;

        vertices[w].halfedge = e2;
        
        assert(this->is_valid());
        return w;
    }
};