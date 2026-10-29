# 2D Geometry Library

C++로 구현한 2차원 계산기하학 라이브러리입니다.

계산기하학 알고리즘을 직접 구현하고 공부하기 위한 목적으로 개발하고 있으며, 이후 연구 및 실험 코드에서도 재사용할 수 있는 형태를 목표로 하고 있습니다.

현재는 `double` 기반의 기하 연산을 사용합니다.

## 개발 환경 및 실행 방법 (Windows)

Visual Studio와 VS Code 모두 기존 `computational-geometry-library.sln` / `.vcxproj`를 사용합니다.
컴파일러는 MSVC v143이며, Debug / Release 및 x64 / Win32 전체에 C++23을 적용했습니다.
현재 Visual Studio 2022에서는 `stdcpp23`이 `/std:c++23preview`에 해당합니다.
C++23 지원 범위는 설치된 MSVC 버전에 따릅니다.
([Microsoft C++ 표준 옵션 문서](https://learn.microsoft.com/en-us/cpp/build/reference/std-specify-language-standard-version?view=msvc-170))

### 필요한 설치

- Visual Studio 2022 **17.13 이상** 또는 동등한 Build Tools
- Visual Studio Installer의 **C++를 사용한 데스크톱 개발** 워크로드: MSVC v143, Windows SDK 포함
- VS Code의 Microsoft **C/C++** 확장 (`ms-vscode.cpptools`)

기존 Visual Studio 설치를 그대로 사용하므로 MinGW, GCC, CMake를 추가로 설치할 필요는 없습니다.

### VS Code

이 저장소의 **폴더 전체**를 VS Code에서 열고, 실행 및 디버그에서 `Debug x64 (MSVC)`를 선택합니다.

| 작업 | Windows 단축키 | 동작 |
|---|---|---|
| 빌드 후 일반 실행 | `Ctrl+F5` | 중단점 없이 실행 |
| 빌드 후 디버깅 실행 | `F5` | 중단점에서 정지, 변수 확인 가능 |
| 빌드만 하기 | `Ctrl+Shift+B` | Debug x64 빌드 |
| 중단점 설정/해제 | `F9` | 현재 줄의 중단점 변경 |
| 디버깅 중 다음 줄 | `F10` | 함수 내부로 들어가지 않고 진행 |
| 디버깅 중 함수 내부 진입 | `F11` | 호출한 함수 내부로 진행 |
| 디버깅 종료 | `Shift+F5` | 실행 중인 디버깅 세션 종료 |

입력과 출력은 하단 **터미널**에서 처리합니다. `main.cpp`의 `main()`은 현재 비어 있으므로,
출력 없이 즉시 종료되는 것이 정상입니다. 실행 결과를 보고 싶다면 `main()`에 실험 코드를 작성합니다.
실행할 때는 파일별 `Run Code`(Code Runner) 버튼 대신 위 단축키 또는 **실행 및 디버그**를 사용합니다.

Release 실행은 실행 및 디버그에서 `Release x64 (MSVC)`를 선택한 뒤 `Ctrl+F5`를 누릅니다.
Release 빌드, 재빌드, 정리는 **터미널 → 작업 실행**에서 각각 `Build: Release x64`,
`Rebuild: Debug x64`, `Clean: Debug x64`를 선택합니다.

빌드 스크립트가 Visual Studio 설치와 Windows SDK를 찾아 자동완성 설정
`.vscode/c_cpp_properties.json`을 생성합니다. 새로 clone한 PC에서는 `Ctrl+Shift+B`를 한 번 누르거나,
**터미널 → 작업 실행 → Configure: MSVC IntelliSense**를 실행합니다.
기본 자동완성 구성은 `MSVC x64 Debug`이며, Release 조건부 코드를 편집할 때는
**C/C++: Select IntelliSense Configuration**에서 `MSVC x64 Release`를 선택할 수 있습니다.
컴파일러 설치 경로가 바뀌어도 빌드 시 자동으로 갱신됩니다.

일반 PowerShell 터미널에서도 다음 명령으로 빌드할 수 있습니다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\.vscode\build.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\.vscode\build.ps1 -Configuration Release
```

설정 파일의 역할: `tasks.json`은 빌드, `launch.json`은 실행/디버깅,
`settings.json`은 편집기 설정, `extensions.json`은 필요한 확장 추천입니다.
([VS Code의 MSVC 설정 문서](https://code.visualstudio.com/docs/cpp/config-msvc))

### Visual Studio

기존 `.sln`을 열고 **Debug / x64** 또는 **Release / x64**를 선택합니다.
`Ctrl+Shift+B`로 빌드, `F5`로 디버깅, `Ctrl+F5`로 일반 실행합니다.
두 IDE는 같은 프로젝트 설정과 실행 파일을 사용합니다.

### 파일 관리 및 Git

| 파일/폴더 | 관리 방법 |
|---|---|
| `.cpp`, `.hpp`, `README.md` | 소스 및 문서이므로 Git에 포함 |
| `.sln`, `.vcxproj`, `.vcxproj.filters` | Visual Studio에 필요한 공유 설정이므로 유지하고 Git에 포함 |
| `.vscode/build.ps1`, `tasks.json`, `launch.json`, `settings.json`, `extensions.json` | VS Code 공유 설정이므로 Git에 포함 |
| `.vscode/c_cpp_properties.json` | PC별 경로가 들어가는 자동 생성 파일, Git에서 제외 |
| `.vs/`, `*.user` | IDE 캐시와 개인 설정, Git에서 제외 |
| `x64/`, `Win32/`, `Debug/`, `Release/`, `obj/` | 빌드 결과와 중간 파일, Git에서 제외 |
| `computat.486b2baf/` 등 `computat.*/` | 이전 빌드의 중간 파일, Git에서 제외 |

캐시 및 빌드 결과는 IDE와 실행 중인 프로그램을 종료한 상태에서 삭제해도 재생성됩니다.
파일 탐색기에서 일부 생성 폴더를 숨겼으며, 실제 파일은 남아 있습니다.
Git 변경 목록에는 소스, 문서, 공유 설정만 올리면 됩니다.

새 `.cpp`를 추가할 때는 Visual Studio의 **프로젝트 → 추가 → 기존 항목**으로 등록하거나
`.vcxproj`의 `ClCompile` 목록에 추가해야 두 IDE에서 함께 빌드됩니다.
`.hpp`는 `ClInclude`로 등록하며, 실제 사용하는 `.cpp`에서 `#include`합니다.
VS Code에서 파일을 만드는 것만으로는 Visual Studio 프로젝트에 자동 등록되지 않습니다.

소스와 문서는 UTF-8로 저장하며, MSVC에도 `/utf-8`을 적용했습니다.
C++20으로 바꾸고 싶다면 `.vcxproj`의 공통 `LanguageStandard`를 `stdcpp20`으로 바꾸고,
`.vscode/settings.json`의 `C_Cpp.default.cppStandard`를 `c++20`으로 맞춘 뒤 재빌드합니다.
생성된 자동완성 설정도 프로젝트의 표준을 따라 갱신됩니다.

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
├─ dcel.hpp
└─ triangulation.hpp
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

## Convex Polygon Intersection

`geometry/convex_polygon_intersection.hpp`는 다음 free function을 제공합니다.

```cpp
ConvexPolygon intersection(const ConvexPolygon& a, const ConvexPolygon& b);
double intersection_area(const ConvexPolygon& a, const ConvexPolygon& b);
```

입력은 CCW 순서의 단순 볼록 경계이며 첫 정점을 끝에 반복하지 않습니다.
중복 끝점과 불필요한 공선 정점은 정리합니다. 빈 집합, 점, 선분 입력과
교집합도 지원하며 결과의 `points` 크기 0/1/2가 각각 이를 나타냅니다.
양의 면적을 가진 결과는 CCW이며 `.area()`로 면적을 구합니다.
이러한 낮은 차원의 결과는 기본 생성 후 `points`를 채웁니다. 기존
`ConvexPolygon(vector)` 생성자의 full-dimensional assertion은 변경하지 않습니다.

Sutherland–Hodgman 클리핑으로 큰 경계를 작은 경계의 반평면들에 잘라냅니다.
시간은 `O(n*m+n+m)`, 추가 공간은 `O(n+m)`입니다. 선형 시간 교집합이나
최대 겹침 최적화 알고리즘을 구현한 것은 아닙니다. 수치 정책은 기존
`double`/`EPS`를 따르며 exact predicates를 보장하지 않습니다. 입력의 단순성은
전제이고, 기존 `is_simple()`의 미구현 부분을 이 함수에서 해결하지는 않습니다.

검증: `powershell -NoProfile -ExecutionPolicy Bypass -File tests/run_polygon_intersection_tests.ps1`
(`-Configuration Release`도 지원).

---

## Minkowski Sum and Translation Overlap Region

`algorithm/minkowski_sum.hpp`는 CCW 볼록 다각형의 변을 방향각 순서로 병합하여
민코프스키 합을 구하는 free function을 제공합니다.

```cpp
ConvexPolygon pairwise_hull(const ConvexPolygon& a, const ConvexPolygon& b, bool subtract_second);
ConvexPolygon minkowski_sum(const ConvexPolygon& a, const ConvexPolygon& b);
ConvexPolygon translation_overlap_region(const ConvexPolygon& p, const ConvexPolygon& q);
```

`minkowski_sum`은 `A+B = conv{a+b}`, `translation_overlap_region`은 `(P+t)`가 `Q`와 적어도 접하는
모든 이동 벡터의 집합 `D = Q+(-P) = conv{q-p}`를 반환합니다. 후자의
인자 순서는 이동하는 `P`, 고정된 `Q`입니다. 경계의 점/선분 접촉도 포함합니다.

시간과 추가 공간은 모두 `O(n+m)`입니다. 입력은 볼록 다각형의 CCW 경계 순서여야 하며,
시작 정점은 자유롭습니다. 빈 집합/점/선분, 연속 중복점, 마지막의 닫는 중복점과 공선 경계점도 처리합니다.
결과는 중복점과 불필요한 공선 경계점이 없는 CCW 경계이며, 퇴화하면 점 하나 또는 선분의 두 끝점을 반환합니다.
정렬되지 않은 점 집합은 호출 전에 별도로 `convex_hull`을 구해야 합니다.

`pairwise_hull(a, b, false)`는 `A+B`, `pairwise_hull(a, b, true)`는 `A+(-B)`입니다.
원점 대칭은 CCW를 유지하므로 같은 병합 로직을 사용하며, 대칭 후 시작 정점만 다시 찾습니다.
이는 모든 점 쌍의 차를 모은 집합으로, 차집합 `A\B`와는 다릅니다.

입력은 유한한 좌표와 기하 연산이 `double`의 표현 범위 안에 있는 CCW 볼록 경계를 전제로 합니다.
방향각 비교와 공선 정리는 기존 `Point::cross()`와 `orientation()`의 부호를 사용하며,
`EPS` 없는 부동소수점 판정입니다.

검증: `powershell -NoProfile -ExecutionPolicy Bypass -File tests/run_minkowski_sum_tests.ps1`
(`-Configuration Release`도 지원).

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

## DCEL

`DCEL`은 직선 간선으로 이루어진 평면 분할을 저장합니다. 간선의 왼쪽 면을
`HalfEdge::face`로 두고, `next` / `prev`는 그 면의 경계를 순회합니다.
비연결 성분, 구멍, 분리된 트리와 고립 정점도 같은 평면의 면에 배정합니다.

```cpp
DCEL d({{0, 0}, {2, 0}, {2, 2}, {0, 2}},
       {{0, 1}, {1, 2}, {2, 3}, {3, 0}});

int f = d.left_face(d.find_halfedge(0, 1));
int diagonal = d.split_face(f, 0, 2);
int midpoint = d.split_edge(d.find_halfedge(0, 1), {1, 0});

assert(d.is_valid());
assert(d.is_valid_geometry());
```

### 입력과 면 표현

- 좌표는 유한한 값이어야 하며, 서로 다른 정점의 좌표가 같으면 안 됩니다.
- 간선은 유효한 정점 ID를 연결해야 합니다. 자기 루프, 중복 간선과 겹침은 허용하지 않습니다.
- 간선은 공유 끝점에서만 만날 수 있습니다. 교차점이나 간선 내부의 정점은 입력 전에 간선을 나누어 표현해야 합니다.
- 기본 생성자도 외부 면 하나를 가진 유효한 빈 DCEL을 만듭니다.
- `Face::halfedge`는 유계 면의 외곽 경계 시작점입니다. 외부 면에는 외곽 경계가 없으므로 `-1`입니다.
- `Face::inner_halfedges`는 내부 경계들의 시작점을 저장합니다. 외부 면의 여러 경계와 분리된 트리의 경계 순회도 여기에 포함됩니다.
- 고립 정점은 `Vertex::face` 및 해당 면의 `isolated_vertices`에 기록합니다. 간선에 연결된 정점의 `face`는 `-1`입니다.

`face_boundaries(f)`로 각 경계의 시작 halfedge를 얻고, `boundary_halfedges(start)`로
그 경계만 순회합니다. `face_halfedges(f)`와 `face_vertices(f)`는 모든 경계를 이어 붙인 결과이며,
구멍이 있는 면에서는 하나의 polygon vertex sequence로 해석하면 안 됩니다.
`face_signed_area(f)`는 구멍을 포함한 유한 경계의 부호 있는 면적 합입니다.
외부 면에 대해서는 무한한 실제 면적을 의미하지 않습니다.

### 분할과 검증

- `split_face(f, u, v)`는 간선 방향을 이용해 올바른 삽입 위치를 찾습니다. 경계에 같은 정점이 반복되어도 처리합니다.
- `split_face_at_corners(eu, ev)`는 두 outgoing halfedge로 삽입 위치를 직접 지정합니다. 각 halfedge는 새 간선 방향의 바로 시계 방향 이웃이어야 합니다.
- 면 분할은 유계 면의 **동일한 외곽 경계**에 있는 두 정점을 연결합니다. 외부 면 분할, 내부 경계 자체의 분할, 서로 다른 경계 성분 연결은 아직 지원하지 않으며 오류로 처리합니다.
- 새 간선은 기존 간선이나 다른 정점을 부적절하게 교차·접촉하면 안 됩니다. 면 분할 후 구멍, 분리된 트리와 고립 정점의 소속도 갱신합니다.
- `split_edge(e, p)`의 `p`는 해당 선분의 내부에 있어야 합니다. 기존 끝점이나 이미 존재하는 정점은 사용할 수 없습니다.
- 분할 연산은 기존 ID를 유지하고 새 요소를 뒤에 추가합니다. 벡터 재할당 때문에 C++ 참조·포인터·iterator는 무효화될 수 있습니다.
- 잘못된 ID는 `std::out_of_range`, 지원하지 않거나 잘못된 기하 입력은 `std::invalid_argument`로 처리합니다. 입력 거부는 Release에서도 활성화됩니다.
- `is_valid()`는 참조, twin/next/prev, 정점 및 면 순회의 완전성, 고립 정점 소속과 연결 성분을 포함한 Euler 관계를 검사합니다.
- `is_valid_geometry()`는 추가로 간선 교차, 각도 순서와 기하학적 면 소속을 검사합니다. 생성 및 다시 생성하는 비용이 있어 테스트·진단용으로 사용합니다.

입력 검증은 현재 `O(VE + E²)`, 경계의 면 배정은 최악 `O(EF + VE)`입니다.
공선성과 위상적 접촉, 각도 순서는 전역 `EPS`를 사용하지 않고 부동소수점 부호로 판정합니다.
이는 exact predicate를 보장하지 않으며, 거의 공선인 입력이나 극단적인 좌표에 대한 수치 안정성 개선은 별도 과제입니다.

회귀 테스트는 VS Code의 **터미널 → 작업 실행 → Test: DCEL Debug x64 / Release x64**로 실행합니다.
두 IDE의 프로젝트 설정에서 컴파일러·SDK·C++ 표준을 읽어 사용하며, 테스트 결과물은 `obj/tests/`에 생성합니다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tests\run_dcel_tests.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\tests\run_dcel_tests.ps1 -Configuration Release
powershell -NoProfile -ExecutionPolicy Bypass -File .\tests\run_dcel_tests.ps1 -Platform Win32
```

---

## Triangulation

`structure/triangulation.hpp`는 삼각분할 알고리즘들이 공유하는 **결과 자료구조와 연산**을 제공합니다.
점 집합, 단순 폴리곤, 구멍이 있는 폴리곤의 삼각분할 알고리즘은 각각 구현하되,
계산한 결과를 `Triangulation`으로 반환할 수 있습니다.
대상 영역에 속하는 면은 모두 CCW 삼각형이며, 무한 외부 면과 폴리곤의 구멍은 대상에서 제외합니다.

### 결과 만들기

```cpp
#include "structure/triangulation.hpp"

auto t = Triangulation::from_triangles(
    {{0, 0}, {2, 0}, {2, 2}, {0, 2}},
    {{0, 1, 2}, {0, 2, 3}});

int diagonal = t.dcel().find_halfedge(0, 2);
if (t.can_flip(diagonal)) t.flip_edge(diagonal); // 0-2 -> 1-3

auto hit = locate(t, {1, 1});
if (hit.type == Triangulation::LocationType::EDGE) {
    bool domain_boundary = t.is_boundary_edge(hit.halfedge);
}
```

- `from_triangles(points, triangles)`는 **이미 계산된** 삼각형마다 정점 ID 3개를 담은 목록을 가져옵니다. 입력 삼각형의 CW/CCW 방향은 모두 허용합니다. 정점 ID는 입력 순서를 유지합니다.
- `Triangulation(dcel)`은 DCEL의 모든 유계 면을 삼각분할 대상으로 지정합니다.
- `Triangulation(dcel, triangle_faces)`는 지정한 면 ID들만 대상으로 삼습니다. 구멍이 삼각형 모양이어도 자동 추론할 수 없으므로, 구멍이 있는 DCEL은 이 생성자로 대상을 지정합니다.
- 생성 시 DCEL의 기하 검증과 삼각형 조건을 검사합니다. 겹치는 삼각형, 0 면적, 중복 삼각형, 삼각형 안의 고립 정점 등은 거부합니다.
- 대상 삼각형은 경계 간선 3개와 서로 다른 정점 3개를 가지며, 내부 경계나 고립 정점이 없습니다. 제외된 면에는 이 조건을 요구하지 않습니다.
- 대상 삼각형이 없는 결과는 빈 영역입니다. 사용하지 않는 정점은 조회의 대상 영역에 포함되지 않습니다. 점 하나나 공선 점 집합에 대한 0차원/1차원 삼각분할 정책은 향후 알고리즘에서 별도로 정해야 합니다.

`dcel()`은 읽기 전용 참조를 제공합니다. 생성 중에는 일반 `DCEL`을 사용하고,
완성된 결과를 `Triangulation`에 전달합니다. 삼각형을 사각형으로 바꾸는 일반 DCEL 분할 연산이
완성된 결과의 조건을 깨뜨리지 않도록 구성했습니다.

### 공통 조회

| 함수 | 결과 |
|---|---|
| `triangle_faces()` | 대상 삼각형들의 DCEL 면 ID 목록 |
| `is_triangle(f)` | 해당 면이 대상 삼각형인지 여부 |
| `triangle_halfedges(f)` / `triangle_vertices(f)` | CCW 순서의 halfedge / 정점 ID 3개 |
| `triangle_neighbors(f)` | 위 halfedge 순서에 대응하는 이웃 삼각형 ID; 영역 밖이면 `-1` |
| `incident_triangles(v)` | 정점에 닿는 대상 삼각형들 |
| `opposite_vertex(e)` | 방향 있는 간선 `e`의 왼쪽 삼각형에서 반대편 정점 |
| `triangle_area(f)` | 삼각형 면적 |
| `is_boundary_edge(e)` | 양쪽 중 한 면만 대상 삼각형인 경계 간선인지 여부 |
| `is_interior_edge(e)` | 양쪽 면 모두 대상 삼각형인 내부 간선인지 여부 |
| `locate(t, p)` / `t.locate(p)` | 정점·간선·삼각형 내부·영역 밖 중 어디에 있는지와 관련 ID |

점 위치 결과는 `LocationType::VERTEX`, `EDGE`, `FACE`, `OUTSIDE`입니다.
`face`는 해당 위치에 닿는 대상 삼각형 하나이며, `EDGE`는 `halfedge`, `VERTEX`는 `vertex`를 제공합니다.
사용하지 않는 ID와 영역 밖의 `face`는 `-1`입니다. 공통 간선·정점에서는 어느 쪽 대상 삼각형이 반환되어도 유효합니다.
`EDGE`에는 삼각분할 내부 간선도 포함되므로, 폴리곤의 경계인지 알아보려면 `is_boundary_edge()`를 사용합니다.
점 위치는 현재 삼각형 전체를 순회하며, 폴리곤의 기존 finite-ray 판정에 의존하지 않습니다.
비유한 쿼리 좌표는 `std::invalid_argument`, 방향 판정의 산술 overflow는 `std::overflow_error`로 처리합니다.

### Edge flip과 제약 간선

`can_flip(e)`는 다음 조건을 확인합니다.

- 양쪽 면이 대상 삼각형이어야 합니다. 영역의 외곽과 구멍 경계는 flip할 수 없습니다.
- 두 삼각형의 합집합이 엄격히 볼록한 사각형이어야 합니다. 새 삼각형의 면적이 0인 경우도 거부합니다.
- 새 대각선이 이미 존재하지 않아야 하며, 기존 간선에 명시적인 제약 표시가 없어야 합니다.

`flip_edge(e)`는 가능한 경우 대각선을 교체하고 `true`를 반환합니다. 불가능한 flip은 변경 없이 `false`를 반환합니다.
잘못된 ID는 다른 조회와 마찬가지로 `std::out_of_range`입니다.
정점·halfedge·면 ID는 유지하지만, 기존 대각선의 halfedge ID는 **새 대각선**을 나타내게 됩니다.
정점의 대표 halfedge, 삼각형 순환, 이웃 관계도 함께 갱신합니다.

`set_constrained(e, true/false)`는 알고리즘이 유지해야 할 내부 선분 등에 제약 표시를 설정합니다.
표시는 twin에도 함께 적용되며 `is_constrained(e)`로 조회합니다. 영역 경계는 이 표시와 무관하게 항상 보호됩니다.
제약 표시를 설정하는 것만으로 간선을 삽입하거나 constrained Delaunay 삼각분할을 계산하지는 않습니다.
일반 flip은 Delaunay 조건을 검사하지 않으며, 그 조건을 유지하는 알고리즘은 별도로 구현해야 합니다.
([CGAL의 일반 삼각분할 flip 규약](https://doc.cgal.org/latest/Triangulation_2/classCGAL_1_1Triangulation__2.html))

`is_valid()`는 DCEL 구조, 대상 면의 삼각형 조건과 제약 표시를 검사하고,
`is_valid_geometry()`는 DCEL 기하 검증도 실행합니다. Debug flip 후에는 구조 검증 assertion이 적용됩니다.
위상 판정은 DCEL과 같은 엄격한 부동소수점 부호를 사용하며 exact/adaptive predicate는 아직 제공하지 않습니다.

회귀 테스트는 **터미널 → 작업 실행 → Test: Triangulation Debug x64 / Release x64**로 실행합니다.
사각형과 오목한 영역, 구멍, 비연결 영역, 제약 간선 및 무작위 연속 flip의 위상·면적 보존을 검사합니다.
테스트 컴파일 설정은 Visual Studio 프로젝트와 공유합니다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tests\run_triangulation_tests.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\tests\run_triangulation_tests.ps1 -Configuration Release
powershell -NoProfile -ExecutionPolicy Bypass -File .\tests\run_triangulation_tests.ps1 -Platform Win32
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
| `Triangulation::triangle_vertices`, `triangle_neighbors`, `triangle_area` | `O(1)` |
| `Triangulation::incident_triangles(v)` | `O(deg(v))` |
| `Triangulation::locate(p)` | `O(T)`; `T`는 대상 삼각형 수 |
| `Triangulation::can_flip(e)`, `flip_edge(e)` | 새 대각선 중복 확인 `O(deg(c))` + 연결 갱신 `O(1)`; Debug 구조 assertion 비용 별도 |

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
- 모든 polygon의 경계 정점은 CCW 순서를 전제로 연산합니다.
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

- [x] 삼각분할 공통 결과 자료구조, 조회와 edge flip
- [ ] 점 집합 / 폴리곤 삼각분할 알고리즘, Delaunay 조건 검사 및 점 위치 조회 가속
- [ ] DCEL 및 triangulation 확장: 분할 후 `assert(is_valid())`는 Debug에서 유지하고, 기하 검증은 회귀 테스트에서 실행합니다. Release에서는 `NDEBUG`로 전체 구조 assertion을 비활성화합니다.
- [ ] Voronoi Diagram
- [ ] Visibility Complex

---

## Goal

최종적으로는 단순한 PS용 코드 모음이 아니라,

- 계산기하 알고리즘을 직접 구현하고 검증할 수 있고
- 새로운 알고리즘을 실험할 때 재사용할 수 있으며
- Visibility Complex와 같은 보다 복잡한 기하 자료구조의 기반으로 사용할 수 있는

2D Computational Geometry Library를 만드는 것을 목표로 합니다.
