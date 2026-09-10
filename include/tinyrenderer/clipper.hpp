#pragma once

#include "tinyrenderer/math.hpp"

#include <array>
#include <vector>

namespace tinyrenderer {

// 裁剪发生在透视除法之前；此时顶点仍保留齐次裁剪坐标和 w。
struct ClipVertex {
    Vec4 position;
};

using ClipTriangle = std::array<ClipVertex, 3>;

/*
这里使用 Sutherland-Hodgman 多边形裁剪算法：
让三角形依次通过视锥体的六个平面，每次切掉当前平面外的部分。
最终把结果限制到 -w <= x、y、z <= w；部分可见时会生成边界顶点，
并把保留下来的多边形重新拆成零个或多个三角形。
*/
[[nodiscard]] std::vector<ClipTriangle> clip_triangle_to_frustum(
    const ClipTriangle& triangle);

} // namespace tinyrenderer
