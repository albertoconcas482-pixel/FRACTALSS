#include "render.hpp"
#include "colors.hpp"
#include <fstream>
#include <stdexcept>

void render::render_color(fractal_pl& plane, const std::string& color_type) {
    color_registry::apply_color(color_type, plane);
}

void render::render_to_ppm(const fractal_pl& plane, const std::string& filename) {
    std::ofstream out{filename};

    if (!out) {
        throw std::runtime_error{"impossibile aprire il file di output"};
    }

    out << "P3\n";
    out << plane.nx() << ' ' << plane.ny() << '\n';
    out << "255\n";

    const auto& data{plane.data()};

    for (const auto& el : data) {
        out << static_cast<int>(el.r) << ' '
            << static_cast<int>(el.g) << ' '
            << static_cast<int>(el.b) << '\n';
    }
}
