#include "tinyrenderer/depth_buffer.hpp"
#include "tinyrenderer/image.hpp"
#include "tinyrenderer/rasterizer.hpp"

#include <filesystem>
#include <iostream>

int main()
{
    constexpr int width = 256;
    constexpr int height = 256;
    tinyrenderer::Image image{width, height, {12, 12, 18}};
    tinyrenderer::DepthBuffer depth_buffer{width, height};

    // 故意先画近处、再画远处：正确遮挡应由深度决定，而不是由绘制顺序决定。
    tinyrenderer::draw_triangle_with_depth(image,
                                           depth_buffer,
                                           {{45.0F, 70.0F}, 0.25F},
                                           {{215.0F, 85.0F}, 0.25F},
                                           {{130.0F, 235.0F}, 0.25F},
                                           {235, 120, 70});
    tinyrenderer::draw_triangle_with_depth(image,
                                           depth_buffer,
                                           {{30.0F, 220.0F}, 0.75F},
                                           {{128.0F, 25.0F}, 0.75F},
                                           {{225.0F, 220.0F}, 0.75F},
                                           {70, 125, 230});

    const std::filesystem::path output_directory{"output"};
    std::filesystem::create_directories(output_directory);
    const auto output_path = output_directory / "depth.ppm";
    if (!image.write_ppm(output_path)) {
        std::cerr << "Failed to write " << output_path << '\n';
        return 1;
    }

    std::cout << "Wrote " << output_path << '\n';
    return 0;
}
