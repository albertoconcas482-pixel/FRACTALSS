#include "mandel_pipeline.hpp"
#include "mandel_set.hpp"

void mandel_pipeline::mandel_check(fractal_pl& plane, int maxiter) {
    auto& data{plane.data()};
    const std::size_t total_pixels{data.size()};
    const std::size_t nx{plane.nx()};
    const std::size_t ny{plane.ny()};

    const double dx{(plane.xmax() - plane.xmin()) / static_cast<double>(nx - 1)};
    const double dy{(plane.ymax() - plane.ymin()) / static_cast<double>(ny - 1)};

    // Incremental geometric state tracking (Strength Reduction)
    double cr{plane.xmin()};
    double ci{plane.ymax()}; // Row 0 maps to ymax
    std::size_t current_col{0};

    for (std::size_t i = 0; i < total_pixels; ++i) {
        if (mandel_set::is_in_cardioid(cr, ci) || mandel_set::is_in_period2_bulb(cr, ci)) {
            data[i].escapeiter = maxiter;
        } else {
            data[i].escapeiter = mandel_set::iterate_with_period(cr, ci, maxiter);
        }

        // Incremental coordinate step
        cr += dx;
        ++current_col;

        // Handle flat vector layout carriage return
        if (current_col == nx) {
            current_col = 0;
            cr = plane.xmin();
            ci -= dy; // Move down towards ymin
        }
    }
}
