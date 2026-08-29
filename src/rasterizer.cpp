#include "tinyrenderer/rasterizer.hpp"

#include "tinyrenderer/image.hpp"

#include <algorithm>
#include <cstdlib>

namespace tinyrenderer {

void draw_line(Image& image,
               int x0,
               int y0,
               int x1,
               int y1,
               const Color color) noexcept
{
    // 固定遍历方向，所以交换两个端点后画出的像素仍然相同。
    if (x0 > x1 || (x0 == x1 && y0 > y1)) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    const auto dx = std::abs(static_cast<long long>(x1) - x0);
    const int step_x = x0 < x1 ? 1 : -1;
    const auto dy = -std::abs(static_cast<long long>(y1) - y0);
    const int step_y = y0 < y1 ? 1 : -1;
    auto error = dx + dy;

    /*
    这里使用 Bresenham 直线算法的全方向整数版本。
    它用 error 累计像素对理想直线的偏差，据此决定下一步移动 x、y 或两者。
    doubled_error = 2 * error 避开了 0.5 阈值，所以循环只需整数加减和比较。
    */
    while (true) {
        image.set_pixel(x0, y0, color);

        if (x0 == x1 && y0 == y1) {
            break;
        }

        const auto doubled_error = 2 * error;
        if (doubled_error >= dy) {
            error += dy;
            x0 += step_x;
        }
        if (doubled_error <= dx) {
            error += dx;
            y0 += step_y;
        }
    }
}

} // namespace tinyrenderer
