#include "render.hpp"
#include "colors.hpp"
#include <fstream>
#include <stdexcept>

void render::render_color(fractal_pl& plane, const std::string& color_type) {
    // Delegates color assignment to color_registry based on color_type name
    color_registry::apply_color(color_type, plane);
}

void render::render_to_ppm(const fractal_pl& plane, const std::string& filename) {
    std::ofstream out{filename};

    if (!out) {
        throw std::runtime_error{"cannot open output file"};
    }

    // PPM header: format P3 (ASCII RGB), width, height, max color value
    out << "P3\n";
    out << plane.nx() << ' ' << plane.ny() << '\n';
    out << "255\n";

    const auto& data{plane.data()};

    // Write one RGB triplet per line, row-major order
    for (const auto& el : data) {
        out << static_cast<int>(el.r) << ' '
            << static_cast<int>(el.g) << ' '
            << static_cast<int>(el.b) << '\n';
    }
}
