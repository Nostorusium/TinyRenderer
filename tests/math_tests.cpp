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

    const Mat4 constructed_translation = Mat4::translation({5.0F, -2.0F, 3.0F});
    check_vector(constructed_translation * point,
                 Vec4{6.0F, 0.0F, 6.0F, 1.0F},
                 "translation constructor moves a point");
    check_vector(constructed_translation * Vec4{1.0F, 2.0F, 3.0F, 0.0F},
                 Vec4{1.0F, 2.0F, 3.0F, 0.0F},
                 "translation constructor leaves a direction unchanged");

    const Mat4 constructed_scale = Mat4::scaling({2.0F, 3.0F, 4.0F});
    check_vector(constructed_scale * point,
                 Vec4{2.0F, 6.0F, 12.0F, 1.0F},
                 "scaling constructor changes each spatial axis");

    constexpr float half_turn = 3.14159265358979323846F;
    constexpr float quarter_turn = half_turn / 2.0F;
    check_vector(Mat4::rotation_x(quarter_turn) * Vec4{0.0F, 1.0F, 0.0F, 0.0F},
                 Vec4{0.0F, 0.0F, 1.0F, 0.0F},
                 "positive X rotation moves Y toward Z");
    check_vector(Mat4::rotation_y(quarter_turn) * Vec4{0.0F, 0.0F, 1.0F, 0.0F},
                 Vec4{1.0F, 0.0F, 0.0F, 0.0F},
                 "positive Y rotation moves Z toward X");
    check_vector(Mat4::rotation_z(quarter_turn) * Vec4{1.0F, 0.0F, 0.0F, 0.0F},
                 Vec4{0.0F, 1.0F, 0.0F, 0.0F},
                 "positive Z rotation moves X toward Y");

    const Mat4 model = Mat4::translation({10.0F, 0.0F, 0.0F})
        * Mat4::rotation_z(quarter_turn)
        * Mat4::scaling({2.0F, 2.0F, 2.0F});
    check_vector(model * Vec4{1.0F, 0.0F, 0.0F, 1.0F},
                 Vec4{10.0F, 2.0F, 0.0F, 1.0F},
                 "model composition applies scale then rotation then translation");

    const Vec3 camera_position{2.0F, 3.0F, 4.0F};
    const Vec3 camera_target{2.0F, 3.0F, 3.0F};
    const auto view = tinyrenderer::look_at(camera_position,
                                            camera_target,
                                            Vec3{0.0F, 1.0F, 0.0F});
    if (view) {
        check_vector(*view * Vec4{2.0F, 3.0F, 4.0F, 1.0F},
                     Vec4{0.0F, 0.0F, 0.0F, 1.0F},
                     "view moves the camera position to the origin");
        check_vector(*view * Vec4{2.0F, 3.0F, 3.0F, 1.0F},
                     Vec4{0.0F, 0.0F, -1.0F, 1.0F},
                     "view places the target on the camera negative Z axis");
        check_vector(*view * Vec4{1.0F, 0.0F, 0.0F, 0.0F},
                     Vec4{1.0F, 0.0F, 0.0F, 0.0F},
                     "camera translation does not affect a direction");

        const Mat4 placed_model = Mat4::translation({1.0F, 0.0F, 0.0F});
        check_vector((*view * placed_model) * Vec4{0.0F, 0.0F, 0.0F, 1.0F},
                     Vec4{-1.0F, -3.0F, -4.0F, 1.0F},
                     "view runs after model when transforming a local point");
    } else {
        std::cerr << "FAILED: valid camera creates a view matrix\n";
        ++failures;
    }

    const auto turned_view = tinyrenderer::look_at(Vec3{},
                                                   Vec3{1.0F, 0.0F, 0.0F},
                                                   Vec3{0.0F, 1.0F, 0.0F});
    if (turned_view) {
        check_vector(*turned_view * Vec4{1.0F, 0.0F, 0.0F, 1.0F},
                     Vec4{0.0F, 0.0F, -1.0F, 1.0F},
                     "turned camera still sees its target along negative Z");
    } else {
        std::cerr << "FAILED: turned camera creates a view matrix\n";
        ++failures;
    }

    if (tinyrenderer::look_at(Vec3{}, Vec3{}, Vec3{0.0F, 1.0F, 0.0F})) {
        std::cerr << "FAILED: camera position and target cannot coincide\n";
        ++failures;
    }
    if (tinyrenderer::look_at(Vec3{},
                              Vec3{0.0F, 0.0F, -1.0F},
                              Vec3{0.0F, 0.0F, -1.0F})) {
        std::cerr << "FAILED: camera up cannot be parallel to its view direction\n";
        ++failures;
    }

    if (failures == 0) {
        std::cout << "All math tests passed.\n";
    }
    return failures == 0 ? 0 : 1;
}
