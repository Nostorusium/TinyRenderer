#include "tinyrenderer/math.hpp"

#include <cmath>
#include <iostream>

namespace {

int failures = 0;

void check_close(const float actual, const float expected, const char* message)
{
    if (std::abs(actual - expected) > 1.0e-5F) {
        std::cerr << "FAILED: " << message << ", expected " << expected
                  << " but got " << actual << '\n';
        ++failures;
    }
}

void check_vector(const tinyrenderer::Vec3 actual,
                  const tinyrenderer::Vec3 expected,
                  const char* message)
{
    if (std::abs(actual.x - expected.x) > 1.0e-5F
        || std::abs(actual.y - expected.y) > 1.0e-5F
        || std::abs(actual.z - expected.z) > 1.0e-5F) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

void check_vector(const tinyrenderer::Vec4 actual,
                  const tinyrenderer::Vec4 expected,
                  const char* message)
{
    if (std::abs(actual.x - expected.x) > 1.0e-5F
        || std::abs(actual.y - expected.y) > 1.0e-5F
        || std::abs(actual.z - expected.z) > 1.0e-5F
        || std::abs(actual.w - expected.w) > 1.0e-5F) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

} // namespace

int main()
{
    using tinyrenderer::Vec3;

    check_vector(Vec3{1.0F, 2.0F, 3.0F} + Vec3{4.0F, 5.0F, 6.0F},
                 Vec3{5.0F, 7.0F, 9.0F},
                 "vector addition combines matching components");
    check_vector(Vec3{4.0F, 5.0F, 6.0F} - Vec3{1.0F, 2.0F, 3.0F},
                 Vec3{3.0F, 3.0F, 3.0F},
                 "vector subtraction combines matching components");
    check_vector(Vec3{1.0F, -2.0F, 3.0F} * 2.0F,
                 Vec3{2.0F, -4.0F, 6.0F},
                 "scalar multiplication changes vector length");

    check_close(tinyrenderer::dot(Vec3{1.0F, 2.0F, 3.0F},
                                  Vec3{4.0F, -5.0F, 6.0F}),
                12.0F,
                "dot product combines component products");
    check_vector(tinyrenderer::cross(Vec3{1.0F, 0.0F, 0.0F},
                                     Vec3{0.0F, 1.0F, 0.0F}),
                 Vec3{0.0F, 0.0F, 1.0F},
                 "X cross Y equals Z in the right-handed system");

    check_close(tinyrenderer::length(Vec3{3.0F, 4.0F, 0.0F}),
                5.0F,
                "vector length uses the Pythagorean theorem");
    const auto unit_vector = tinyrenderer::normalized(Vec3{3.0F, 4.0F, 0.0F});
    if (unit_vector) {
        check_vector(*unit_vector,
                     Vec3{0.6F, 0.8F, 0.0F},
                     "normalization preserves direction and makes length one");
        check_close(tinyrenderer::length(*unit_vector),
                    1.0F,
                    "normalized vector has unit length");
    } else {
        std::cerr << "FAILED: non-zero vector can be normalized\n";
        ++failures;
    }

    if (tinyrenderer::normalized(Vec3{})) {
        std::cerr << "FAILED: zero vector has no direction to normalize\n";
        ++failures;
    }

    using tinyrenderer::Mat4;
    using tinyrenderer::Vec4;

    const Vec4 point{1.0F, 2.0F, 3.0F, 1.0F};
    check_vector(Mat4::identity() * point,
                 point,
                 "identity matrix leaves a vector unchanged");

    Mat4 translation = Mat4::identity();
    // 列向量约定下，平移量位于最后一列。
    translation(0, 3) = 5.0F;
    translation(1, 3) = -2.0F;
    translation(2, 3) = 3.0F;
    check_vector(translation * point,
                 Vec4{6.0F, 0.0F, 6.0F, 1.0F},
                 "translation moves a point whose w is one");
    check_vector(translation * Vec4{1.0F, 2.0F, 3.0F, 0.0F},
                 Vec4{1.0F, 2.0F, 3.0F, 0.0F},
                 "translation does not move a direction whose w is zero");

    Mat4 scale = Mat4::identity();
    scale(0, 0) = 2.0F;
    scale(1, 1) = 3.0F;
    scale(2, 2) = 4.0F;
    const Mat4 scale_then_translate = translation * scale;
    check_vector(scale_then_translate * point,
                 Vec4{7.0F, 4.0F, 15.0F, 1.0F},
                 "rightmost scale runs before leftmost translation");
    check_vector(scale_then_translate * point,
                 translation * (scale * point),
                 "combined matrix matches separate transformations");

    if (failures == 0) {
        std::cout << "All math tests passed.\n";
    }
    return failures == 0 ? 0 : 1;
}
