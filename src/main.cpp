#include <atomic>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <exception>
#include <iostream>
#include <string>
#include <thread>

#include "fractal_pl.hpp"
#include "mandel_set.hpp"
#include "render.hpp"

/*
 * Entry point. Orchestrates the rendering pipeline:
 *
 *   1. fractal_pl  — build the complex plane grid
 *   2. mandel_set  — compute Mandelbrot membership for each point
 *   3. render      — colorize and write the result to a PPM file
 *
 * maxiter is derived from zoom level using the empirical formula:
 *   zoom    = 3.5 / (xmax - xmin)
 *   maxiter = base_quality * sqrt(2 * log2(zoom))
 *
 * zoom is clamped to a minimum of 2.0 so that log2 stays positive
 * even at the standard full view (zoom = 1). A hard floor of 200
 * iterations is also enforced.
 *
 * Aspect ratio rule: (xmax - xmin) / (ymax - ymin) must equal
 * render_width / render_height to avoid geometric distortion.
 * Current: 0.008 / 0.006 = 4/3 = 1920/1440.
 */
int main() {
    std::atomic<bool> timer_running {false};
    std::thread timer_thread;

    try {
        // --- viewport: cardioide/bulbo-2 junction zone, 4:3 aspect ratio ---
        constexpr double xmin  {-1.790};
        constexpr double xmax  {-1.782};
        constexpr double ymin  {-0.003};
        constexpr double ymax  { 0.003};

        // --- resolution: 4:3 at 2K area (1920x1440) ---
        constexpr int render_width  {1920};
        constexpr int render_height {1440};

        // maxiter calibrated on zoom level
        // zoom clamped to >= 2.0 so log2 stays positive at full view
        constexpr double base_quality {1000.0};
        const double zoom         { 3.5 / (xmax - xmin) };
        const double zoom_clamped { std::max(zoom, 2.0) };
        const int    maxiter      { std::max(200, static_cast<int>(base_quality * std::sqrt(2.0 * std::log2(zoom_clamped)))) };

        fractal_pl plane(xmin, xmax, ymin, ymax, render_width, render_height);

        const std::string color_scheme {"crazy"};
        const std::string output_file  {"mandelbrot_full.ppm"};

        std::cout << "zoom    = " << zoom    << "\n";
        std::cout << "maxiter = " << maxiter << "\n";
        std::cout << "pixel   = " << render_width * render_height << "\n";

        const auto start_time {std::chrono::steady_clock::now()};

        timer_running = true;

        timer_thread = std::thread([&timer_running, &start_time]() {
            using namespace std::chrono_literals;

            while (timer_running) {
                const auto now {std::chrono::steady_clock::now()};
                const auto elapsed_seconds {
                    std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count()
                };

                std::cout << "\rTempo di generazione: "
                          << elapsed_seconds
                          << " s" << std::flush;

                std::this_thread::sleep_for(1s);
            }
        });

        mandel_set::mandel_check(plane, maxiter);
        render::render_color(plane, color_scheme);
        render::render_to_ppm(plane, output_file);

        timer_running = false;
        if (timer_thread.joinable()) {
            timer_thread.join();
        }

        const auto end_time {std::chrono::steady_clock::now()};
        const auto total_seconds {
            std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count()
        };

        std::cout << "\rTempo di generazione: "
                  << total_seconds
                  << " s\n";

        std::cout << "Render completato: " << output_file << "\n";
    }
    catch (const std::exception& e) {
        timer_running = false;

        if (timer_thread.joinable()) {
            timer_thread.join();
        }

        std::cerr << "\nErrore: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
