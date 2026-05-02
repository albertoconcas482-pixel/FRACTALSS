#pragma once
#include <complex>

/*
 * mandel_set — atomic primitives for Mandelbrot set computation.
 *
 * This module is a toolbox: each function does exactly one thing.
 * No function knows about the others, no orchestration happens here.
 *
 * The pipeline that composes these primitives lives in
 * mandel_pipeline.hpp / .cpp.
 */
namespace mandel_set {

    /*
     * Computes the real part of z^2 + c for one iteration step.
     * Receives zr2, zi2 (cached squares) and cr (real part of c).
     * Equivalent to: zr^2 - zi^2 + cr
     */
    inline double next_real(double zr2, double zi2, double cr) {
        return zr2 - zi2 + cr;
    }

    /*
     * Computes the imaginary part of z^2 + c for one iteration step.
     * Receives zr, zi (current z) and ci (imaginary part of c).
     * Equivalent to: 2 * zr * zi + ci
     */
    inline double next_imag(double zr, double zi, double ci) {
        return 2.0 * zr * zi + ci;
    }

    /*
     * Returns true if c belongs to the main cardioid of the Mandelbrot set.
     * Uses the algebraic condition: q*(q + (cr - 0.25)) <= 0.25 * ci^2
     * where q = (cr - 0.25)^2 + ci^2.
     */
    inline bool is_in_cardioid(double cr, double ci) {
        const double x_shifted{cr - 0.25};
        const double y2{ci * ci};
        const double q{x_shifted * x_shifted + y2};
        return q * (q + x_shifted) <= 0.25 * y2;
    }

    /*
     * Returns true if c belongs to the period-2 bulb of the Mandelbrot set.
     * The bulb is a circle centered at (-1, 0) with radius 0.25.
     */
    inline bool is_in_period2_bulb(double cr, double ci) {
        const double x_shifted{cr + 1.0};
        return (x_shifted * x_shifted + ci * ci) <= 0.0625;
    }

    /*
     * Iterates z = z^2 + c until |z|^2 > 4 (escape) or maxiter is reached.
     * Returns the number of iterations before escape, or maxiter if the
     * point did not diverge (assumed interior).
     *
     * Does NOT apply cardioid / bulb early exit — that is the pipeline's
     * responsibility.
     */
    int iterate(double cr, double ci, int maxiter);

    /*
     * Like iterate(), but also applies periodicity checking (cycle detection).
     *
     * Every period_k iterations the current z is saved as a reference.
     * If at any point |z - z_ref|^2 < epsilon, the orbit has entered a cycle
     * and the point is interior: returns maxiter immediately.
     *
     * period_k  — how often to update the reference point.
     *             Smaller values detect cycles faster but add overhead per
     *             iteration. Typical range: 10–100.
     * epsilon   — squared distance threshold for cycle detection.
     *             1e-20 is tight enough to avoid false positives.
     *
     * Returns maxiter if the point is interior (by cycle or by exhaustion),
     * or n < maxiter if the point escapes at iteration n.
     * Contract is identical to iterate() from the caller's perspective.
     */
    int iterate_with_period(double cr, double ci, int maxiter, int period_k);

} // namespace mandel_set
