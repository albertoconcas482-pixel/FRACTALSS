#pragma once
#include <complex>

/*
 * Represents a single point in the complex plane,
 * together with all data produced during fractal
 * computation and final coloring.
 *
 * Fields:
 *   c          - coordinate in the complex plane
 *   inside     - true if the point belongs to the Mandelbrot set
 *   escapeiter - number of iterations before divergence;
 *                set to maxiter if the point is inside the set
 *   r, g, b    - final RGB color components (0-255)
 *                assigned by render_color
 */
struct fractal_el {
    std::complex<double> c{};
    bool inside{};
    int escapeiter{};
    unsigned char r{};
    unsigned char g{};
    unsigned char b{};
};
