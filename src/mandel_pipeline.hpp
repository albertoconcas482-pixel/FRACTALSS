#pragma once
#include "fractal_pl.hpp"

/*
 * mandel_pipeline — classification strategy for the Mandelbrot set.
 *
 * This module composes the atomic primitives from mandel_set into
 * a complete per-point classification pipeline. It decides the order
 * of the checks and writes results into fractal_el (inside, escapeiter).
 *
 * mandel_set knows nothing about fractal_pl or the pipeline order.
 * mandel_pipeline knows nothing about rendering or colors.
 */
namespace mandel_pipeline {

    /*
     * Classifies every point in the plane as inside or outside the
     * Mandelbrot set, recording the escape iteration count.
     *
     * Pipeline order (fastest checks first):
     *   1. is_in_cardioid   — O(1), catches most interior points
     *   2. is_in_period2_bulb — O(1), catches the second-largest region
     *   3. iterate          — full iteration loop for remaining points
     *
     * Interior points (all three paths) write escapeiter = maxiter
     * and inside = true. This keeps the contract uniform for colorizers.
     */
    void mandel_check(fractal_pl& plane, int maxiter);

} // namespace mandel_pipeline
