#include "tinyrenderer/clipper.hpp"
#include "tinyrenderer/depth_buffer.hpp"
#include "tinyrenderer/image.hpp"
#include "tinyrenderer/math.hpp"
#include "tinyrenderer/model.hpp"
#include "tinyrenderer/rasterizer.hpp"

#include <array>
#include <filesystem>
#include <iostream>
#include <numbers>
#include <optional>
#include <vector>

int main()
{
    constexpr int width = 256;
    constexpr int height = 256;
    tinyrenderer::Image image{width, height, {12, 12, 18}};
    tinyrenderer::DepthBuffer depth_buffer{width, height};

    tinyrenderer::Model cube;
    std::string error;
    if (!tinyrenderer::load_obj("models/cube.obj", cube, error)) {
        std::cerr << error << '\n';
        return 1;
    }

    constexpr float pi = std::numbers::pi_v<float>;
    const tinyrenderer::Mat4 model =
        tinyrenderer::Mat4::translation({0.0F, -0.05F, 0.0F})
        * tinyrenderer::Mat4::scaling({0.9F, 0.9F, 0.9F});
    const auto view = tinyrenderer::look_at({2.4F, 1.8F, 3.2F},
                                            {0.0F, 0.0F, 0.0F},
                                            {0.0F, 1.0F, 0.0F});
    const auto projection = tinyrenderer::perspective(
        pi / 3.0F,
        static_cast<float>(width) / static_cast<float>(height),
        0.1F,
        100.0F);
    const auto viewport = tinyrenderer::viewport(width, height);
    if (!view || !projection || !viewport) {
        std::cerr << "Could not construct the transformation pipeline\n";
        return 1;
    }

    // 列向量从右向左执行：局部坐标先经 Model，再经 View，最后进入裁剪空间。
    const tinyrenderer::Mat4 model_view_projection = *projection * *view * model;

    std::vector<tinyrenderer::ClipVertex> clip_vertices;
    clip_vertices.reserve(cube.vertices.size());
    for (const auto& vertex : cube.vertices) {
        const tinyrenderer::Vec4 clip_position = model_view_projection
            * tinyrenderer::Vec4{vertex.x, vertex.y, vertex.z, 1.0F};
        clip_vertices.push_back({clip_position});
    }

    const auto to_screen_vertex =
        [&viewport](const tinyrenderer::ClipVertex vertex)
        -> std::optional<tinyrenderer::ScreenVertex> {
        const auto ndc_position =
            tinyrenderer::perspective_divide(vertex.position);
        if (!ndc_position) {
            return std::nullopt;
        }

        const tinyrenderer::Vec4 viewport_position = *viewport
            * tinyrenderer::Vec4{
                ndc_position->x, ndc_position->y, ndc_position->z, 1.0F};
        return tinyrenderer::ScreenVertex{
            {viewport_position.x, viewport_position.y},
            viewport_position.z};
    };

    constexpr std::array<tinyrenderer::Color, 6> face_colors{{
        {225, 90, 75},
        {65, 125, 225},
        {235, 175, 65},
        {85, 190, 125},
        {165, 100, 220},
        {70, 185, 205},
    }};

    for (std::size_t face_index = 0; face_index < cube.faces.size(); ++face_index) {
        const auto& face = cube.faces[face_index];
        const tinyrenderer::ClipTriangle input_triangle{{
            clip_vertices[face.vertex_indices[0]],
            clip_vertices[face.vertex_indices[1]],
            clip_vertices[face.vertex_indices[2]],
        }};

        // 裁剪先于透视除法；部分可见的面可能被重新拆成多个三角形。
        for (const auto& clipped_triangle :
             tinyrenderer::clip_triangle_to_frustum(input_triangle)) {
            const auto vertex0 = to_screen_vertex(clipped_triangle[0]);
            const auto vertex1 = to_screen_vertex(clipped_triangle[1]);
            const auto vertex2 = to_screen_vertex(clipped_triangle[2]);
            if (!vertex0 || !vertex1 || !vertex2) {
                std::cerr << "A clipped cube vertex cannot be projected\n";
                return 1;
            }

            tinyrenderer::draw_triangle_with_depth(
                image,
                depth_buffer,
                *vertex0,
                *vertex1,
                *vertex2,
                face_colors[(face_index / 2) % face_colors.size()]);
        }
    }

    const std::filesystem::path output_directory{"output"};
    std::filesystem::create_directories(output_directory);
    const auto output_path = output_directory / "pipeline.ppm";
    if (!image.write_ppm(output_path)) {
        std::cerr << "Failed to write " << output_path << '\n';
        return 1;
    }

    std::cout << "Wrote " << output_path << '\n';
    return 0;
}
