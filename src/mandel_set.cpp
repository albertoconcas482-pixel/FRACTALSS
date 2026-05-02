#include "mandel_set.hpp"

int mandel_set::iterate(double cr, double ci, int maxiter) {
    double zr{0.0};
    double zi{0.0};
    double zr2{0.0};  // cached zr^2, reused each iteration
    double zi2{0.0};  // cached zi^2, reused each iteration

    int iter{0};

    while ((zr2 + zi2) <= 4.0 && iter < maxiter) {
        zi  = next_imag(zr, zi, ci);
        zr  = next_real(zr2, zi2, cr);
        zr2 = zr * zr;
        zi2 = zi * zi;
        ++iter;
    }

    return iter;
}

int mandel_set::iterate_with_period(double cr, double ci, int maxiter, int period_k) {
    // Squared distance threshold for cycle detection.
    // Using squared distance avoids a sqrt() per iteration.
    // 1e-20 is tight enough to avoid false positives on exterior points.
    constexpr double epsilon{1e-20};

    double zr{0.0};
    double zi{0.0};
    double zr2{0.0};
    double zi2{0.0};

    // Reference point: saved every period_k iterations and compared
    // against the current z to detect orbital cycles.
    double zr_ref{0.0};
    double zi_ref{0.0};

    int iter{0};

    while ((zr2 + zi2) <= 4.0 && iter < maxiter) {
        zi  = next_imag(zr, zi, ci);
        zr  = next_real(zr2, zi2, cr);
        zr2 = zr * zr;
        zi2 = zi * zi;
        ++iter;

        // Cycle detection: check if z is close to the saved reference.
        const double dr{zr - zr_ref};
        const double di{zi - zi_ref};
        if (dr * dr + di * di < epsilon) {
            // Orbit is periodic — point is interior.
            return maxiter;
        }

        // Update reference every period_k iterations.
        if (iter % period_k == 0) {
            zr_ref = zr;
            zi_ref = zi;
        }
    }

    return iter;
}
