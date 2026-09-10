#include "tinyrenderer/math.hpp"

#include <cmath>

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

} // namespace tinyrenderer
