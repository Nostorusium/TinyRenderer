#pragma once

#include "tinyrenderer/color.hpp"

namespace tinyrenderer {

class Image;

/*
输入线段的两个端点
draw_line 会找出最接近理想直线的像素并写进图片
*/
void draw_line(Image& image, int x0, int y0, int x1, int y1, Color color) noexcept;

} // namespace tinyrenderer
