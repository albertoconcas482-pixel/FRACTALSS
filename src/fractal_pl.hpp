#pragma once
#include <cstddef>
#include <vector>
#include "fractal_el.hpp"

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
