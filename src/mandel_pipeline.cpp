#include "mandel_pipeline.hpp"
#include "mandel_set.hpp"

void mandel_pipeline::mandel_check(fractal_pl& plane, int maxiter) {
    auto& data{plane.data()};

    for (auto& el : data) {
        const double cr{el.c.real()};
        const double ci{el.c.imag()};

        // Step 1 & 2: O(1) early exit for analytically known interior regions.
        // These cover the vast majority of interior points and are essentially free.
        if (mandel_set::is_in_cardioid(cr, ci) || mandel_set::is_in_period2_bulb(cr, ci)) {
            el.escapeiter = maxiter;
            el.inside     = true;
            continue;
        }

        // Step 3: full iteration loop for all remaining points.
        const int iter{mandel_set::iterate(cr, ci, maxiter)};
        el.escapeiter = iter;
        el.inside     = (iter == maxiter);
    }
}
