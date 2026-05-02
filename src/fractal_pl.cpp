#include "fractal_pl.hpp"

fractal_pl::fractal_pl(double xmin, double xmax,
                       double ymin, double ymax,
                       std::size_t nx, std::size_t ny)
    : xmin_{xmin}
    , xmax_{xmax}
    , ymin_{ymin}
    , ymax_{ymax}
    , nx_{nx}
    , ny_{ny}
    , data_(nx * ny)
{
    init_grid();
}

void fractal_pl::init_grid() {
    // dx and dy are the step sizes between adjacent points
    // along the real and imaginary axes respectively.
    const auto dx{(xmax_ - xmin_) / static_cast<double>(nx_ - 1)};
    const auto dy{(ymax_ - ymin_) / static_cast<double>(ny_ - 1)};

    for (std::size_t row{}; row < ny_; ++row) {
        for (std::size_t col{}; col < nx_; ++col) {
            const auto i{row * nx_ + col};

            const auto real{xmin_ + static_cast<double>(col) * dx};
            // imag decreases with row: row 0 = ymax (top of viewport)
            const auto imag{ymax_ - static_cast<double>(row) * dy};

            data_[i].c         = {real, imag};
            data_[i].inside    = {};
            data_[i].escapeiter = {};
            // r, g, b removed: color is handled by render via an
            // external buffer, not stored inside fractal_el.
        }
    }
}
