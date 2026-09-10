#pragma once

#include <array>
#include <cstddef>
#include <optional>

namespace tinyrenderer {

// 三维向量用三个分量表示空间中的方向或位移。
// 注意这个命名习惯 你很可能在其他地方也看到这样的取名
struct Vec3 {
    float x{};
    float y{};
    float z{};
};

/*
齐次坐标在三维坐标后增加 w：点通常使用 w = 1，方向使用 w = 0。
这样平移只影响点，不影响方向，之后也能用同一种矩阵表达透视投影。
*/
struct Vec4 {
    float x{};
    float y{};
    float z{};
    float w{};
};

// Mat4即 Matrix4，指4x4矩阵
class Mat4 {
public:
    Mat4() = default;

    // 生成单位矩阵
    [[nodiscard]] static Mat4 identity() noexcept;

    /*
    Model变换：包含平移、缩放、旋转。影响物体在世界空间中的“摆放”

    */
    [[nodiscard]] static Mat4 translation(Vec3 offset) noexcept;
    [[nodiscard]] static Mat4 scaling(Vec3 factors) noexcept;

    // 旋转角使用弧度，并遵守右手坐标系的正方向。
    [[nodiscard]] static Mat4 rotation_x(float radians) noexcept;
    [[nodiscard]] static Mat4 rotation_y(float radians) noexcept;
    [[nodiscard]] static Mat4 rotation_z(float radians) noexcept;

    /*
    邦邦卡邦 C++语法小课堂
    [[nodiscard]] 表示返回值不应该被无意义地丢弃。
    如果读了但没用会产生编译警告，但不影响运行。

    函数后的 const 表示该函数不会修改对象的状态 为此它返回的是复制的值而不是引用
    第二个函数返回 float& 返回一个引用，于是允许了调用者修改矩阵元素
    */

    // 用 (row, column) 访问数学元素，不让算法依赖底层数组排列。
    [[nodiscard]] float operator()(std::size_t row, std::size_t column) const;
    float& operator()(std::size_t row, std::size_t column);

private:
    // row-major 只描述内存布局：同一行的四个元素连续存放。
    std::array<float, 16> elements_{};
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

/*
View 变换把世界坐标转换到摄像机坐标。
摄像机位置与观察目标不能重合，up 也不能和观察方向平行。
View 变换的另一个理解是让包括摄像机在内的所有物体都围绕摄像机位置旋转、平移，使摄像机最终落在原点，朝向 -Z 轴。
所以它只需要取对摄像机的M变换的逆矩阵
*/
[[nodiscard]] std::optional<Mat4> look_at(Vec3 eye,
                                          Vec3 target,
                                          Vec3 up) noexcept;

/*
透视投影先把摄像机空间的视锥体变成齐次裁剪坐标。
之后还要除以 w，才能得到范围为 [-1, 1] 的 NDC 标准立方体。
*/
[[nodiscard]] std::optional<Mat4> perspective(float vertical_fov_radians,
                                              float aspect_ratio,
                                              float near_plane,
                                              float far_plane) noexcept;

// 透视除法会让远处物体变小；w 太接近零时无法安全执行。
[[nodiscard]] std::optional<Vec3> perspective_divide(Vec4 clip_position) noexcept;

// 项目使用列向量，因此变换写作 M * v；平移位于矩阵最后一列。
[[nodiscard]] Vec4 operator*(const Mat4& matrix, Vec4 vector);

// 组合矩阵 left * right 作用于向量时，right 对应的变换先执行。
[[nodiscard]] Mat4 operator*(const Mat4& left, const Mat4& right);

} // namespace tinyrenderer
