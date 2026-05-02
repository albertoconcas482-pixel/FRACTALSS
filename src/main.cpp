#include <atomic>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <exception>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "fractal_pl.hpp"
#include "mandel_pipeline.hpp"
#include "render.hpp"

// ─── benchmark mode ──────────────────────────────────────────────────────────
// Set to 1 to run a CSV benchmark sweep instead of a normal render.
// Output: xmin,xmax,ymin,ymax,nx,ny,maxiter,period_k,plane_ms,mandel_ms,total_ms
#define BENCHMARK_MODE 0

// ─── period_k ────────────────────────────────────────────────────────────────
// How often the reference point is updated inside iterate_with_period.
// Used in normal render mode only. In benchmark mode period_k is swept.
// Typical range: 10–100. To be calibrated after benchmark results.
static constexpr int period_k{20};

// ─────────────────────────────────────────────────────────────────────────────

#if BENCHMARK_MODE

int main() {
    try {
        // Same viewport as the original benchmark for direct comparison.
        constexpr double xmin{-0.80};
        constexpr double xmax{-0.70};
        constexpr double ymin{ 0.05};
        constexpr double ymax{ 0.15};

        const std::vector<std::size_t> resolution_values{2000, 5000, 8000, 10000};
        const std::vector<int>         maxiter_values   {50, 100, 200, 400, 800};
        const std::vector<int>         period_k_values  {10, 20, 50, 100};

        using clock_type = std::chrono::steady_clock;

        std::cout << "xmin,xmax,ymin,ymax,nx,ny,maxiter,period_k,"
                     "plane_ms,mandel_ms,total_ms\n";

        for (std::size_t resolution : resolution_values) {
            for (int maxiter : maxiter_values) {
                for (int k : period_k_values) {
                    const auto total_start{clock_type::now()};

                    const auto plane_start{clock_type::now()};
                    fractal_pl plane{xmin, xmax, ymin, ymax, resolution, resolution};
                    const auto plane_end{clock_type::now()};

                    const auto mandel_start{clock_type::now()};
                    mandel_pipeline::mandel_check(plane, maxiter, k);
                    const auto mandel_end{clock_type::now()};

                    const auto total_end{clock_type::now()};

                    const auto plane_ms{std::chrono::duration_cast<std::chrono::milliseconds>(
                        plane_end - plane_start).count()};
                    const auto mandel_ms{std::chrono::duration_cast<std::chrono::milliseconds>(
                        mandel_end - mandel_start).count()};
                    const auto total_ms{std::chrono::duration_cast<std::chrono::milliseconds>(
                        total_end - total_start).count()};

                    std::cout << xmin << ',' << xmax << ','
                              << ymin << ',' << ymax << ','
                              << resolution << ',' << resolution << ','
                              << maxiter << ',' << k << ','
                              << plane_ms << ',' << mandel_ms << ','
                              << total_ms << '\n';
                }
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Errore: " << e.what() << '\n';
        return 1;
    }

    return 0;
}

#else // ── normal render mode ──────────────────────────────────────────────────

/*
 * Entry point. Orchestrates the rendering pipeline:
 *
 *   1. fractal_pl       — build the complex plane grid
 *   2. mandel_pipeline  — classify each point (cardioid / bulb2 / period check)
 *   3. render           — colorize and write the result to a PNG file
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
    std::atomic<bool> timer_running{false};
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

        // maxiter calibrated on zoom level.
        // zoom clamped to >= 2.0 so log2 stays positive at full view.
        constexpr double base_quality{1000.0};
        const double zoom        {3.5 / (xmax - xmin)};
        const double zoom_clamped{std::max(zoom, 2.0)};
        const int    maxiter     {std::max(200, static_cast<int>(
                                    base_quality * std::sqrt(2.0 * std::log2(zoom_clamped))))};

        fractal_pl plane(xmin, xmax, ymin, ymax, render_width, render_height);

        const std::string color_scheme{"crazy"};
        const std::string output_file {"mandelbrot_full.png"};

        std::cout << "zoom     = " << zoom     << "\n";
        std::cout << "maxiter  = " << maxiter  << "\n";
        std::cout << "period_k = " << period_k << "\n";
        std::cout << "pixel    = " << render_width * render_height << "\n";

        const auto start_time{std::chrono::steady_clock::now()};
        timer_running = true;

        timer_thread = std::thread([&timer_running, &start_time]() {
            using namespace std::chrono_literals;
            while (timer_running) {
                const auto now{std::chrono::steady_clock::now()};
                const auto elapsed_seconds{
                    std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count()
                };
                std::cout << "\rTempo di generazione: "
                          << elapsed_seconds
                          << " s" << std::flush;
                std::this_thread::sleep_for(1s);
            }
        });

        mandel_pipeline::mandel_check(plane, maxiter, period_k);

        // render_color allocates and fills the RGB buffer, returns ownership.
        // render_to_png reads it as a raw pointer (stbi C API) then buffer
        // is automatically released at end of scope.
        auto buffer{render::render_color(plane, color_scheme)};
        render::render_to_png(plane, buffer.get(), output_file);

        timer_running = false;
        if (timer_thread.joinable()) timer_thread.join();

        const auto end_time{std::chrono::steady_clock::now()};
        const auto total_seconds{
            std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count()
        };

        std::cout << "\rTempo di generazione: "
                  << total_seconds
                  << " s\n";
        std::cout << "Render completato: " << output_file << "\n";
    }
    catch (const std::exception& e) {
        timer_running = false;
        if (timer_thread.joinable()) timer_thread.join();
        std::cerr << "\nErrore: " << e.what() << '\n';
        return 1;
    }

    return 0;
}

#endif // BENCHMARK_MODE
