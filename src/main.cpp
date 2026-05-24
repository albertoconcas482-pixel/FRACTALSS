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
        // Viewport settings (Cardioid tail junction / high density filaments area)
        constexpr double xmin  {-1.790};
        constexpr double xmax  {-1.782};
        constexpr double ymin  {-0.003};
        constexpr double ymax  { 0.003};

        constexpr int render_width  {1920};
        constexpr int render_height {1440};

        // Adaptive runtime computation of maximum iterations based on zoom factor
        constexpr double base_quality{1000.0};
        const double zoom        {3.5 / (xmax - xmin)};
        const double zoom_clamped{std::max(zoom, 2.0)};
        const int    maxiter     {std::max(200, static_cast<int>(
                                    base_quality * std::sqrt(2.0 * std::log2(zoom_clamped))))};

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
