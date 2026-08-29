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
    /*
    图形学通常把模型表面拆成许多三角形，交给光栅化器处理。
    所以当前项目中，一个 Face 就是由 3 个顶点构成的三角面。
    这里只保存顶点下标，不重复保存顶点坐标。
    OBJ 也允许多边形面，但当前加载器暂不支持。
    */
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
