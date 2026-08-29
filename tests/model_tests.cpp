#include "tinyrenderer/model.hpp"

#include <iostream>
#include <sstream>
#include <string>

namespace {

int failures = 0;

void check(const bool condition, const char* message)
{
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

bool load_text(const std::string& text,
               tinyrenderer::Model& model,
               std::string& error)
{
    std::istringstream input{text};
    return tinyrenderer::load_obj(input, model, error);
}

} // namespace

int main()
{
    tinyrenderer::Model triangle;
    std::string error;
    check(load_text("# one triangle\n"
                    "v -1 0 0\n"
                    "v 1 0 0\n"
                    "v 0 1 0\n"
                    "f 1 2 3\n",
                    triangle,
                    error),
          "loads a triangle");
    check(triangle.vertices.size() == 3, "stores three vertices");
    check(triangle.faces.size() == 1, "stores one face");
    check(triangle.faces[0].vertex_indices == std::array<std::size_t, 3>{0, 1, 2},
          "converts OBJ indices from one-based to zero-based");

    tinyrenderer::Model slash_indices;
    check(load_text("v 0 0 0\n"
                    "v 1 0 0\n"
                    "v 0 1 0\n"
                    "f 1/4/7 2/5/8 3/6/9\n",
                    slash_indices,
                    error),
          "reads the vertex part of slash indices");

    tinyrenderer::Model quad;
    check(!load_text("v 0 0 0\n"
                     "v 1 0 0\n"
                     "v 1 1 0\n"
                     "v 0 1 0\n"
                     "f 1 2 3 4\n",
                     quad,
                     error),
          "rejects non-triangle faces");

    tinyrenderer::Model bad_index;
    check(!load_text("v 0 0 0\n"
                     "v 1 0 0\n"
                     "v 0 1 0\n"
                     "f 1 2 4\n",
                     bad_index,
                     error),
          "rejects an index outside the vertex list");

    if (failures == 0) {
        std::cout << "All model tests passed.\n";
    }
    return failures == 0 ? 0 : 1;
}
