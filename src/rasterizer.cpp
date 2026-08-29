#include "tinyrenderer/rasterizer.hpp"

#include "tinyrenderer/image.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace tinyrenderer {

void draw_line(Image& image,
               int x0,
               int y0,
               int x1,
               int y1,
               const Color color) noexcept
{
    // 固定遍历方向，所以交换两个端点后画出的像素仍然相同。
    if (x0 > x1 || (x0 == x1 && y0 > y1)) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    const auto dx = std::abs(static_cast<long long>(x1) - x0);
    const int step_x = x0 < x1 ? 1 : -1;
    const auto dy = -std::abs(static_cast<long long>(y1) - y0);
    const int step_y = y0 < y1 ? 1 : -1;
    auto error = dx + dy;

    /*
    这里使用 Bresenham 直线算法的全方向整数版本。
    它用 error 累计像素对理想直线的偏差，据此决定下一步移动 x、y 或两者。
    doubled_error = 2 * error 避开了 0.5 阈值，所以循环只需整数加减和比较。
    */
    while (true) {
        image.set_pixel(x0, y0, color);

        if (x0 == x1 && y0 == y1) {
            break;
        }

        const auto doubled_error = 2 * error;
        if (doubled_error >= dy) {
            error += dy;
            x0 += step_x;
        }
        if (doubled_error <= dx) {
            error += dx;
            y0 += step_y;
        }
    }
}

/*
重心坐标用三个顶点的权重表示平面上的点；权重全不小于 0 时，点在三角形内。
当前用它判断像素是否被三角形覆盖，之后还会用同一组权重插值深度、颜色和 UV。
*/
std::optional<BarycentricCoordinates> barycentric_at(
    const ScreenPoint point0,
    const ScreenPoint point1,
    const ScreenPoint point2,
    const ScreenPoint sample) noexcept
{
    // 分母与三角形的带方向面积成正比；接近 0 表示三个顶点几乎在同一直线上。
    const float denominator =
        (point1.y - point2.y) * (point0.x - point2.x)
        + (point2.x - point1.x) * (point0.y - point2.y);
    constexpr float degenerate_epsilon = 1.0e-6F;
    if (std::abs(denominator) <= degenerate_epsilon) {
        return std::nullopt;
    }

    const float weight0 =
        ((point1.y - point2.y) * (sample.x - point2.x)
         + (point2.x - point1.x) * (sample.y - point2.y))
        / denominator;
    const float weight1 =
        ((point2.y - point0.y) * (sample.x - point2.x)
         + (point0.x - point2.x) * (sample.y - point2.y))
        / denominator;

    // 三个权重的和恒为 1，所以最后一个可以由前两个得到。
    return BarycentricCoordinates{weight0, weight1, 1.0F - weight0 - weight1};
}

/*
这是“包围盒扫描 + 重心坐标”的三角形光栅化方法。
包围盒缩小候选像素范围，重心坐标再判断每个像素中心是否在三角形内。
*/
void draw_triangle(Image& image,
                   const ScreenPoint point0,
                   const ScreenPoint point1,
                   const ScreenPoint point2,
                   const Color color) noexcept
{
    if (!barycentric_at(point0, point1, point2, point0)) {
        return;
    }

    const float triangle_min_x = std::min({point0.x, point1.x, point2.x});
    const float triangle_max_x = std::max({point0.x, point1.x, point2.x});
    const float triangle_min_y = std::min({point0.y, point1.y, point2.y});
    const float triangle_max_y = std::max({point0.y, point1.y, point2.y});

    // 先把包围盒限制到图片内，避免遍历一定会被丢弃的像素。
    const float first_sample_x = std::max(triangle_min_x, 0.5F);
    const float last_sample_x =
        std::min(triangle_max_x, static_cast<float>(image.width()) - 0.5F);
    const float first_sample_y = std::max(triangle_min_y, 0.5F);
    const float last_sample_y =
        std::min(triangle_max_y, static_cast<float>(image.height()) - 0.5F);
    if (first_sample_x > last_sample_x || first_sample_y > last_sample_y) {
        return;
    }

    const int min_x = static_cast<int>(std::ceil(first_sample_x - 0.5F));
    const int max_x = static_cast<int>(std::floor(last_sample_x - 0.5F));
    const int min_y = static_cast<int>(std::ceil(first_sample_y - 0.5F));
    const int max_y = static_cast<int>(std::floor(last_sample_y - 0.5F));
    
    // 这个边界误差主要是为了浮点数运算产生的精度误差考虑，并不是设计上希望容忍误差。
    constexpr float edge_epsilon = 1.0e-6F;
    for (int y = min_y; y <= max_y; ++y) {
        for (int x = min_x; x <= max_x; ++x) {
            // 覆盖判断使用像素中心，而不是像素左上角。
            const ScreenPoint sample{static_cast<float>(x) + 0.5F,
                                     static_cast<float>(y) + 0.5F};
            const auto weights = barycentric_at(point0, point1, point2, sample);
            if (weights && weights->weight0 >= -edge_epsilon
                && weights->weight1 >= -edge_epsilon
                && weights->weight2 >= -edge_epsilon) {
                image.set_pixel(x, y, color);
            }
        }
    }
}

} // namespace tinyrenderer
