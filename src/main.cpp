#include <iostream>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <string>

#include "fractal_pl.hpp"
#include "mandel_pipeline.hpp"
#include "render.hpp" // Added back to enable the presentation layer
#include "utils.hpp"  // Kept in case you still need raw binary exports

int main() {
    try {
        // Target Viewport Settings (Seahorse valley dense spiral area)
        constexpr double cx{-1.338396208};
        constexpr double cy{-0.051328422};
        
        // Target zoom level derived from your external explorer input
        constexpr double target_zoom{536915.0};
        
        // Mathematically derive the viewport bounds preserving the 4:3 aspect ratio (1920x1440)
        constexpr double x_width {3.5 / target_zoom};
        constexpr double y_height{x_width * (1440.0 / 1920.0)};

        const double xmin{cx - x_width / 2.0};
        const double xmax{cx + x_width / 2.0};
        const double ymin{cy - y_height / 2.0};
        const double ymax{cy + y_height / 2.0};

        constexpr int render_width  {1920};
        constexpr int render_height {1440};
        
        // Adaptive runtime computation of maximum iterations based on zoom factor
        constexpr double base_quality{1000.0};
        const double zoom        {3.5 / (xmax - xmin)};
        const double zoom_clamped{std::max(zoom, 2.0)};
        const int    maxiter     {std::max(200, static_cast<int>(
                                    base_quality * std::sqrt(2.0 * std::log2(zoom_clamped))))};

        // Output configurations
        const std::string output_png_file  {"mandelbrot_smooth.png"};
        const std::string output_data_file {"fractal_dump.bin"};

        std::cout << "=======================================\n";
        std::cout << "      FRACTALSS - High Performance mode\n";
        std::cout << "=======================================\n";
        std::cout << "Computed Zoom  : " << zoom     << "\n";
        std::cout << "Max Iterations : " << maxiter  << "\n";
        std::cout << "Resolution     : " << render_width << "x" << render_height << "\n";
        std::cout << "Output Target  : " << output_png_file << "\n\n";

        std::cout << "Initializing memory grid and starting compute kernel..." << std::endl;
        
        const auto start_time{std::chrono::steady_clock::now()};

        // 1. Grid construction (RAM optimized flat row-major layout)
        fractal_pl plane(xmin, xmax, ymin, ymax, render_width, render_height);

        // 2. Kernel execution using OpenMP and Brent's Cycle Detection
        mandel_pipeline::mandel_check(plane, maxiter);
        std::cout << "Kernel execution finished successfully." << std::endl;

        // 3. Presentation Layer: Apply the optimized 2-pass smooth color scheme
        std::cout << "Applying adaptive smooth coloring algorithm (LUT-based)..." << std::endl;
        auto color_buffer = render::render_color(plane, "smooth_lava", maxiter);

        // 4. Image I/O: Stream the RGB buffer directly to a lossless PNG file via stb_image_write
        std::cout << "Streaming color buffer to disk as PNG..." << std::endl;
        render::render_to_png(plane, color_buffer.get(), output_png_file);

        // Optional: Keep the raw data dump if you still want to cross-analyze via Python
        // export_data_raw(plane, output_data_file);

        const auto end_time{std::chrono::steady_clock::now()};
        const auto total_ms{
            std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count()
        };

        std::cout << "\nRender process completed successfully!\n";
        std::cout << "Total execution time: " << total_ms / 1000.0 << " s\n";
        std::cout << "=======================================\n";
    }
    catch (const std::exception& e) {
        std::cerr << "\nFatal Exception: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
