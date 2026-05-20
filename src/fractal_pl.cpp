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
    , dx_{(xmax - xmin) / static_cast<double>(nx - 1)}
    , dy_{(ymax - ymin) / static_cast<double>(ny - 1)}
    , data_(nx * ny) 
{
    // Step deltas are precomputed and cached to avoid redundant division overhead.
}
