cmake_minimum_required(VERSION 3.25)

project(TinyRenderer LANGUAGES CXX)

add_library(tinyrenderer_core
    src/image.cpp
    src/model.cpp
    src/rasterizer.cpp
)

target_include_directories(tinyrenderer_core PUBLIC include)
target_compile_features(tinyrenderer_core PUBLIC cxx_std_20)

add_executable(tiny_renderer
    src/main.cpp
)

target_link_libraries(tiny_renderer PRIVATE tinyrenderer_core)

if(MSVC)
    target_compile_options(tinyrenderer_core PUBLIC /utf-8)
    target_compile_options(tinyrenderer_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(tinyrenderer_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)

if(BUILD_TESTING)
    add_executable(image_tests tests/image_tests.cpp)
    target_link_libraries(image_tests PRIVATE tinyrenderer_core)
    add_test(NAME image_tests COMMAND image_tests)

    add_executable(rasterizer_tests tests/rasterizer_tests.cpp)
    target_link_libraries(rasterizer_tests PRIVATE tinyrenderer_core)
    add_test(NAME rasterizer_tests COMMAND rasterizer_tests)

    add_executable(model_tests tests/model_tests.cpp)
    target_link_libraries(model_tests PRIVATE tinyrenderer_core)
    add_test(NAME model_tests COMMAND model_tests)
endif()
