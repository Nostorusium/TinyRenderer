#include "tinyrenderer/image.hpp"

#include <fstream>
#include <stdexcept>

namespace tinyrenderer {

Image::Image(const int width, const int height, const Color clear_color)
    : width_(width),
      height_(height)
{
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument{"Image dimensions must be positive"};
    }

    const auto pixel_count =
        static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    pixels_.assign(pixel_count, clear_color);
}

int Image::width() const noexcept
{
    return width_;
}

int Image::height() const noexcept
{
    return height_;
}

bool Image::contains(const int x, const int y) const noexcept
{
    return x >= 0 && x < width_ && y >= 0 && y < height_;
}

bool Image::set_pixel(const int x, const int y, const Color color) noexcept
{
    if (!contains(x, y)) {
        return false;
    }

    pixels_[index_of(x, y)] = color;
    return true;
}

Color Image::pixel(const int x, const int y) const
{
    if (!contains(x, y)) {
        throw std::out_of_range{"Pixel coordinates are outside the image"};
    }

    return pixels_[index_of(x, y)];
}

bool Image::write_ppm(const std::filesystem::path& path) const
{
    std::ofstream output{path, std::ios::binary};
    if (!output) {
        return false;
    }

    output << "P6\n" << width_ << ' ' << height_ << "\n255\n";

    for (const Color color : pixels_) {
        output.put(static_cast<char>(color.red));
        output.put(static_cast<char>(color.green));
        output.put(static_cast<char>(color.blue));
    }

    return output.good();
}

std::size_t Image::index_of(const int x, const int y) const noexcept
{
    return static_cast<std::size_t>(y) * static_cast<std::size_t>(width_)
        + static_cast<std::size_t>(x);
}

} // namespace tinyrenderer
