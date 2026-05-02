#include "mandel_set.hpp"

int mandel_set::iterate(double cr, double ci, int maxiter) {
    double zr{0.0};
    double zi{0.0};
    double zr2{0.0};  // cached zr^2, reused each iteration
    double zi2{0.0};  // cached zi^2, reused each iteration

    int iter{0};

    // Iterate z = z^2 + c until |z|^2 > 4 (escape) or maxiter reached.
    // zr2 and zi2 are computed once per iteration and reused both in
    // the escape test and in the next step calculation.
    while ((zr2 + zi2) <= 4.0 && iter < maxiter) {
        zi  = next_imag(zr, zi, ci);
        zr  = next_real(zr2, zi2, cr);
        zr2 = zr * zr;
        zi2 = zi * zi;
        ++iter;
    }

    return iter;
}
