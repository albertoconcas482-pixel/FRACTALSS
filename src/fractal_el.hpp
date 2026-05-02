#pragma once
#include <complex>

/*
 * Represents a single point in the complex plane,
 * together with all data produced during fractal computation.
 *
 * RGB color data has been removed: coloring is a presentation
 * concern handled by render, which fills an external buffer.
 *
 * Fields:
 *   c          - coordinate in the complex plane
 *   inside     - true if the point belongs to the Mandelbrot set
 *   escapeiter - number of iterations before divergence;
 *                set to maxiter if the point is inside the set
 */
struct fractal_el {
    std::complex<double> c{};
    bool inside{};
    int escapeiter{};
};
