#include "tinyrenderer/depth_buffer.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {

int failures = 0;

void check(const bool condition, const char* message)
{
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

} // namespace

int main()
{
    tinyrenderer::DepthBuffer depth_buffer{3, 2};
    check(depth_buffer.width() == 3, "width is retained");
    check(depth_buffer.height() == 2, "height is retained");
    check(std::isinf(depth_buffer.depth(1, 1)), "depth starts at positive infinity");

    check(depth_buffer.test_and_set(1, 1, 0.7F), "first finite depth passes");
    check(!depth_buffer.test_and_set(1, 1, 0.8F), "farther depth fails");
    check(depth_buffer.test_and_set(1, 1, 0.2F), "nearer depth passes");
    check(depth_buffer.depth(1, 1) == 0.2F, "nearest depth is retained");
    check(!depth_buffer.test_and_set(1, 1, 0.2F), "equal depth fails strict test");

    check(!depth_buffer.test_and_set(-1, 0, 0.5F), "out-of-bounds depth is rejected");
    check(!depth_buffer.test_and_set(0, 0, -0.1F), "depth below zero is rejected");
    check(!depth_buffer.test_and_set(0, 0, 1.1F), "depth above one is rejected");
    check(!depth_buffer.test_and_set(0,
                                     0,
                                     std::numeric_limits<float>::quiet_NaN()),
          "NaN depth is rejected");

    bool rejected_bad_dimensions = false;
    try {
        const tinyrenderer::DepthBuffer invalid{0, 2};
    } catch (const std::invalid_argument&) {
        rejected_bad_dimensions = true;
    }
    check(rejected_bad_dimensions, "non-positive dimensions are rejected");

    if (failures == 0) {
        std::cout << "All depth buffer tests passed.\n";
    }
    return failures == 0 ? 0 : 1;
}
