#include "tinyrenderer/rasterizer.hpp"

#include "tinyrenderer/depth_buffer.hpp"
#include "tinyrenderer/image.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace tinyrenderer {
namespace {

struct PixelBounds {
    int min_x{};
    int max_x{};
    int min_y{};
    int max_y{};
};

/*
top-left rule 是光栅化阶段的共享边规则。
假如一个像素落在两个三角形的共享边上，到底归谁？
为了避免两个三角形重复覆盖或同时漏掉，我们规定：
位于三角形上边或左边的边界采样点算内部。下边或右边不算。
*/

// 判断点在一条有向边的哪一侧，>0 在左侧，<0 在右侧，=0 在边上。
float edge_function(const ScreenPoint start,
                    const ScreenPoint end,
                    const ScreenPoint sample) noexcept
{
    return (end.x - start.x) * (sample.y - start.y)
        - (end.y - start.y) * (sample.x - start.x);
}

bool is_top_left_edge(const ScreenPoint start, const ScreenPoint end) noexcept
{
    const float delta_x = end.x - start.x;
    const float delta_y = end.y - start.y;

    // 图片 Y 向下：向上的边是左边，水平且向右的边是上边。
    return delta_y < 0.0F || (delta_y == 0.0F && delta_x > 0.0F);
}

bool edge_contains_sample(const ScreenPoint start,
                          const ScreenPoint end,
                          const ScreenPoint sample) noexcept
{
    const float edge_value = edge_function(start, end, sample);
    // 这个边界误差主要是为了浮点数运算产生的精度误差考虑，并不是设计上希望容忍误差。
    constexpr float edge_epsilon = 1.0e-6F;
    if (edge_value > edge_epsilon) {
        return true;
    }
    if (edge_value < -edge_epsilon) {
        return false;
    }
    // 认为可以 = 0

    // 共享边附近只由 top-left 一侧接收，避免两个三角形重复覆盖或同时漏掉。
    return is_top_left_edge(start, end);
}

// 向量叉乘判别法
bool triangle_contains_sample(const ScreenPoint point0,
                              const ScreenPoint point1,
                              const ScreenPoint point2,
                              const ScreenPoint sample) noexcept
{
    return edge_contains_sample(point0, point1, sample)
        && edge_contains_sample(point1, point2, sample)
        && edge_contains_sample(point2, point0, sample);
}

// 包围盒切割
std::optional<PixelBounds> clipped_pixel_bounds(const Image& image,
                                                const ScreenPoint point0,
                                                const ScreenPoint point1,
                                                const ScreenPoint point2) noexcept
{
    const float triangle_min_x = std::min({point0.x, point1.x, point2.x});
    const float triangle_max_x = std::max({point0.x, point1.x, point2.x});
    const float triangle_min_y = std::min({point0.y, point1.y, point2.y});
    const float triangle_max_y = std::max({point0.y, point1.y, point2.y});

    const float first_sample_x = std::max(triangle_min_x, 0.5F);
    const float last_sample_x =
        std::min(triangle_max_x, static_cast<float>(image.width()) - 0.5F);
    const float first_sample_y = std::max(triangle_min_y, 0.5F);
    const float last_sample_y =
        std::min(triangle_max_y, static_cast<float>(image.height()) - 0.5F);
    if (first_sample_x > last_sample_x || first_sample_y > last_sample_y) {
        return std::nullopt;
    }

    return PixelBounds{
        static_cast<int>(std::ceil(first_sample_x - 0.5F)),
        static_cast<int>(std::floor(last_sample_x - 0.5F)),
        static_cast<int>(std::ceil(first_sample_y - 0.5F)),
        static_cast<int>(std::floor(last_sample_y - 0.5F))};
}

} // namespace

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
                   ScreenPoint point0,
                   ScreenPoint point1,
                   ScreenPoint point2,
                   const Color color) noexcept
{
    constexpr float degenerate_epsilon = 1.0e-6F;
    const float signed_area = edge_function(point0, point1, point2);
    if (std::abs(signed_area) <= degenerate_epsilon) {
        return;
    }
    if (signed_area < 0.0F) {
        std::swap(point1, point2);
    }

    // 先把包围盒限制到图片内，避免遍历一定会被丢弃的像素。
    const auto bounds = clipped_pixel_bounds(image, point0, point1, point2);
    if (!bounds) {
        return;
    }

    for (int y = bounds->min_y; y <= bounds->max_y; ++y) {
        for (int x = bounds->min_x; x <= bounds->max_x; ++x) {
            // 覆盖判断使用像素中心，而不是像素左上角。
            const ScreenPoint sample{static_cast<float>(x) + 0.5F,
                                     static_cast<float>(y) + 0.5F};
            if (triangle_contains_sample(point0, point1, point2, sample)) {
                image.set_pixel(x, y, color);
            }
        }
    }
}

/*
深度测试为每个像素保留最靠近摄像机的片段，避免绘制顺序决定遮挡结果。
像素深度由三个顶点深度按重心权重插值得到，权重越大，对应顶点影响越大。
*/
void draw_triangle_with_depth(Image& image,
                              DepthBuffer& depth_buffer,
                              ScreenVertex vertex0,
                              ScreenVertex vertex1,
                              ScreenVertex vertex2,
                              const Color color) noexcept
{
    if (image.width() != depth_buffer.width() || image.height() != depth_buffer.height()) {
        return;
    }

    constexpr float degenerate_epsilon = 1.0e-6F;
    const float signed_area =
        edge_function(vertex0.position, vertex1.position, vertex2.position);
    if (std::abs(signed_area) <= degenerate_epsilon) {
        return;
    }
    if (signed_area < 0.0F) {
        std::swap(vertex1, vertex2);
    }

    const auto bounds = clipped_pixel_bounds(image,
                                             vertex0.position,
                                             vertex1.position,
                                             vertex2.position);
    if (!bounds) {
        return;
    }

    for (int y = bounds->min_y; y <= bounds->max_y; ++y) {
        for (int x = bounds->min_x; x <= bounds->max_x; ++x) {
            // 光栅化采样点，对应像素中心
            const ScreenPoint sample{static_cast<float>(x) + 0.5F,
                                     static_cast<float>(y) + 0.5F};
            // 如果不符合统一的 top-left 边界规则，说明在三角形外或属于共享边另一侧，不画。
            if (!triangle_contains_sample(vertex0.position,
                                          vertex1.position,
                                          vertex2.position,
                                          sample)) {
                continue;
            }

            // 计算出重心坐标，对应3个权重
            const auto weights = barycentric_at(vertex0.position,
                                                vertex1.position,
                                                vertex2.position,
                                                sample);
            if (!weights) {
                continue;
            }

            // 我们不直接计算每个内部点的深度，而是利用重心坐标的线性插值性质，由3个顶点的深度和重心坐标计算出该点的深度。            
            const float depth = weights->weight0 * vertex0.depth
                + weights->weight1 * vertex1.depth
                + weights->weight2 * vertex2.depth;
            if (depth_buffer.test_and_set(x, y, depth)) {
                image.set_pixel(x, y, color);
            }
        }
    }
}

} // namespace tinyrenderer
