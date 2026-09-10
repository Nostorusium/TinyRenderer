#pragma once

#include <cstddef>
#include <vector>

namespace tinyrenderer {

/*
z-buffer的思想是 对屏幕上的每个像素，记住目前见过的、距离摄像机最近的深度。如果一个新的像素里的比最近像素远，那就不画了。
深度缓冲为每个像素记录目前看见的最近深度。本项目约定深度位于 [0, 1]，数值越小越靠近摄像机。
*/
class DepthBuffer {
public:
    DepthBuffer(int width, int height);

    [[nodiscard]] int width() const noexcept;
    [[nodiscard]] int height() const noexcept;
    [[nodiscard]] bool contains(int x, int y) const noexcept;

    // 设置深度 只有新深度更小时才保存并返回 true
    // 越界或无效深度返回 false。
    bool test_and_set(int x, int y, float depth) noexcept;
    [[nodiscard]] float depth(int x, int y) const;

private:
    [[nodiscard]] std::size_t index_of(int x, int y) const noexcept;

    int width_{};
    int height_{};
    std::vector<float> depths_;
};

} // namespace tinyrenderer
