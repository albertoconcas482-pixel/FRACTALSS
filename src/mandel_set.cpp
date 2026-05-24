#include "mandel_set.hpp"

mandel_set::iteration_result mandel_set::iterate_with_period(double cr, double ci, int maxiter) {
    constexpr double epsilon{1e-20};

    double zr{0.0};
    double zi{0.0};
    double zr2{0.0};
    double zi2{0.0};

    double zr_ref{0.0};
    double zi_ref{0.0};

    int iter{0};
    int power{1}; // Brent's geometric tracking window

    while ((zr2 + zi2) <= 4.0 && iter < maxiter) {
        zi  = next_imag(zr, zi, ci);
        zr  = next_real(zr2, zi2, cr);
        zr2 = zr * zr;
        zi2 = zi * zi;
        ++iter;

        const double dr{zr - zr_ref};
        const double di{zi - zi_ref};
        if (dr * dr + di * di < epsilon) {
            // Orbit cycle detected: point is interior. Magnitude is irrelevant here.
            return {maxiter, 0.0f}; 
        }

        // When the current iteration hits the end of the tracking window,
        // we double the window scale and save a new orbit reference point.
        if (iter == power) {
            zr_ref = zr;
            zi_ref = zi;
            power *= 2;
        }
    }

    // Loop finished (either escaped or reached maxiter).
    // Return iteration count and the current squared magnitude (|z|^2).
    return {iter, static_cast<float>(zr2 + zi2)};
}
