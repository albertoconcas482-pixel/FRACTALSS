cmake_minimum_required(VERSION 3.16)

project(Fractals VERSION 1.0 LANGUAGES CXX)

add_executable(mandelbrot
    src/main.cpp
    src/fractal_pl.cpp
    src/mandel_set.cpp
    src/mandel_pipeline.cpp
    src/render.cpp
)

target_include_directories(mandelbrot PRIVATE src)

target_compile_features(mandelbrot PRIVATE cxx_std_20)
