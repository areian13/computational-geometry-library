# 2D Geometry Library

C++로 구현한 2차원 계산기하학 라이브러리입니다.

계산기하학 알고리즘을 직접 구현하고 공부하기 위한 목적으로 개발하고 있으며, 이후 연구 및 실험 코드에서도 재사용할 수 있는 형태를 목표로 하고 있습니다.

현재는 `double` 기반의 기하 연산을 사용합니다.

---

## Structure

```text
geometry/
├─ point.hpp
├─ point_location.hpp
├─ segment.hpp
├─ circle.hpp
├─ polygon.hpp
└─ bitangent.hpp

algorithm/
├─ convex_hull.hpp
├─ closest_point_pair.hpp
└─ farthest_point_pair.hpp

structure/
```

각 디렉터리는 다음 기준으로 구분합니다.

- `geometry/`
  - 기본 기하 객체
  - geometric predicate
  - 객체 사이의 기본적인 관계 및 query
- `algorithm/`
  - 입력으로부터 새로운 결과를 계산하는 독립적인 알고리즘
- `structure/`
  - 계산 이후에도 상태를 유지하며 query 또는 traversal에 사용되는 자료구조

---

## Geometry

### Point

`Point`는 점과 2차원 벡터를 함께 표현합니다.

지원 연산:

- 사칙연산
- `dot`
- `cross`
- `norm`
- `dist`
- `orientation`
- `ccw`
- exact equality
- approximate equality

```cpp
Point a = {0, 0};
Point b = {3, 4};

double d = dist(a, b);
double cross = a.cross(b);
```

---

### Segment

선분과 관련된 기본 연산을 제공합니다.

- 길이
- 점의 선분 위 포함 여부
- 선분 교차 여부
- 선분 교점

```cpp
Segment s1 = {{0, 0}, {2, 2}};
Segment s2 = {{0, 2}, {2, 0}};

if (intersects(s1, s2)) {
    auto result = intersection(s1, s2);
}
```

---

### Circle

원을 중심점과 반지름으로 표현합니다.

```cpp
Circle circle = {{0, 0}, 1.0};

PointLocation location = locate(circle, {0.5, 0.0});
```

---

### Polygon

다음 polygon type을 제공합니다.

```text
Polygon
└─ SimplePolygon
   └─ ConvexPolygon
```

- `Polygon`
  - 일반적인 polygonal chain
- `SimplePolygon`
  - self-intersection이 없는 polygon
- `ConvexPolygon`
  - convex simple polygon

Polygon의 정점 배열에는 첫 번째 정점을 마지막에 다시 저장하지 않습니다.

---

## Point Location

점이 기하 객체에 대해 어디에 위치하는지를 `PointLocation`으로 표현합니다.

```cpp
enum class PointLocation {
    INSIDE = 0,
    ON_BOUNDARY = 1,
    ON_VERTEX = 2,
    OUTSIDE = 3
};
```

객체에 따라 일부 상태는 발생하지 않을 수 있습니다.

예를 들어 `Circle`에는 vertex가 없으므로 `ON_VERTEX`를 반환하지 않습니다.

```cpp
PointLocation locate(const Circle& circle, const Point& p);
PointLocation locate(const SimplePolygon& polygon, const Point& p);
PointLocation locate(const ConvexPolygon& polygon, const Point& p);
```

현재 시간복잡도는 다음과 같습니다.

| Query | Complexity |
|---|---:|
| `locate(Circle, Point)` | `O(1)` |
| `locate(SimplePolygon, Point)` | `O(n)` |
| `locate(ConvexPolygon, Point)` | `O(log n)` |

---

## Bitangent

두 convex 기하 객체 사이의 bitangent를 표현하기 위한 타입을 제공합니다.

```cpp
enum class BitangentType {
    LL,
    LR,
    RL,
    RR
};

struct Bitangent {
    Segment segment;
    BitangentType type;
};
```

Bitangent는 directed segment로 표현하며, segment의 방향을 기준으로 두 객체가 각각 어느 쪽에 위치하는지에 따라 `LL`, `LR`, `RL`, `RR`로 구분합니다.

객체별로 동일한 이름의 free function을 구현하는 방식을 사용합니다.

```cpp
auto bits = bitangents(a, b);

auto ll = bit_LL(a, b);
auto lr = bit_LR(a, b);
```

---

## Algorithms

### Convex Hull

```cpp
ConvexPolygon hull = convex_hull(points);
```

원본 입력에서 hull vertex의 index도 필요한 경우:

```cpp
auto [hull, indices] = indexed_convex_hull(points);
```

`indices[i]`는 다음 관계를 만족합니다.

```cpp
hull[i] == points[indices[i]]
```

시간복잡도:

```text
O(n log n)
```

---

### Closest Point Pair

가장 가까운 두 입력 점의 **원본 index**를 반환합니다.

```cpp
auto [i, j] = closest_point_pair(points);
```

입력 점이 2개보다 적으면:

```cpp
{-1, -1}
```

을 반환합니다.

반환되는 index는 다음을 만족합니다.

```cpp
i < j
```

시간복잡도:

```text
O(n log n)
```

---

### Farthest Point Pair

가장 먼 두 입력 점의 **원본 index**를 반환합니다.

```cpp
auto [i, j] = farthest_point_pair(points);
```

Convex Hull을 구성한 뒤 Rotating Calipers를 사용합니다.

입력 점이 2개보다 적으면:

```cpp
{-1, -1}
```

을 반환합니다.

모든 입력점이 동일한 좌표인 경우에도 서로 다른 두 원본 index를 반환합니다.

시간복잡도:

```text
O(n log n)
```

---

## Complexity

| Function | Complexity |
|---|---:|
| `convex_hull` | `O(n log n)` |
| `indexed_convex_hull` | `O(n log n)` |
| `closest_point_pair` | `O(n log n)` |
| `farthest_point_pair` | `O(n log n)` |
| `locate(Circle, Point)` | `O(1)` |
| `locate(SimplePolygon, Point)` | `O(n)` |
| `locate(ConvexPolygon, Point)` | `O(log n)` |
| `on_segment` | `O(1)` |
| `intersects(Segment, Segment)` | `O(1)` |
| `intersection(Segment, Segment)` | `O(1)` |

---

## Numeric Conventions

현재 모든 좌표 계산은 `double`을 사용합니다.

```cpp
inline constexpr double EPS = 1e-9;
```

### Exact comparison

저장된 값 자체의 동일성을 검사할 때는 exact comparison을 사용합니다.

예:

```cpp
Point a, b;

if (a == b) {
    ...
}
```

`Point::operator==`는 두 좌표를 exact하게 비교합니다.

### Approximate comparison

기하 연산의 결과를 판정할 때는 필요한 경우 `EPS` 기반 비교를 사용합니다.

```cpp
is_zero(x);
approx_eq(a, b);
sign(x);
```

예를 들어 Circle의 boundary 판정은 계산된 거리와 반지름을 approximate하게 비교합니다.

---

## Conventions

현재 라이브러리에서는 다음 convention을 사용합니다.

- 좌표 타입은 `double`입니다.
- `Point::operator==`는 exact comparison입니다.
- floating-point geometric predicate는 필요한 경우 `EPS`를 사용합니다.
- Polygon 정점 배열의 마지막에 첫 번째 정점을 반복해서 저장하지 않습니다.
- `ConvexPolygon`은 CCW 순서를 기준으로 연산합니다.
- Convex Hull은 불필요한 collinear intermediate vertex를 제거합니다.
- point pair 알고리즘은 원본 입력의 index를 반환합니다.
- point pair의 두 index는 서로 다른 입력 원소를 의미합니다.
- geometric relation은 가능한 경우 free function으로 표현합니다.

예:

```cpp
dist(a, b);
intersects(a, b);
locate(object, p);
bitangents(a, b);
```

---

## Degenerate Cases

일부 degenerate case를 지원하지만 모든 연산이 완전히 robust한 것은 아닙니다.

현재 주요 정책은 다음과 같습니다.

| Case | Current behavior |
|---|---|
| Duplicate points | Point pair 알고리즘에서 지원 |
| All input points identical | Farthest pair는 서로 다른 두 원본 index를 반환 |
| Collinear points in Convex Hull | 양 끝점만 유지 |
| Point on polygon vertex | `ON_VERTEX` |
| Point on polygon edge | `ON_BOUNDARY` |
| Point on circle | `ON_BOUNDARY` |
| Segment endpoint intersection | 지원 |
| Collinear segment overlap | 별도의 overlap intersection으로 처리 |

---

## Robustness and Known Limitations

이 라이브러리는 현재 학습 및 연구 실험을 위한 구현으로, exact geometric computation을 제공하지 않습니다.

| Component | Current implementation | Limitation / Future work |
|---|---|---|
| Numeric predicates | Fixed `EPS = 1e-9` | Coordinate scale에 따라 tolerance의 의미가 달라질 수 있음 |
| `orientation`, `ccw` | `double` arithmetic | Near-degenerate input에서 floating-point error 가능 |
| Point equality | Exact `double` comparison | 계산을 거친 점의 동일성 판정에는 적합하지 않을 수 있음 |
| Segment intersection | Floating-point predicates | Exact / adaptive predicate 미지원 |
| Circle predicates | `double` + `EPS` | Near-tangent configuration에서 robustness 개선 필요 |
| Simple Polygon point location | Finite long segment를 이용한 ray casting | Proper `Ray` 구현 후 교체 예정 |
| Convex Polygon point location | Binary search | CCW convex polygon 및 정점 표현에 대한 가정 존재 |
| Bitangent | Floating-point construction | Degenerate tangent configuration의 추가 처리 필요 |

### Simple Polygon Point Location

현재 Simple Polygon의 point location에서는 무한 ray를 직접 표현하지 않고 충분히 긴 유한 선분을 사용합니다.

```cpp
Segment ray = {p, {p.x + 1. / EPS, p.y + EPS}};
```

이는 기존 구현과의 검증을 우선하기 위한 임시 구현입니다.

향후 `Ray` 타입 및 Ray-Segment intersection을 구현한 뒤 제거할 예정입니다.

---

## General Position

향후 Visibility 관련 알고리즘에서는 별도의 general position assumption을 사용할 수 있습니다.

예를 들어 Visibility Complex 구현에서는 다음과 같은 조건을 가정할 수 있습니다.

- geometric objects are pairwise disjoint
- degenerate bitangents do not occur
- no three objects share the same supporting tangent
- additional degeneracies may be excluded when required by the algorithm

이러한 가정은 일반적인 geometry primitive의 제약과는 분리해서 관리할 예정입니다.

---

## Roadmap

### Geometry

- [ ] `Line`
- [ ] `Ray`
- [ ] `HalfPlane`
- [ ] Ray-Segment intersection
- [ ] Line-related intersection / projection operations

### Robustness

- [ ] Remove finite-ray workaround from Simple Polygon point location
- [ ] Improve scale handling of `EPS`
- [ ] Robust / adaptive geometric predicates
- [ ] Additional degeneracy handling

### Algorithms

- [ ] All closest point pairs
- [ ] All farthest point pairs
- [ ] Additional polygon algorithms
- [ ] Tangent / support queries for convex objects

### Structures

- [ ] DCEL
- [ ] Voronoi Diagram
- [ ] Visibility Complex

---

## Goal

최종적으로는 단순한 PS용 코드 모음이 아니라,

- 계산기하 알고리즘을 직접 구현하고 검증할 수 있고
- 새로운 알고리즘을 실험할 때 재사용할 수 있으며
- Visibility Complex와 같은 보다 복잡한 기하 자료구조의 기반으로 사용할 수 있는

2D Computational Geometry Library를 만드는 것을 목표로 합니다.