#include "tinyrenderer/clipper.hpp"

#include <array>
#include <cmath>
#include <iostream>
#include <limits>

namespace {

int failures = 0;

void check(const bool condition, const char* message)
{
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

bool inside_frustum(const tinyrenderer::ClipVertex vertex)
{
    const auto position = vertex.position;
    return std::isfinite(position.x)
        && std::isfinite(position.y)
        && std::isfinite(position.z)
        && std::isfinite(position.w)
        && position.x >= -position.w
        && position.x <= position.w
        && position.y >= -position.w
        && position.y <= position.w
        && position.z >= -position.w
        && position.z <= position.w;
}

} // namespace

int main()
{
    using tinyrenderer::ClipTriangle;
    using tinyrenderer::ClipVertex;

    const ClipTriangle inside{{
        {{-0.5F, -0.5F, 0.0F, 1.0F}},
        {{0.5F, -0.5F, 0.0F, 1.0F}},
        {{0.0F, 0.5F, 0.0F, 1.0F}},
    }};
    const auto unchanged = tinyrenderer::clip_triangle_to_frustum(inside);
    check(unchanged.size() == 1, "fully visible triangle remains one triangle");

    const std::array fully_outside{
        ClipTriangle{{{{-2.0F, -0.2F, 0.0F, 1.0F}},
                      {{-2.0F, 0.2F, 0.0F, 1.0F}},
                      {{-1.5F, 0.0F, 0.0F, 1.0F}}}},
        ClipTriangle{{{{2.0F, -0.2F, 0.0F, 1.0F}},
                      {{2.0F, 0.2F, 0.0F, 1.0F}},
                      {{1.5F, 0.0F, 0.0F, 1.0F}}}},
        ClipTriangle{{{{-0.2F, -2.0F, 0.0F, 1.0F}},
                      {{0.2F, -2.0F, 0.0F, 1.0F}},
                      {{0.0F, -1.5F, 0.0F, 1.0F}}}},
        ClipTriangle{{{{-0.2F, 2.0F, 0.0F, 1.0F}},
                      {{0.2F, 2.0F, 0.0F, 1.0F}},
                      {{0.0F, 1.5F, 0.0F, 1.0F}}}},
        ClipTriangle{{{{-0.2F, 0.0F, -2.0F, 1.0F}},
                      {{0.2F, 0.0F, -2.0F, 1.0F}},
                      {{0.0F, 0.2F, -1.5F, 1.0F}}}},
        ClipTriangle{{{{-0.2F, 0.0F, 2.0F, 1.0F}},
                      {{0.2F, 0.0F, 2.0F, 1.0F}},
                      {{0.0F, 0.2F, 1.5F, 1.0F}}}},
    };
    for (const auto& triangle : fully_outside) {
        check(tinyrenderer::clip_triangle_to_frustum(triangle).empty(),
              "triangle outside one frustum plane is discarded");
    }

    const ClipTriangle crossing_near{{
        {{0.0F, 0.0F, -2.0F, 1.0F}},
        {{-0.5F, -0.5F, 0.0F, 1.0F}},
        {{0.5F, -0.5F, 0.0F, 1.0F}},
    }};
    const auto clipped = tinyrenderer::clip_triangle_to_frustum(crossing_near);
    check(clipped.size() == 2,
          "triangle crossing near plane becomes a clipped quad and two triangles");

    bool found_near_intersection = false;
    for (const auto& triangle : clipped) {
        for (const ClipVertex vertex : triangle) {
            check(inside_frustum(vertex), "every generated vertex is inside the frustum");
            if (std::abs(vertex.position.z + vertex.position.w) <= 1.0e-5F) {
                found_near_intersection = true;
            }
        }
    }
    check(found_near_intersection, "near-plane clipping creates boundary vertices");

    ClipTriangle invalid = inside;
    invalid[0].position.x = std::numeric_limits<float>::quiet_NaN();
    check(tinyrenderer::clip_triangle_to_frustum(invalid).empty(),
          "non-finite clip coordinates are rejected");

    if (failures == 0) {
        std::cout << "All clipper tests passed.\n";
    }
    return failures == 0 ? 0 : 1;
}
