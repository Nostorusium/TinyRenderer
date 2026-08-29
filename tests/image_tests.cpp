#include "tinyrenderer/image.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace {

int failures = 0;

void check(const bool condition, const char* message)
{
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

} // namespace

int main()
{
    using tinyrenderer::Color;
    using tinyrenderer::Image;

    const Color black{};
    const Color orange{255, 128, 32};
    Image image{3, 2, black};

    check(image.width() == 3, "width is retained");
    check(image.height() == 2, "height is retained");
    check(image.set_pixel(2, 1, orange), "in-bounds pixel can be written");
    check(image.pixel(2, 1) == orange, "pixel is read from the expected location");
    check(image.pixel(0, 0) == black, "other pixels retain the clear color");
    check(!image.set_pixel(-1, 0, orange), "negative x is rejected");
    check(!image.set_pixel(3, 1, orange), "x at width is rejected");
    check(!image.set_pixel(0, 2, orange), "y at height is rejected");

    const std::filesystem::path output_path{"image_test.ppm"};
    check(image.write_ppm(output_path), "PPM file can be written");

    std::ifstream input{output_path, std::ios::binary};
    const std::string bytes{
        std::istreambuf_iterator<char>{input}, std::istreambuf_iterator<char>{}};

    const std::string expected_header{"P6\n3 2\n255\n"};
    check(bytes.starts_with(expected_header), "PPM header describes a binary 3x2 image");
    check(bytes.size() == expected_header.size() + 3U * 2U * 3U,
          "PPM contains exactly three bytes per pixel");

    input.close();
    std::filesystem::remove(output_path);

    if (failures == 0) {
        std::cout << "All image tests passed.\n";
    }
    return failures == 0 ? 0 : 1;
}
