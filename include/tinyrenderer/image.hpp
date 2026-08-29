#pragma once

#include "tinyrenderer/color.hpp"

#include <filesystem>
#include <vector>

namespace tinyrenderer {


/*
二维图片是二维网格(x,y)，但计算机内存本质上是一维连续的空间
所以 pixels_ 是长宽展开后的的一维数组 用来实际存储像素数据

write_ppm用来把图片保存为PPM格式的文件 持久化存储
*/
class Image {
public:
    Image(int width, int height, Color clear_color = {});

    [[nodiscard]] int width() const noexcept;
    [[nodiscard]] int height() const noexcept;
    [[nodiscard]] bool contains(int x, int y) const noexcept;

    // 越界时返回 false，不访问像素数组。
    bool set_pixel(int x, int y, Color color) noexcept;
    [[nodiscard]] Color pixel(int x, int y) const;

    [[nodiscard]] bool write_ppm(const std::filesystem::path& path) const;

private:
    // 按行展开二维坐标：(x, y) -> y * width + x。
    [[nodiscard]] std::size_t index_of(int x, int y) const noexcept;

    int width_{};
    int height_{};
    std::vector<Color> pixels_;
};

} // namespace tinyrenderer
