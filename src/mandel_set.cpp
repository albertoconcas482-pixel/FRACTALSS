#include "mandel_set.hpp"

namespace {
    /*
     * Computes the imaginary part of z^2 + c.
     * Receives zr, zi (current z) and ci (imaginary part of c).
     * Equivalent to: 2 * zr * zi + ci
     */
    inline double next_imag(double zr, double zi, double ci) {
        return 2.0 * zr * zi + ci;
    }

    /*
     * Computes the real part of z^2 + c.
     * Receives zr2, zi2 (cached squares of zr and zi) and cr (real part of c).
     * Equivalent to: zr^2 - zi^2 + cr
     * zr2 and zi2 are cached from the previous iteration to avoid recomputation.
     */
    inline double next_real(double zr2, double zi2, double cr) {
        return zr2 - zi2 + cr;
    }

    /*
     * Returns true if c belongs to the main cardioid of the Mandelbrot set.
     * Uses the algebraic condition: q*(q + (cr - 0.25)) <= 0.25 * ci^2
     * where q = (cr - 0.25)^2 + ci^2.
     * Points inside the cardioid always belong to the set.
     */
    inline bool is_in_main_cardioid(double cr, double ci) {
        const double x_shifted {cr - 0.25};
        const double y2 {ci * ci};
        const double q {x_shifted * x_shifted + y2};
        return q * (q + x_shifted) <= 0.25 * y2;
    }

    /*
     * Returns true if c belongs to the period-2 bulb of the Mandelbrot set.
     * The bulb is a circle centered at (-1, 0) with radius 0.25.
     * Points inside always belong to the set.
     */
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
        double zr2 = 0.0;  // cached zr^2, reused each iteration
        double zi2 = 0.0;  // cached zi^2, reused each iteration
        const double cr = el.c.real();
        const double ci = el.c.imag();

        // Early exit: skip full iteration for known interior points
        if (is_in_main_cardioid(cr, ci) || is_in_period2_bulb(cr, ci)) {
            el.escapeiter = maxiter;
            el.inside = true;
            continue;
        }

        int iter = 0;

        // Iterate z = z^2 + c until |z|^2 > 4 (escape) or maxiter is reached
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
