#pragma once

#include <cstdint>

namespace tinyrenderer {

/*
颜色只是一个R,G,B构成的三元组
uint8_t 意思是无符号8位整数，范围是0-255
_t意思是type，这种风格的命名来源于早年的C语言标准库
*/

struct Color {
    std::uint8_t red{};
    std::uint8_t green{};
    std::uint8_t blue{};

    // 让编译器逐项比较三个颜色通道。
    bool operator==(const Color&) const = default;
};

} // namespace tinyrenderer
