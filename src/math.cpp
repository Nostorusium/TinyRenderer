#include "tinyrenderer/math.hpp"

#include <cmath>
#include <stdexcept>

namespace tinyrenderer {

Vec3 operator+(const Vec3 left, const Vec3 right) noexcept
{
    return {left.x + right.x, left.y + right.y, left.z + right.z};
}

Vec3 operator-(const Vec3 left, const Vec3 right) noexcept
{
    return {left.x - right.x, left.y - right.y, left.z - right.z};
}

Vec3 operator*(const Vec3 vector, const float scalar) noexcept
{
    return {vector.x * scalar, vector.y * scalar, vector.z * scalar};
}

float dot(const Vec3 left, const Vec3 right) noexcept
{
    return left.x * right.x + left.y * right.y + left.z * right.z;
}

Vec3 cross(const Vec3 left, const Vec3 right) noexcept
{
    return {
        left.y * right.z - left.z * right.y,
        left.z * right.x - left.x * right.z,
        left.x * right.y - left.y * right.x};
}

float length(const Vec3 vector) noexcept
{
    return std::sqrt(dot(vector, vector));
}

std::optional<Vec3> normalized(const Vec3 vector) noexcept
{
    const float vector_length = length(vector);
    constexpr float zero_length_epsilon = 1.0e-6F;
    if (vector_length <= zero_length_epsilon) {
        return std::nullopt;
    }

    const float inverse_length = 1.0F / vector_length;
    return vector * inverse_length;
}

std::optional<Mat4> look_at(const Vec3 eye,
                            const Vec3 target,
                            const Vec3 up) noexcept
{
    // 摄像机朝 -Z 看，所以摄像机自身的 +Z 轴指向观察目标的反方向。
    const auto camera_z = normalized(eye - target);
    if (!camera_z) {
        return std::nullopt;
    }

    const auto camera_x = normalized(cross(up, *camera_z));
    if (!camera_x) {
        return std::nullopt;
    }
    const Vec3 camera_y = cross(*camera_z, *camera_x);

    Mat4 view = Mat4::identity();

    // 前三行分别计算一个世界坐标在摄像机 X、Y、Z 方向上的分量。
    view(0, 0) = camera_x->x;
    view(0, 1) = camera_x->y;
    view(0, 2) = camera_x->z;
    view(1, 0) = camera_y.x;
    view(1, 1) = camera_y.y;
    view(1, 2) = camera_y.z;
    view(2, 0) = camera_z->x;
    view(2, 1) = camera_z->y;
    view(2, 2) = camera_z->z;

    // 这里的负号相当于对摄像机的世界位置取逆：摄像机最终会落在原点。
    view(0, 3) = -dot(*camera_x, eye);
    view(1, 3) = -dot(camera_y, eye);
    view(2, 3) = -dot(*camera_z, eye);
    return view;
}

std::optional<Mat4> perspective(const float vertical_fov_radians,
                                const float aspect_ratio,
                                const float near_plane,
                                const float far_plane) noexcept
{
    constexpr float pi = 3.14159265358979323846F;
    if (!std::isfinite(vertical_fov_radians)
        || !std::isfinite(aspect_ratio)
        || !std::isfinite(near_plane)
        || !std::isfinite(far_plane)
        || vertical_fov_radians <= 0.0F
        || vertical_fov_radians >= pi
        || aspect_ratio <= 0.0F
        || near_plane <= 0.0F
        || far_plane <= near_plane) {
        return std::nullopt;
    }

    const float vertical_scale =
        1.0F / std::tan(vertical_fov_radians * 0.5F);

    Mat4 projection;

    // X 和 Y 先按视野角与宽高比缩放，使视锥体的侧面最终对应 NDC 的 ±1。
    projection(0, 0) = vertical_scale / aspect_ratio;
    projection(1, 1) = vertical_scale;

    // 摄像机朝 -Z 看；这两项让近、远裁剪面在透视除法后映射到 NDC 的 -1 和 +1。
    projection(2, 2) = (far_plane + near_plane) / (near_plane - far_plane);
    projection(2, 3) =
        (2.0F * far_plane * near_plane) / (near_plane - far_plane);

    // clip.w = -camera.z。之后除以 w，距离越远，X 和 Y 就会被除得越小。
    projection(3, 2) = -1.0F;
    return projection;
}

std::optional<Vec3> perspective_divide(const Vec4 clip_position) noexcept
{
    constexpr float zero_w_epsilon = 1.0e-6F;
    if (!std::isfinite(clip_position.x)
        || !std::isfinite(clip_position.y)
        || !std::isfinite(clip_position.z)
        || !std::isfinite(clip_position.w)
        || std::abs(clip_position.w) <= zero_w_epsilon) {
        return std::nullopt;
    }

    const float inverse_w = 1.0F / clip_position.w;
    return Vec3{
        clip_position.x * inverse_w,
        clip_position.y * inverse_w,
        clip_position.z * inverse_w};
}

Mat4 Mat4::identity() noexcept
{
    Mat4 result;
    result.elements_[0] = 1.0F;
    result.elements_[5] = 1.0F;
    result.elements_[10] = 1.0F;
    result.elements_[15] = 1.0F;
    return result;
}

Mat4 Mat4::translation(const Vec3 offset) noexcept
{
    Mat4 result = identity();

    // 列向量约定下，最后一列乘以 w：点的 w = 1，所以会受到平移；方向的 w = 0，所以不会。
    result(0, 3) = offset.x;
    result(1, 3) = offset.y;
    result(2, 3) = offset.z;
    return result;
}

Mat4 Mat4::scaling(const Vec3 factors) noexcept
{
    Mat4 result = identity();
    result(0, 0) = factors.x;
    result(1, 1) = factors.y;
    result(2, 2) = factors.z;
    return result;
}

Mat4 Mat4::rotation_x(const float radians) noexcept
{
    Mat4 result = identity();
    const float cosine = std::cos(radians);
    const float sine = std::sin(radians);

    result(1, 1) = cosine;
    result(1, 2) = -sine;
    result(2, 1) = sine;
    result(2, 2) = cosine;
    return result;
}

Mat4 Mat4::rotation_y(const float radians) noexcept
{
    Mat4 result = identity();
    const float cosine = std::cos(radians);
    const float sine = std::sin(radians);

    result(0, 0) = cosine;
    result(0, 2) = sine;
    result(2, 0) = -sine;
    result(2, 2) = cosine;
    return result;
}

Mat4 Mat4::rotation_z(const float radians) noexcept
{
    Mat4 result = identity();
    const float cosine = std::cos(radians);
    const float sine = std::sin(radians);

    result(0, 0) = cosine;
    result(0, 1) = -sine;
    result(1, 0) = sine;
    result(1, 1) = cosine;
    return result;
}

float Mat4::operator()(const std::size_t row, const std::size_t column) const
{
    if (row >= 4 || column >= 4) {
        throw std::out_of_range{"Matrix coordinates are outside 4x4 bounds"};
    }
    return elements_[row * 4 + column];
}

float& Mat4::operator()(const std::size_t row, const std::size_t column)
{
    if (row >= 4 || column >= 4) {
        throw std::out_of_range{"Matrix coordinates are outside 4x4 bounds"};
    }
    return elements_[row * 4 + column];
}

Vec4 operator*(const Mat4& matrix, const Vec4 vector)
{
    // 结果的每个分量，都是矩阵对应行与列向量的点积。
    return {
        matrix(0, 0) * vector.x + matrix(0, 1) * vector.y
            + matrix(0, 2) * vector.z + matrix(0, 3) * vector.w,
        matrix(1, 0) * vector.x + matrix(1, 1) * vector.y
            + matrix(1, 2) * vector.z + matrix(1, 3) * vector.w,
        matrix(2, 0) * vector.x + matrix(2, 1) * vector.y
            + matrix(2, 2) * vector.z + matrix(2, 3) * vector.w,
        matrix(3, 0) * vector.x + matrix(3, 1) * vector.y
            + matrix(3, 2) * vector.z + matrix(3, 3) * vector.w};
}

Mat4 operator*(const Mat4& left, const Mat4& right)
{
    Mat4 result;
    for (std::size_t row = 0; row < 4; ++row) {
        for (std::size_t column = 0; column < 4; ++column) {
            float value = 0.0F;
            // 左矩阵的一行乘右矩阵的一列，得到结果中的一个元素。
            for (std::size_t inner = 0; inner < 4; ++inner) {
                value += left(row, inner) * right(inner, column);
            }
            result(row, column) = value;
        }
    }
    return result;
}

} // namespace tinyrenderer
