#include "mandel_set.hpp"

namespace {
    inline double next_imag(double zr, double zi, double ci) {
        return 2.0 * zr * zi + ci;
    }

    inline double next_real(double zr2, double zi2, double cr) {
        return zr2 - zi2 + cr;
    }

    
    inline bool is_in_main_cardioid(double cr, double ci) {
        const double x_shifted {cr - 0.25};
        const double y2 {ci * ci};
        const double q {x_shifted * x_shifted + y2};
    
        return q * (q + x_shifted) <= 0.25 * y2;
    }

    inline bool is_in_period2_bulb(double cr, double ci) {
            const double x_shifted {cr + 1.0};
            return (x_shifted * x_shifted + ci * ci) <= 0.0625;
        }
    
}

void mandel_set::mandel_check(fractal_pl& plane, int maxiter) {
    auto& data = plane.data();

    for (auto& el : data) {
        double zr = 0.0;
        double zi = 0.0;
        double zr2 = 0.0;
        double zi2 = 0.0;
        const double cr = el.c.real();
        const double ci = el.c.imag();
        if (is_in_main_cardioid(cr, ci) || is_in_period2_bulb(cr, ci)) {
            el.escapeiter = maxiter;
            el.inside = true;
            continue;
        }
        int iter = 0;

        while ((zr2 + zi2) <= 4.0 && iter < maxiter) {
            zi = next_imag(zr, zi, ci);
            zr = next_real(zr2, zi2, cr);

            zr2 = zr * zr;
            zi2 = zi * zi;
            ++iter;
        }

        el.escapeiter = iter;
        el.inside = (iter == maxiter);
    }
}
