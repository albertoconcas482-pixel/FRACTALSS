#pragma once
#include <memory>
#include <string>
#include "fractal_pl.hpp"

/*
 * Rendering module.
 *
 * Responsibilities:
 *   - render_color: reads the plane, delegates to color_registry,
 *                   and returns the RGB buffer as a unique_ptr.
 *                   The caller owns the buffer.
 *
 *   - render_to_png: writes the RGB buffer to a PNG file.
 *                    Receives const fractal_pl& only to read nx/ny
 *                    (image dimensions). The buffer is passed as a
 *                    raw pointer because stbi_write_png is a C API.
 *
 * Typical usage in main:
 *
 *   auto buffer = render::render_color(plane, "crazy");
 *   render::render_to_png(plane, buffer.get(), "output.png");
 */
class render {
public:
    static std::unique_ptr<unsigned char[]> render_color(
        const fractal_pl& plane,
        const std::string& color_type
    );

    static void render_to_png(
        const fractal_pl& plane,
        const unsigned char* buffer,
        const std::string& filename
    );
};
