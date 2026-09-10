#include "tinyrenderer/depth_buffer.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace tinyrenderer {

DepthBuffer::DepthBuffer(const int width, const int height)
    : width_(width),
      height_(height)
{
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument{"Depth buffer dimensions must be positive"};
    }

    const auto pixel_count =
        static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    depths_.assign(pixel_count, std::numeric_limits<float>::infinity());
}

int DepthBuffer::width() const noexcept
{
    return width_;
}

int DepthBuffer::height() const noexcept
{
    return height_;
}

bool DepthBuffer::contains(const int x, const int y) const noexcept
{
    return x >= 0 && x < width_ && y >= 0 && y < height_;
}

bool DepthBuffer::test_and_set(const int x, const int y, const float depth) noexcept
{
    if (!contains(x, y) || !std::isfinite(depth) || depth < 0.0F || depth > 1.0F) {
        return false;
    }

    float& stored_depth = depths_[index_of(x, y)];
    if (depth >= stored_depth) {
        return false;
    }

    stored_depth = depth;
    return true;
}

float DepthBuffer::depth(const int x, const int y) const
{
    if (!contains(x, y)) {
        throw std::out_of_range{"Depth coordinates are outside the buffer"};
    }
    return depths_[index_of(x, y)];
}

std::size_t DepthBuffer::index_of(const int x, const int y) const noexcept
{
    return static_cast<std::size_t>(y) * static_cast<std::size_t>(width_)
        + static_cast<std::size_t>(x);
}

} // namespace tinyrenderer
