#include "render.hpp"
#include "colors.hpp"
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
    const int width  { static_cast<int>(plane.nx()) };
    const int height { static_cast<int>(plane.ny()) };
    constexpr int channels { 3 };

    // stride = 0 tells stbi to compute it automatically as width * channels.
    // No padding between rows, buffer is tightly packed.
    const int result { stbi_write_png(
        filename.c_str(),
        width, height,
        channels,
        buffer,
        width * channels)
    };

    if (result == 0) {
        throw std::runtime_error{"stbi_write_png: cannot write output file"};
    }
}
