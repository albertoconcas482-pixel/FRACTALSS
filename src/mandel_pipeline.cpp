#include "mandel_pipeline.hpp"
#include "mandel_set.hpp"

void mandel_pipeline::mandel_check(fractal_pl& plane, int maxiter) {
    auto& data{plane.data()};
    const std::size_t nx{plane.nx()};
    const std::size_t ny{plane.ny()};

    // Cache limits to avoid repetitive function calls
    const double xmin = plane.xmin();
    const double ymax = plane.ymax();
    const double dx = (plane.xmax() - xmin) / static_cast<double>(nx - 1);
    const double dy = (ymax - plane.ymin()) / static_cast<double>(ny - 1);

    // OpenMP dynamic scheduling: assigns one row at a time to idle threads, 
    // ensuring perfect load balancing across dense and empty regions of the fractal.
    #pragma omp parallel for schedule(dynamic, 1)
    for (std::size_t row = 0; row < ny; ++row) {
        
        // Loop-Invariant Code Motion (Hoisting)
        // The imaginary coordinate is calculated ONCE per row per thread.
        const double ci = ymax - static_cast<double>(row) * dy;

        for (std::size_t col = 0; col < nx; ++col) {
            
            // Linear flat index computation for contiguous memory writes
            const std::size_t i = row * nx + col;
            
            // Real coordinate computation
            const double cr = xmin + static_cast<double>(col) * dx;

            if (mandel_set::is_in_cardioid(cr, ci) || mandel_set::is_in_period2_bulb(cr, ci)) {
                data[i].escapeiter = maxiter;
                data[i].magnitude_sq = 0.0f; // Interior points do not escape
            } else {
                auto result = mandel_set::iterate_with_period(cr, ci, maxiter);
                data[i].escapeiter = result.escapeiter;
                data[i].magnitude_sq = result.magnitude_sq;
            }
        }
    }
}
