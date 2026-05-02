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
     * Classifies every point using: cardioid + bulb2 check, then
     * iterate_with_period (cycle detection).
     *
     * period_k controls how often the reference point is updated.
     * Passed explicitly so the benchmark can sweep different values.
     */
    void mandel_check(fractal_pl& plane, int maxiter, int period_k);

    /*
     * Same pipeline but uses plain iterate() instead of
     * iterate_with_period — no cycle detection overhead.
     * Used as the CARDIOID baseline in benchmarks.
     */
    void mandel_check_cardioid(fractal_pl& plane, int maxiter);

} // namespace mandel_pipeline
