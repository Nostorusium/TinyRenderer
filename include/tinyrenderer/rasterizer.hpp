#pragma once

#include "tinyrenderer/color.hpp"

#include <optional>

namespace tinyrenderer {

class Image;

struct ScreenPoint {
    float x{};
    float y{};
};

struct BarycentricCoordinates {
    float weight0{};
    float weight1{};
    float weight2{};
};

/*
输入线段的两个端点
draw_line 会找出最接近理想直线的像素并写进图片
*/
void draw_line(Image& image, int x0, int y0, int x1, int y1, Color color) noexcept;

// 返回 sample 相对三个顶点的权重；退化三角形没有面积，因此返回空值。
[[nodiscard]] std::optional<BarycentricCoordinates> barycentric_at(
    ScreenPoint point0,
    ScreenPoint point1,
    ScreenPoint point2,
    ScreenPoint sample) noexcept;

void draw_triangle(Image& image,
                   ScreenPoint point0,
                   ScreenPoint point1,
                   ScreenPoint point2,
                   Color color) noexcept;

} // namespace tinyrenderer
