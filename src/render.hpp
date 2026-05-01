#pragma once
#include <string>
#include "fractal_pl.hpp"

class render {
public:
    static void render_color(fractal_pl& plane, const std::string& color_type);
    static void render_to_ppm(const fractal_pl& plane, const std::string& filename);
};
