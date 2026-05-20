#include "render.hpp"
#include "colors.hpp"
#include <filesystem>
#include <stdexcept>

// Compile stb_image_write implementation once in this translation unit
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

std::unique_ptr<unsigned char[]> render::render_color(
    const fractal_pl& plane,
    const std::string& color_type,
    int maxiter)
{
    return color_registry::apply_color(color_type, plane, maxiter);
}

void render::render_to_png(
    const fractal_pl& plane,
    const unsigned char* buffer,
    const std::string& filename)
{
    const std::filesystem::path output_path{filename};

    // Safely generate directory trees only if a parent folder is declared in the target string
    if (output_path.has_parent_path()) {
        std::filesystem::create_directories(output_path.parent_path());
    }

    const int width  { static_cast<int>(plane.nx()) };
    const int height { static_cast<int>(plane.ny()) };
    constexpr int channels { 3 };

    const int result { stbi_write_png(
        output_path.string().c_str(),
        width, height,
        channels,
        buffer,
        width * channels)
    };

    if (result == 0) {
        throw std::runtime_error{"stbi_write_png: failed to write output image: " + output_path.string()};
    }
}
