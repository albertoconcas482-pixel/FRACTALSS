#pragma once
#include "fractal_pl.hpp"

/*
 * Mandelbrot set computation module.
 *
 * For each point in the plane, mandel_check determines whether
 * the point belongs to the Mandelbrot set and records the number
 * of iterations before divergence.
 *
 * Results are written directly into the fractal_el elements
 * of the plane (inside, escapeiter).
 */
namespace mandel_set {
    void mandel_check(fractal_pl& plane, int maxiter);
}
