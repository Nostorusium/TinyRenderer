#pragma once

#include <array>
#include <cstddef>
#include <filesystem>
#include <istream>
#include <string>
#include <vector>

namespace tinyrenderer {

struct Vertex {
    float x{};
    float y{};
    float z{};
};

struct Face {
    // OBJ 从 1 开始编号；读入后统一改成 C++ 的 0 起始下标。
    std::array<std::size_t, 3> vertex_indices{};
};

struct Model {
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
};

// 当前只读取顶点和三角形面，不尝试实现完整 OBJ 标准。
[[nodiscard]] bool load_obj(std::istream& input, Model& model, std::string& error);
[[nodiscard]] bool load_obj(const std::filesystem::path& path,
                            Model& model,
                            std::string& error);

} // namespace tinyrenderer
