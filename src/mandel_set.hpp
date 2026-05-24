#pragma once

namespace mandel_set {

    // Struct returned by the computational kernel
    struct iteration_result {
        int escapeiter;
        float magnitude_sq;
    };

    inline double next_real(double zr2, double zi2, double cr) {
        return zr2 - zi2 + cr;
    }

    inline double next_imag(double zr, double zi, double ci) {
        return 2.0 * zr * zi + ci;
    }

    inline bool is_in_cardioid(double cr, double ci) {
        const double x_shifted{cr - 0.25};
        const double y2{ci * ci};
        const double q{x_shifted * x_shifted + y2};
        return q * (q + x_shifted) <= 0.25 * y2;
    }

    inline bool is_in_period2_bulb(double cr, double ci) {
        const double x_shifted{cr + 1.0};
        return (x_shifted * x_shifted + ci * ci) <= 0.0625;
    }

    /*
     * Cycle detection via Brent's Algorithm.
     * Returns both the iteration count and the squared magnitude at escape time.
     */
    iteration_result iterate_with_period(double cr, double ci, int maxiter);

} // namespace mandel_set
