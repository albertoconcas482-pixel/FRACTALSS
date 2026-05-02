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

// ─── benchmark mode ─────────────────────────────────────────────────────────
// Set to 1 to run a CSV benchmark sweep instead of a normal render.
// Output: mode,nx,ny,maxiter,period_k,plane_ms,mandel_ms,total_ms
//
//   CARDIOID — cardioid+bulb2 check, then plain iterate() (no period check)
//   PERIOD   — cardioid+bulb2 check, then iterate_with_period()
//
// period_k is swept only for PERIOD rows; CARDIOID rows print period_k=0.
#define BENCHMARK_MODE 0

// ─── period_k ────────────────────────────────────────────────────────────────
// Used in normal render mode only.
static constexpr int period_k{20};

// ─────────────────────────────────────────────────────────────────────────────

#if BENCHMARK_MODE

int main() {
    try {
        constexpr double xmin{-0.80};
        constexpr double xmax{-0.70};
        constexpr double ymin{ 0.05};
        constexpr double ymax{ 0.15};

        const std::vector<std::size_t> resolution_values{2000, 5000, 8000, 10000};
        const std::vector<int>         maxiter_values   {50, 100, 200, 400, 800};
        const std::vector<int>         period_k_values  {10, 20, 50, 100};

        using clock_type = std::chrono::steady_clock;

        std::cout << "mode,nx,ny,maxiter,period_k,plane_ms,mandel_ms,total_ms\n";

        // ── CARDIOID baseline (no period check) ──────────────────────────────
        for (std::size_t resolution : resolution_values) {
            for (int maxiter : maxiter_values) {
                const auto total_start{clock_type::now()};

                const auto plane_start{clock_type::now()};
                fractal_pl plane{xmin, xmax, ymin, ymax, resolution, resolution};
                const auto plane_end{clock_type::now()};

                const auto mandel_start{clock_type::now()};
                mandel_pipeline::mandel_check_cardioid(plane, maxiter);
                const auto mandel_end{clock_type::now()};

                const auto total_end{clock_type::now()};

                const auto plane_ms{std::chrono::duration_cast<std::chrono::milliseconds>(
                    plane_end - plane_start).count()};
                const auto mandel_ms{std::chrono::duration_cast<std::chrono::milliseconds>(
                    mandel_end - mandel_start).count()};
                const auto total_ms{std::chrono::duration_cast<std::chrono::milliseconds>(
                    total_end - total_start).count()};

                std::cout << "CARDIOID,"
                          << resolution << ',' << resolution << ','
                          << maxiter << ",0,"
                          << plane_ms << ',' << mandel_ms << ','
                          << total_ms << '\n';
            }
        }

        // ── PERIOD sweep ─────────────────────────────────────────────────────
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

                    std::cout << "PERIOD,"
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
 */
int main() {
    std::atomic<bool> timer_running{false};
    std::thread timer_thread;

    try {
        constexpr double xmin  {-1.790};
        constexpr double xmax  {-1.782};
        constexpr double ymin  {-0.003};
        constexpr double ymax  { 0.003};

        constexpr int render_width  {1920};
        constexpr int render_height {1440};

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
