#include "tinyrenderer/image.hpp"
#include "tinyrenderer/model.hpp"
#include "tinyrenderer/rasterizer.hpp"

#include <cmath>
#include <filesystem>
#include <iostream>
#include <utility>

int main(const int argc, char* argv[])
{
    constexpr int width = 256;
    constexpr int height = 256;

    tinyrenderer::Image image{width, height, {12, 12, 18}};

    const std::filesystem::path model_path = argc > 1 ? argv[1] : "models/cube.obj";
    tinyrenderer::Model model;
    std::string error;
    if (!tinyrenderer::load_obj(model_path, model, error)) {
        std::cerr << error << '\n';
        return 1;
    }

    const auto to_screen = [](const tinyrenderer::Vertex& vertex) {
        // 临时使用斜投影，让立方体的深度也能看出来；以后由矩阵变换替代。
        const float projected_x = vertex.x + 0.35F * vertex.z;
        const float projected_y = vertex.y - 0.25F * vertex.z;
        const int screen_x = static_cast<int>(std::lround(width / 2.0F + projected_x * 80.0F));
        const int screen_y = static_cast<int>(std::lround(height / 2.0F - projected_y * 80.0F));
        return std::pair{screen_x, screen_y};
    };

    constexpr tinyrenderer::Color line_color{230, 235, 255};
    for (const auto& face : model.faces) {
        for (std::size_t edge = 0; edge < face.vertex_indices.size(); ++edge) {
            const auto& first_vertex = model.vertices[face.vertex_indices[edge]];
            const auto& second_vertex =
                model.vertices[face.vertex_indices[(edge + 1) % face.vertex_indices.size()]];
            const auto [x0, y0] = to_screen(first_vertex);
            const auto [x1, y1] = to_screen(second_vertex);
            tinyrenderer::draw_line(image, x0, y0, x1, y1, line_color);
        }
    }

    const std::filesystem::path output_directory{"output"};
    std::filesystem::create_directories(output_directory);
    const auto output_path = output_directory / "wireframe.ppm";

    if (!image.write_ppm(output_path)) {
        std::cerr << "Failed to write " << output_path << '\n';
        return 1;
    }

    std::cout << "Wrote " << output_path << '\n';
    return 0;
}
