#include "tinyrenderer/clipper.hpp"

#include <array>
#include <cmath>
#include <vector>

namespace tinyrenderer {
namespace {

enum class ClipPlane {
    left,
    right,
    bottom,
    top,
    near,
    far,
};

float signed_distance(const ClipVertex& vertex, const ClipPlane plane) noexcept
{
    /*
    这些值不是普通空间距离，而是齐次裁剪平面的“内外判断值”。
    大于等于 0 表示顶点在该平面内侧；小于 0 表示应该被裁掉。
    */
    const Vec4 position = vertex.position;
    switch (plane) {
    case ClipPlane::left:
        return position.x + position.w;
    case ClipPlane::right:
        return position.w - position.x;
    case ClipPlane::bottom:
        return position.y + position.w;
    case ClipPlane::top:
        return position.w - position.y;
    case ClipPlane::near:
        return position.z + position.w;
    case ClipPlane::far:
        return position.w - position.z;
    }
    return -1.0F;
}

bool is_finite(const ClipVertex& vertex) noexcept
{
    return std::isfinite(vertex.position.x)
        && std::isfinite(vertex.position.y)
        && std::isfinite(vertex.position.z)
        && std::isfinite(vertex.position.w);
}

ClipVertex interpolate(const ClipVertex& start,
                       const ClipVertex& end,
                       const float amount) noexcept
{
    // 边穿过裁剪平面时，用线性插值找到交点；新顶点仍处于透视除法前的裁剪空间。
    return {{
        start.position.x + (end.position.x - start.position.x) * amount,
        start.position.y + (end.position.y - start.position.y) * amount,
        start.position.z + (end.position.z - start.position.z) * amount,
        start.position.w + (end.position.w - start.position.w) * amount}};
}

std::vector<ClipVertex> clip_polygon_against_plane(
    const std::vector<ClipVertex>& polygon,
    const ClipPlane plane)
{
    std::vector<ClipVertex> output;
    if (polygon.empty()) {
        return output;
    }

    ClipVertex previous = polygon.back();
    float previous_distance = signed_distance(previous, plane);
    bool previous_inside = previous_distance >= 0.0F;

    /*
    Sutherland-Hodgman 逐条检查多边形边：
    两端都在内侧时保留终点；跨越平面时先生成交点；两端都在外侧时不保留。
    */
    for (const ClipVertex current : polygon) {
        const float current_distance = signed_distance(current, plane);
        const bool current_inside = current_distance >= 0.0F;

        if (previous_inside != current_inside) {
            const float amount = previous_distance
                / (previous_distance - current_distance);
            output.push_back(interpolate(previous, current, amount));
        }
        if (current_inside) {
            output.push_back(current);
        }

        previous = current;
        previous_distance = current_distance;
        previous_inside = current_inside;
    }
    return output;
}

} // namespace

std::vector<ClipTriangle> clip_triangle_to_frustum(
    const ClipTriangle& triangle)
{
    for (const ClipVertex vertex : triangle) {
        if (!is_finite(vertex)) {
            return {};
        }
    }

    std::vector<ClipVertex> polygon{triangle.begin(), triangle.end()};
    constexpr std::array planes{
        ClipPlane::left,
        ClipPlane::right,
        ClipPlane::bottom,
        ClipPlane::top,
        ClipPlane::near,
        ClipPlane::far,
    };

    // Sutherland-Hodgman：逐个平面裁剪多边形，保留进入和留在可见区域的边。
    for (const ClipPlane plane : planes) {
        polygon = clip_polygon_against_plane(polygon, plane);
        if (polygon.size() < 3) {
            return {};
        }
    }

    std::vector<ClipTriangle> triangles;
    triangles.reserve(polygon.size() - 2);
    // 裁剪后的结果仍是凸多边形，所以可以固定第一个顶点，用扇形方式重新拆成三角形。
    for (std::size_t index = 1; index + 1 < polygon.size(); ++index) {
        triangles.push_back({polygon[0], polygon[index], polygon[index + 1]});
    }
    return triangles;
}

} // namespace tinyrenderer
