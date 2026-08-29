#include "tinyrenderer/model.hpp"

#include <charconv>
#include <fstream>
#include <sstream>
#include <string_view>
#include <system_error>
#include <utility>

namespace tinyrenderer {
namespace {

bool parse_vertex_index(const std::string_view token,
                        const std::size_t vertex_count,
                        std::size_t& index)
{
    // f 3/2/1 中，第一个数字 3 才是我们现在需要的顶点索引。
    const auto slash = token.find('/');
    const auto number = token.substr(0, slash);

    std::size_t obj_index{};
    const auto [end, status] = std::from_chars(number.data(),
                                               number.data() + number.size(),
                                               obj_index);
    if (status != std::errc{} || end != number.data() + number.size()) {
        return false;
    }
    if (obj_index == 0 || obj_index > vertex_count) {
        return false;
    }

    index = obj_index - 1;
    return true;
}

} // namespace

bool load_obj(std::istream& input, Model& model, std::string& error)
{
    Model parsed_model;
    std::string line;
    std::size_t line_number = 0;

    while (std::getline(input, line)) {
        ++line_number;
        std::istringstream line_stream{line};
        std::string record_type;
        if (!(line_stream >> record_type) || record_type.starts_with('#')) {
            continue;
        }

        if (record_type == "v") {
            Vertex vertex;
            if (!(line_stream >> vertex.x >> vertex.y >> vertex.z)) {
                error = "Invalid vertex at OBJ line " + std::to_string(line_number);
                return false;
            }
            parsed_model.vertices.push_back(vertex);
            continue;
        }

        if (record_type == "f") {
            std::array<std::string, 3> tokens;
            if (!(line_stream >> tokens[0] >> tokens[1] >> tokens[2])) {
                error = "Face is not a triangle at OBJ line " + std::to_string(line_number);
                return false;
            }

            std::string fourth_token;
            if ((line_stream >> fourth_token) && !fourth_token.starts_with('#')) {
                error = "Face is not a triangle at OBJ line " + std::to_string(line_number);
                return false;
            }

            Face face;
            for (std::size_t corner = 0; corner < face.vertex_indices.size(); ++corner) {
                if (!parse_vertex_index(tokens[corner],
                                        parsed_model.vertices.size(),
                                        face.vertex_indices[corner])) {
                    error = "Invalid face index at OBJ line " + std::to_string(line_number);
                    return false;
                }
            }
            parsed_model.faces.push_back(face);
        }
    }

    model = std::move(parsed_model);
    error.clear();
    return true;
}

bool load_obj(const std::filesystem::path& path, Model& model, std::string& error)
{
    std::ifstream input{path};
    if (!input) {
        error = "Could not open OBJ file: " + path.string();
        return false;
    }
    return load_obj(input, model, error);
}

} // namespace tinyrenderer
