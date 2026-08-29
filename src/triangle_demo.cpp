#include "tinyrenderer/image.hpp"
#include "tinyrenderer/rasterizer.hpp"

#include <filesystem>
#include <iostream>

int main()
{
    tinyrenderer::Image image{256, 256, {12, 12, 18}};
    tinyrenderer::draw_triangle(image,
                                {40.0F, 220.0F},
                                {128.0F, 30.0F},
                                {220.0F, 210.0F},
                                {235, 120, 70});

    const std::filesystem::path output_directory{"output"};
    std::filesystem::create_directories(output_directory);
    const auto output_path = output_directory / "triangle.ppm";
    if (!image.write_ppm(output_path)) {
        std::cerr << "Failed to write " << output_path << '\n';
        return 1;
    }

    std::cout << "Wrote " << output_path << '\n';
    return 0;
}
