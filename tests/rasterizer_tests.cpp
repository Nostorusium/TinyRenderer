#include "tinyrenderer/depth_buffer.hpp"
#include "tinyrenderer/image.hpp"
#include "tinyrenderer/rasterizer.hpp"

#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <iostream>
#include <utility>

namespace {

using Pixel = std::pair<int, int>;

int failures = 0;

void check_exact_pixels(const char* test_name,
                        const tinyrenderer::Image& image,
                        const std::initializer_list<Pixel> expected_pixels,
                        const tinyrenderer::Color line_color)
{
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const bool should_be_colored =
                std::find(expected_pixels.begin(), expected_pixels.end(), Pixel{x, y})
                != expected_pixels.end();
            const auto expected_color = should_be_colored
                ? line_color
                : tinyrenderer::Color{};

            if (image.pixel(x, y) != expected_color) {
                std::cerr << "FAILED: " << test_name << " at (" << x << ", " << y << ")\n";
                ++failures;
            }
        }
    }
}

void check_same_image(const char* test_name,
                      const tinyrenderer::Image& first,
                      const tinyrenderer::Image& second)
{
    for (int y = 0; y < first.height(); ++y) {
        for (int x = 0; x < first.width(); ++x) {
            if (first.pixel(x, y) != second.pixel(x, y)) {
                std::cerr << "FAILED: " << test_name << " at (" << x << ", " << y << ")\n";
                ++failures;
            }
        }
    }
}

void check_close(const char* test_name, const float actual, const float expected)
{
    if (std::abs(actual - expected) > 1.0e-5F) {
        std::cerr << "FAILED: " << test_name << ", expected " << expected
                  << " but got " << actual << '\n';
        ++failures;
    }
}

} // namespace

int main()
{
    constexpr tinyrenderer::Color white{255, 255, 255};

    tinyrenderer::Image horizontal{7, 7};
    tinyrenderer::draw_line(horizontal, 1, 2, 5, 2, white);
    check_exact_pixels("horizontal line",
                       horizontal,
                       {{1, 2}, {2, 2}, {3, 2}, {4, 2}, {5, 2}},
                       white);

    tinyrenderer::Image shallow{7, 7};
    tinyrenderer::draw_line(shallow, 1, 1, 5, 3, white);
    check_exact_pixels("shallow line",
                       shallow,
                       {{1, 1}, {2, 2}, {3, 2}, {4, 3}, {5, 3}},
                       white);

    tinyrenderer::Image steep{7, 7};
    tinyrenderer::draw_line(steep, 1, 1, 3, 5, white);
    check_exact_pixels("steep line",
                       steep,
                       {{1, 1}, {2, 2}, {2, 3}, {3, 4}, {3, 5}},
                       white);

    tinyrenderer::Image negative_slope{7, 7};
    tinyrenderer::draw_line(negative_slope, 1, 5, 5, 3, white);
    check_exact_pixels("negative slope",
                       negative_slope,
                       {{1, 5}, {2, 4}, {3, 4}, {4, 3}, {5, 3}},
                       white);

    tinyrenderer::Image vertical{7, 7};
    tinyrenderer::draw_line(vertical, 3, 5, 3, 1, white);
    check_exact_pixels("vertical line",
                       vertical,
                       {{3, 1}, {3, 2}, {3, 3}, {3, 4}, {3, 5}},
                       white);

    tinyrenderer::Image point{7, 7};
    tinyrenderer::draw_line(point, 3, 4, 3, 4, white);
    check_exact_pixels("single point", point, {{3, 4}}, white);

    tinyrenderer::Image clipped_by_image{4, 3};
    tinyrenderer::draw_line(clipped_by_image, -2, 1, 2, 1, white);
    check_exact_pixels("partially outside line",
                       clipped_by_image,
                       {{0, 1}, {1, 1}, {2, 1}},
                       white);

    tinyrenderer::Image forward{7, 7};
    tinyrenderer::Image backward{7, 7};
    tinyrenderer::draw_line(forward, 1, 1, 5, 3, white);
    tinyrenderer::draw_line(backward, 5, 3, 1, 1, white);
    check_same_image("reversing endpoints", forward, backward);

    using tinyrenderer::ScreenPoint;
    constexpr ScreenPoint point0{1.0F, 1.0F};
    constexpr ScreenPoint point1{5.0F, 1.0F};
    constexpr ScreenPoint point2{1.0F, 5.0F};

    const auto at_vertex =
        tinyrenderer::barycentric_at(point0, point1, point2, point0);
    if (at_vertex) {
        check_close("first vertex weight 0", at_vertex->weight0, 1.0F);
        check_close("first vertex weight 1", at_vertex->weight1, 0.0F);
        check_close("first vertex weight 2", at_vertex->weight2, 0.0F);
    } else {
        std::cerr << "FAILED: vertex has barycentric coordinates\n";
        ++failures;
    }

    const auto on_edge = tinyrenderer::barycentric_at(
        point0, point1, point2, ScreenPoint{3.0F, 1.0F});
    if (on_edge) {
        check_close("edge weight 0", on_edge->weight0, 0.5F);
        check_close("edge weight 1", on_edge->weight1, 0.5F);
        check_close("edge weight 2", on_edge->weight2, 0.0F);
    } else {
        std::cerr << "FAILED: edge has barycentric coordinates\n";
        ++failures;
    }

    const auto inside = tinyrenderer::barycentric_at(
        point0, point1, point2, ScreenPoint{2.0F, 2.0F});
    if (inside) {
        check_close("barycentric weights sum to one",
                    inside->weight0 + inside->weight1 + inside->weight2,
                    1.0F);
    } else {
        std::cerr << "FAILED: inside point has barycentric coordinates\n";
        ++failures;
    }

    const auto degenerate_weights = tinyrenderer::barycentric_at(
        ScreenPoint{1.0F, 1.0F},
        ScreenPoint{2.0F, 2.0F},
        ScreenPoint{3.0F, 3.0F},
        ScreenPoint{2.0F, 2.0F});
    if (degenerate_weights) {
        std::cerr << "FAILED: degenerate triangle has no barycentric coordinates\n";
        ++failures;
    }

    tinyrenderer::Image triangle{7, 7};
    tinyrenderer::draw_triangle(triangle, point0, point1, point2, white);
    check_exact_pixels("filled triangle",
                       triangle,
                       {{1, 1}, {2, 1}, {3, 1}, {4, 1},
                        {1, 2}, {2, 2}, {3, 2},
                        {1, 3}, {2, 3},
                        {1, 4}},
                       white);

    tinyrenderer::Image reversed_triangle{7, 7};
    tinyrenderer::draw_triangle(reversed_triangle, point2, point1, point0, white);
    check_same_image("reversing triangle winding", triangle, reversed_triangle);

    tinyrenderer::Image degenerate_triangle{7, 7};
    tinyrenderer::draw_triangle(degenerate_triangle,
                                ScreenPoint{1.0F, 1.0F},
                                ScreenPoint{2.0F, 2.0F},
                                ScreenPoint{3.0F, 3.0F},
                                white);
    check_exact_pixels("degenerate triangle", degenerate_triangle, {}, white);

    tinyrenderer::Image clipped_triangle{4, 4};
    tinyrenderer::draw_triangle(clipped_triangle,
                                ScreenPoint{-1.0F, -1.0F},
                                ScreenPoint{3.0F, -1.0F},
                                ScreenPoint{-1.0F, 3.0F},
                                white);
    check_exact_pixels("partially outside triangle",
                       clipped_triangle,
                       {{0, 0}, {1, 0}, {0, 1}},
                       white);

    constexpr tinyrenderer::Color far_color{40, 90, 220};
    constexpr tinyrenderer::Color near_color{235, 110, 60};
    constexpr tinyrenderer::ScreenVertex far0{point0, 0.8F};
    constexpr tinyrenderer::ScreenVertex far1{point1, 0.8F};
    constexpr tinyrenderer::ScreenVertex far2{point2, 0.8F};
    constexpr tinyrenderer::ScreenVertex near0{point0, 0.2F};
    constexpr tinyrenderer::ScreenVertex near1{point1, 0.2F};
    constexpr tinyrenderer::ScreenVertex near2{point2, 0.2F};

    tinyrenderer::Image far_then_near{7, 7};
    tinyrenderer::DepthBuffer far_then_near_depth{7, 7};
    tinyrenderer::draw_triangle_with_depth(far_then_near,
                                           far_then_near_depth,
                                           far0,
                                           far1,
                                           far2,
                                           far_color);
    tinyrenderer::draw_triangle_with_depth(far_then_near,
                                           far_then_near_depth,
                                           near0,
                                           near1,
                                           near2,
                                           near_color);

    tinyrenderer::Image near_then_far{7, 7};
    tinyrenderer::DepthBuffer near_then_far_depth{7, 7};
    tinyrenderer::draw_triangle_with_depth(near_then_far,
                                           near_then_far_depth,
                                           near0,
                                           near1,
                                           near2,
                                           near_color);
    tinyrenderer::draw_triangle_with_depth(near_then_far,
                                           near_then_far_depth,
                                           far0,
                                           far1,
                                           far2,
                                           far_color);
    check_same_image("depth makes draw order irrelevant", far_then_near, near_then_far);
    if (far_then_near.pixel(2, 2) != near_color) {
        std::cerr << "FAILED: nearer triangle owns overlapping pixel\n";
        ++failures;
    }

    tinyrenderer::Image interpolated_image{7, 7};
    tinyrenderer::DepthBuffer interpolated_depth{7, 7};
    tinyrenderer::draw_triangle_with_depth(
        interpolated_image,
        interpolated_depth,
        {point0, 0.2F},
        {point1, 0.6F},
        {point2, 1.0F},
        white);
    check_close("depth uses barycentric interpolation",
                interpolated_depth.depth(1, 1),
                0.35F);

    if (failures == 0) {
        std::cout << "All rasterizer tests passed.\n";
    }
    return failures == 0 ? 0 : 1;
}
