#include "render.hpp"
#include "colors.hpp"
#include <filesystem>
#include <stdexcept>

// stb_image_write implementation must be compiled exactly once,
// in this translation unit only — never in a header.
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

std::unique_ptr<unsigned char[]> render::render_color(
    const fractal_pl& plane,
    const std::string& color_type)
{
    // Delegate entirely to color_registry: it allocates the buffer,
    // fills it with RGB triplets and returns ownership to the caller.
    return color_registry::apply_color(color_type, plane);
}

void render::render_to_png(
    const fractal_pl& plane,
    const unsigned char* buffer,
    const std::string& filename)
{
    // Create the output directory next to the executable if it does
    // not exist yet. std::filesystem::create_directories is a no-op
    // when the directory already exists.
    // NOTE: the path is relative to the working directory at runtime
    // (typically the build folder). A future improvement could make
    // this configurable or relative to the project root.
    const std::filesystem::path output_dir{"images"};
    std::filesystem::create_directories(output_dir);

    const std::filesystem::path output_path{output_dir / filename};

    const int width  { static_cast<int>(plane.nx()) };
    const int height { static_cast<int>(plane.ny()) };
    constexpr int channels { 3 };

    // stride = width * channels: rows are tightly packed, no padding.
    const int result { stbi_write_png(
        output_path.string().c_str(),
        width, height,
        channels,
        buffer,
        width * channels)
    };

    if (result == 0) {
        throw std::runtime_error{"stbi_write_png: cannot write output file: " + output_path.string()};
    }
}
