#include "mandel_pipeline.hpp"
#include "mandel_set.hpp"

void mandel_pipeline::mandel_check(fractal_pl& plane, int maxiter, int period_k) {
    auto& data{plane.data()};

    for (auto& el : data) {
        const double cr{el.c.real()};
        const double ci{el.c.imag()};

        // Step 1 & 2: O(1) early exit for analytically known interior regions.
        if (mandel_set::is_in_cardioid(cr, ci) || mandel_set::is_in_period2_bulb(cr, ci)) {
            el.escapeiter = maxiter;
            el.inside     = true;
            continue;
        }

        // Step 3: loop with cycle detection for all remaining points.
        const int iter{mandel_set::iterate_with_period(cr, ci, maxiter, period_k)};
        el.escapeiter = iter;
        el.inside     = (iter == maxiter);
    }
}
