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
    , data_(nx * ny) // Automatically zeroes out escapeiter for all elements
{
    // Grid geometric precomputations removed for maximum allocation efficiency.
}
