#pragma once
#include <memory>
#include <string>
#include "fractal_pl.hpp"

class render {
public:
    static std::unique_ptr<unsigned char[]> render_color(
        const fractal_pl& plane,
        const std::string& color_type,
        int maxiter
    );

    static void render_to_png(
        const fractal_pl& plane,
        const unsigned char* buffer,
        const std::string& filename
    );
};
