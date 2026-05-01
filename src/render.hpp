#pragma once
#include <string>
#include "fractal_pl.hpp"

/*
 * Rendering module.
 *
 * Responsibilities:
 *   - render_color: assigns RGB values to each point in the plane
 *                   by delegating to color_registry
 *   - render_to_ppm: writes the colored plane to a PPM image file
 *
 * Both methods operate on a fractal_pl after mandel_check has been run.
 */
class render {
public:
    static void render_color(fractal_pl& plane, const std::string& color_type);
    static void render_to_ppm(const fractal_pl& plane, const std::string& filename);
};
