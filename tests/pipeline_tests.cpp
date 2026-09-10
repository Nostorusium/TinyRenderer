#include "tinyrenderer/clipper.hpp"
#include "tinyrenderer/depth_buffer.hpp"
#include "tinyrenderer/image.hpp"
#include "tinyrenderer/math.hpp"
#include "tinyrenderer/rasterizer.hpp"

#include <cmath>
#include <iostream>
#include <optional>
#include <numbers>

namespace {

int failures = 0;

void check(const bool condition, const char* message)
{
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

void check_close(const float actual, const float expected, const char* message)
{
    if (std::abs(actual - expected) > 1.0e-5F) {
        std::cerr << "FAILED: " << message << ", expected " << expected
                  << " but got " << actual << '\n';
        ++failures;
    }
}

std::optional<tinyrenderer::ScreenVertex> transform_vertex(
    const tinyrenderer::Vec3 local_position,
    const tinyrenderer::Mat4& model_view_projection,
    const tinyrenderer::Mat4& viewport)
{
    const tinyrenderer::Vec4 clip_position = model_view_projection
        * tinyrenderer::Vec4{
            local_position.x, local_position.y, local_position.z, 1.0F};
    const auto ndc_position = tinyrenderer::perspective_divide(clip_position);
    if (!ndc_position) {
        return std::nullopt;
    }

    const tinyrenderer::Vec4 viewport_position = viewport
        * tinyrenderer::Vec4{
            ndc_position->x, ndc_position->y, ndc_position->z, 1.0F};
    return tinyrenderer::ScreenVertex{
        {viewport_position.x, viewport_position.y},
        viewport_position.z};
}

} // namespace

int main()
{
    using tinyrenderer::Mat4;
    using tinyrenderer::Vec3;

    constexpr int width = 60;
    constexpr int height = 60;
    const auto view = tinyrenderer::look_at({0.0F, 0.0F, 3.0F},
                                            {0.0F, 0.0F, 0.0F},
                                            {0.0F, 1.0F, 0.0F});
    const auto projection = tinyrenderer::perspective(
        std::numbers::pi_v<float> / 2.0F, 1.0F, 1.0F, 10.0F);
    const auto viewport = tinyrenderer::viewport(width, height);
    if (!view || !projection || !viewport) {
        std::cerr << "FAILED: deterministic pipeline can be constructed\n";
        return 1;
    }

    const Mat4 near_mvp = *projection * *view * Mat4::identity();
    const Mat4 far_mvp =
        *projection * *view * Mat4::translation({0.0F, 0.0F, -1.0F});

    const auto near0 = transform_vertex(Vec3{-1.0F, -1.0F, 0.0F},
                                        near_mvp,
                                        *viewport);
    const auto near1 = transform_vertex(Vec3{1.0F, -1.0F, 0.0F},
                                        near_mvp,
                                        *viewport);
    const auto near2 = transform_vertex(Vec3{0.0F, 1.0F, 0.0F},
                                        near_mvp,
                                        *viewport);
    const auto far0 = transform_vertex(Vec3{-1.0F, -1.0F, 0.0F},
                                       far_mvp,
                                       *viewport);
    const auto far1 = transform_vertex(Vec3{1.0F, -1.0F, 0.0F},
                                       far_mvp,
                                       *viewport);
    const auto far2 = transform_vertex(Vec3{0.0F, 1.0F, 0.0F},
                                       far_mvp,
                                       *viewport);
    if (!near0 || !near1 || !near2 || !far0 || !far1 || !far2) {
        std::cerr << "FAILED: visible triangle vertices can be transformed\n";
        return 1;
    }

    check_close(near0->position.x, 20.0F,
                "local left vertex reaches the expected screen x");
    check_close(near0->position.y, 40.0F,
                "local bottom vertex reaches the expected flipped screen y");
    check_close(near0->depth, 20.0F / 27.0F,
                "camera distance reaches the expected viewport depth");

    constexpr tinyrenderer::Color black{};
    constexpr tinyrenderer::Color near_color{235, 120, 70};
    constexpr tinyrenderer::Color far_color{70, 125, 230};
    tinyrenderer::Image image{width, height, black};
    tinyrenderer::DepthBuffer depth_buffer{width, height};

    // 故意先画近处、再画远处，验证三维变换产生的深度确实接入了 Z-buffer。
    tinyrenderer::draw_triangle_with_depth(
        image, depth_buffer, *near0, *near1, *near2, near_color);
    tinyrenderer::draw_triangle_with_depth(
        image, depth_buffer, *far0, *far1, *far2, far_color);

    check(image.pixel(30, 30) == near_color,
          "transformed nearer triangle owns the overlapping pixel");
    check_close(depth_buffer.depth(30, 30), 20.0F / 27.0F,
                "rasterizer retains the transformed nearer depth");
    check(image.pixel(5, 5) == black,
          "pixels outside the projected triangle retain the clear color");

    const Mat4 view_projection = *projection * *view;
    const auto to_clip_vertex =
        [&view_projection](const Vec3 world_position) {
        return tinyrenderer::ClipVertex{
            view_projection
            * tinyrenderer::Vec4{
                world_position.x, world_position.y, world_position.z, 1.0F}};
    };

    const tinyrenderer::ClipTriangle crossing_near{{
        to_clip_vertex({0.0F, 0.4F, 2.5F}),
        to_clip_vertex({-0.8F, -0.6F, 0.0F}),
        to_clip_vertex({0.8F, -0.6F, 0.0F}),
    }};
    const auto clipped_near =
        tinyrenderer::clip_triangle_to_frustum(crossing_near);
    check(clipped_near.size() == 2,
          "world triangle crossing near plane becomes two safe triangles");
    for (const auto& triangle : clipped_near) {
        for (const auto vertex : triangle) {
            check(tinyrenderer::perspective_divide(vertex.position).has_value(),
                  "clipped pipeline vertex can safely pass perspective divide");
        }
    }

    const tinyrenderer::ClipTriangle behind_camera{{
        to_clip_vertex({-0.2F, -0.2F, 4.0F}),
        to_clip_vertex({0.2F, -0.2F, 4.0F}),
        to_clip_vertex({0.0F, 0.2F, 4.0F}),
    }};
    check(tinyrenderer::clip_triangle_to_frustum(behind_camera).empty(),
          "triangle behind camera is discarded before perspective divide");

    if (failures == 0) {
        std::cout << "All pipeline tests passed.\n";
    }
    return failures == 0 ? 0 : 1;
}
