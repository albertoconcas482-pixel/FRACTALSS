#pragma once

/*
 * fractal_el — Represents a single pixel inside the complex plane grid.
 *
 * Optimized to minimize memory footprint: coordinate 'c' and the 'inside' flag 
 * have been removed as they are redundant and can be computed on the fly.
 * Memory consumption is reduced to 4 bytes per element.
 */
struct fractal_el {
    int escapeiter{0};
};
