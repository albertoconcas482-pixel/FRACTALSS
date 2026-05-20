#include "mandel_pipeline.hpp"
#include "mandel_set.hpp"

void mandel_pipeline::mandel_check(fractal_pl& plane, int maxiter) {
    auto& data{plane.data()};
    const std::size_t total_pixels{data.size()};

    // The loop is now completely stateless and ready for parallel block partitioning.
    for (std::size_t i = 0; i < total_pixels; ++i) {
        
        // Resolve geometric coordinates directly from the linear index
        const auto [cr, ci] = plane.get_coordinates(i);

        if (mandel_set::is_in_cardioid(cr, ci) || mandel_set::is_in_period2_bulb(cr, ci)) {
            data[i].escapeiter = maxiter;
        } else {
            data[i].escapeiter = mandel_set::iterate_with_period(cr, ci, maxiter);
        }
    }
}
