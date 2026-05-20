#pragma once
#include <cstddef>
#include <vector>
#include "fractal_el.hpp"

/*
 * fractal_pl — Manages the complex plane grid storage.
 *
 * Allocates and handles a flat row-major vector of size nx * ny.
 * Geometric coordinate precomputations have been completely removed from the constructor.
 */
class fractal_pl {
private:
    double xmin_{};
    double xmax_{};
    double ymin_{};
    double ymax_{};
    double dx_{}; // Cached step size along the real axis
    double dy_{}; // Cached step size along the imaginary axis
    std::size_t nx_{};
    std::size_t ny_{};
    std::vector<fractal_el> data_{};

public:
    fractal_pl(double xmin, double xmax,
               double ymin, double ymax,
               std::size_t nx, std::size_t ny);

    double xmin() const { return xmin_; }
    double xmax() const { return xmax_; }
    double ymin() const { return ymin_; }
    double ymax() const { return ymax_; }

    std::size_t nx() const { return nx_; }
    std::size_t ny() const { return ny_; }


    /*
     * Computes the mathematical complex coordinates (cr, ci) for a given flat index.
     * This function is entirely stateless and execution-order independent.
     */
    inline std::pair<double, double> get_coordinates(std::size_t index) const {
        std::size_t row = index / nx_;
        std::size_t col = index % nx_;

        double cr = xmin_ + static_cast<double>(col) * dx_;
        double ci = ymax_ - static_cast<double>(row) * dy_;
        return {cr, ci};
    }

    std::vector<fractal_el>& data() { return data_; }
    const std::vector<fractal_el>& data() const { return data_; }
};
