#pragma once

/*
 * fractal_el — Represents a single pixel inside the complex plane grid.
 *
 * Optimized to minimize memory footprint. Coordinate 'c' and the 'inside' flag 
 * have been removed. Added 'magnitude_sq' to support smooth coloring 
 * without runtime square root penalties. Total size: 8 bytes per element.
 */
struct fractal_el {
    int escapeiter{0};
    float magnitude_sq{0.0f}; // Stores |z|^2 at the time of escape
};
