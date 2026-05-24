cmake_minimum_required(VERSION 3.16)

project(Fractals VERSION 1.0 LANGUAGES CXX)

# 1. Find OpenMP package for multithreading
find_package(OpenMP REQUIRED)

add_executable(mandelbrot
    src/main.cpp
    src/fractal_pl.cpp
    src/mandel_set.cpp
    src/mandel_pipeline.cpp
    src/render.cpp
)

target_include_directories(mandelbrot PRIVATE src)

# Force C++20 standard
target_compile_features(mandelbrot PRIVATE cxx_std_20)

# 2. THIS IS THE MISSING LINE: Link OpenMP to the executable target
target_link_libraries(mandelbrot PRIVATE OpenMP::OpenMP_CXX)

# 3. Enable aggressive CPU-specific optimizations for GCC and Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # -march=native unlocks specific hardware instructions of your CPU (like AVX)
    # -O3 maximizes speed optimizations
    target_compile_options(mandelbrot PRIVATE -march=native -O3 -ffast-math)
endif()
