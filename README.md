# FRACTALSS

A C++ Mandelbrot set renderer focused on correctness, clean architecture, and progressive optimization.

## Overview

FRACTALSS renders the Mandelbrot set to a PPM image file. The project is structured around a clear separation of responsibilities across independent modules, making it easy to optimize, extend, and parallelize individual stages without touching the rest of the pipeline.

The current focus is on algorithmic optimization of the core computation kernel before introducing any parallelism.

## Pipeline

```
fractal_pl  →  mandel_set  →  render
```

| Stage | Module | Responsibility |
|---|---|---|
| 1 | `fractal_pl` | Build the complex plane grid |
| 2 | `mandel_set` | Compute Mandelbrot membership for each point |
| 3 | `render` | Colorize and write the result to a PPM file |

## Project Structure

```
src/
├── fractal_el.hpp      # Single point data structure (coordinate, membership, color)
├── fractal_pl.hpp/.cpp # Complex plane grid — row-major storage
├── mandel_set.hpp/.cpp # Mandelbrot computation kernel
├── render.hpp/.cpp     # Colorization and PPM file output
└── colors.hpp          # Color scheme registry (self-registering pattern)
```

## Architecture Notes

**Row-major grid layout** — points in `fractal_pl` are stored in a flat `std::vector<fractal_el>` row by row. Element at column `ix`, row `iy` is at index `iy * nx + ix`. Row 0 corresponds to `ymax` (top of viewport).

**Self-registering color schemes** — each colorizer class (e.g. `bw`, `crazy`) registers itself into `color_registry` at program startup via a static member initialization trick. Adding a new color scheme requires no changes to existing code.

**Escape time algorithm** — the core loop iterates `z = z² + c` until `|z|² > 4` (escape) or `maxiter` is reached. Squared magnitudes are cached each iteration to avoid redundant multiplications.

**Early exit optimizations** — points inside the main cardioid and the period-2 bulb are detected analytically before entering the iteration loop, skipping computation entirely for a large fraction of interior points.

## Build

Requires CMake and a C++17-compatible compiler.

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

> **Note:** always build in Release mode. Benchmarks show a ~2.65x speedup over Debug builds.

## Output

The renderer produces a `.ppm` file (ASCII RGB, format P3). The filename matches the color scheme name passed to `render_to_ppm`.

PPM files can be viewed directly in most image viewers or converted with tools like `convert` (ImageMagick):

```bash
convert output.ppm output.png
```

## Optimization Roadmap

Optimizations are applied in order of expected impact, measured and verified with benchmarks before moving to the next step.

- [x] Cardioid and period-2 bulb early exit
- [x] Release mode compilation
- [ ] Periodicity checking (cycle detection for interior points)
- [ ] Partial symmetry exploitation (mirror points across the real axis when viewport permits)
- [ ] Parallelization via index range partitioning on `fractal_pl::data`

## Benchmarks

The bottleneck is consistently `mandel_set::mandel_check`. Grid construction and rendering are negligible by comparison. Computation cost scales approximately as:

```
T ≈ C · nx · ny · maxiter
```

where `C` is an empirical constant of a few nanoseconds per pixel per iteration. This model predicts benchmark times reliably before running them.

See `Fractalss_benchmark-1.pdf` for detailed benchmark tables and analysis.
