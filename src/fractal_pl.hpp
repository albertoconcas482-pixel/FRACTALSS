#pragma once
#include <cstddef>
#include <vector>
#include "fractal_el.hpp"

/*
 * Represents the complex plane sampled on a regular nx * ny grid.
 *
 * The grid is stored row-major in data_: element at column ix,
 * row iy is located at index iy * nx_ + ix.
 * Row 0 corresponds to ymax (top), last row corresponds to ymin (bottom).
 *
 * Responsibilities:
 *   - allocate and initialize the grid on construction
 *   - provide read/write access to the sampled points
 *
 * Mandelbrot computation and coloring are delegated
 * to separate modules.
 *
 * Future: this class is the natural owner of symmetry metadata.
 * If the viewport contains the real axis (ymin < 0 < ymax), fractal_pl
 * could detect the symmetric portion and expose the index range
 * of the rows to compute, allowing mandel_set to skip the mirrored half.
 * fractal_pl would then handle the mirroring step after computation.
 */
class fractal_pl {
private:
    double xmin_{};
    double xmax_{};
    double ymin_{};
    double ymax_{};
    std::size_t nx_{};
    std::size_t ny_{};
    std::vector<fractal_el> data_{};

    void init_grid();

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

    std::vector<fractal_el>& data() { return data_; }
    const std::vector<fractal_el>& data() const { return data_; }
};
