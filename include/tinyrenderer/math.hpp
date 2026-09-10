#pragma once

#include <optional>

namespace tinyrenderer {

// 三维向量用三个分量表示空间中的方向或位移。
// 注意这个命名习惯 你很可能在其他地方也看到这样的取名
struct Vec3 {
    float x{};
    float y{};
    float z{};
};

[[nodiscard]] Vec3 operator+(Vec3 left, Vec3 right) noexcept;
[[nodiscard]] Vec3 operator-(Vec3 left, Vec3 right) noexcept;
[[nodiscard]] Vec3 operator*(Vec3 vector, float scalar) noexcept;

// 点积衡量两个方向有多一致，之后会用于光照和投影计算。
[[nodiscard]] float dot(Vec3 left, Vec3 right) noexcept;

// 叉积产生同时垂直于两个输入的方向，并遵守项目的右手坐标系。
[[nodiscard]] Vec3 cross(Vec3 left, Vec3 right) noexcept;

[[nodiscard]] float length(Vec3 vector) noexcept;

// 归一化只改变长度、不改变方向；零向量没有方向，因此返回空值。
[[nodiscard]] std::optional<Vec3> normalized(Vec3 vector) noexcept;

} // namespace tinyrenderer
