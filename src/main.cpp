#include <iostream>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <string>

#include "fractal_pl.hpp"
#include "mandel_pipeline.hpp"
#include "render.hpp"

int main() {
    try {
     // Target Viewport Settings (Seahorse valley area at low zoom)
        constexpr double cx{-1.338396208};
        constexpr double cy{-0.051328422};
        constexpr double target_zoom{53.0};
        
        // 4K Resolution to ensure high spatial sampling density
        constexpr int render_width  {3840};
        constexpr int render_height {2880};
        
        // Mathematically derive the viewport bounds preserving the 4:3 aspect ratio
        constexpr double x_width {3.5 / target_zoom};
        constexpr double y_height{x_width * (static_cast<double>(render_height) / render_width)};

        const double xmin{cx - x_width / 2.0};
        const double xmax{cx + x_width / 2.0};
        const double ymin{cy - y_height / 2.0};
        const double ymax{cy + y_height / 2.0};
        
        // --- ITERATION BOOST EXPERIMENT ---
        // Option A: Scaling up base_quality 10x (scales maxiter to ~33,800)
        constexpr double base_quality{10000.0}; 
        const double zoom        {3.5 / (xmax - xmin)};
        const double zoom_clamped{std::max(zoom, 2.0)};
        
        const int maxiter {std::max(200, static_cast<int>(
                                base_quality * std::sqrt(2.0 * std::log2(zoom_clamped))))};

        // Option B: Hardcoded brutal override for absolute testing.
        // Uncomment the line below to force exactly 50,000 iterations, bypassing the formula.
        // constexpr int maxiter {50000};
        const std::string color_scheme{"smooth"};
        const std::string output_file {"images/mandelbrot_brent.png"};

        std::cout << "=======================================\n";
        std::cout << "      FRACTALSS - Clean Core Engine    \n";
        std::cout << "=======================================\n";
        std::cout << "Computed Zoom  : " << zoom     << "\n";
        std::cout << "Max Iterations : " << maxiter  << "\n";
        std::cout << "Resolution     : " << render_width << "x" << render_height << "\n";
        std::cout << "Output File    : " << output_file << "\n\n";

        std::cout << "Initializing memory grid and starting compute kernel..." << std::endl;
        
        const auto start_time{std::chrono::steady_clock::now()};

        // 1. Grid construction (RAM optimized)
        fractal_pl plane(xmin, xmax, ymin, ymax, render_width, render_height);

        // 2. Kernel execution using Brent's Cycle Detection and Strength Reduction
        mandel_pipeline::mandel_check(plane, maxiter);

        // 3. Color mapping allocation and native PNG output serialization
        auto buffer{render::render_color(plane, color_scheme, maxiter)};
        render::render_to_png(plane, buffer.get(), output_file);

        const auto end_time{std::chrono::steady_clock::now()};
        const auto total_ms{
            std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count()
        };

        std::cout << "Render engine finished successfully!\n";
        std::cout << "Total execution time: " << total_ms / 1000.0 << " s\n";
        std::cout << "=======================================\n";
    }
    catch (const std::exception& e) {
        std::cerr << "\nFatal Exception: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
