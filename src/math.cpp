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

Mat4 Mat4::identity() noexcept
{
    Mat4 result;
    result.elements_[0] = 1.0F;
    result.elements_[5] = 1.0F;
    result.elements_[10] = 1.0F;
    result.elements_[15] = 1.0F;
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
