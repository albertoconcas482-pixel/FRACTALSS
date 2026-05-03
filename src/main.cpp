#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <exception>
#include <iostream>
#include <string>
#include <thread>

#include "fractal_pl.hpp"
#include "mandel_pipeline.hpp"
#include "render.hpp"

// ─── benchmark mode ──────────────────────────────────────────────────────────
// Set to 1 to run a CSV benchmark sweep instead of a normal render.
// Output: mode,nx,ny,zoom,maxiter,period_k,plane_ms,mandel_ms,total_ms
//
//   CARDIOID — cardioid+bulb2 check + plain iterate() (no period check)
//   PERIOD   — cardioid+bulb2 check + iterate_with_period(k=20)
//
// Sweep: 6 zoom levels (16 → 16384), FHD resolution (1920×1080).
// Centre fixed at (cx=-1.786440, cy=0.0) — cardioid/bulb-2 junction on real axis.
// maxiter derived per-level from: max(200, floor(1000 * sqrt(2 * log2(zoom))))
#define BENCHMARK_MODE 0

// ─── render zoom mode ────────────────────────────────────────────────────────
// Set to 1 to render one PNG per zoom level for visual inspection.
// Files are saved as zoom_16.png, zoom_64.png, ... zoom_16384.png
// Uses mandel_check (period_k=20) + "crazy" color scheme.
// BENCHMARK_MODE and RENDER_ZOOM_MODE are mutually exclusive — set only one to 1.
#define RENDER_ZOOM_MODE 0

// ─── period_k ────────────────────────────────────────────────────────────────
// Used in normal render mode and RENDER_ZOOM_MODE.
static constexpr int period_k{20};

// ─────────────────────────────────────────────────────────────────────────────

struct BenchConfig {
    double      xmin, xmax, ymin, ymax;
    int         maxiter;
    std::size_t zoom;
};

// 6 zoom levels centred on cx=-1.786440, cy=0.0
// width  = 3.5 / zoom,  height = width * (9/16)
// maxiter = max(200, floor(1000 * sqrt(2 * log2(zoom))))
constexpr std::array<BenchConfig, 6> bench_configs {{
    { -1.895815000000000, -1.677065000000000, -0.061523437500000,  0.061523437500000,  2828, 16    },
    { -1.813783750000000, -1.759096250000000, -0.015380859375000,  0.015380859375000,  3464, 64    },
    { -1.793275937500000, -1.779604062500000, -0.003845214843750,  0.003845214843750,  4000, 256   },
    { -1.788148984375000, -1.784731015625000, -0.000961303710938,  0.000961303710938,  4472, 1024  },
    { -1.786867246093750, -1.786012753906250, -0.000240325927734,  0.000240325927734,  4898, 4096  },
    { -1.786546811523438, -1.786333188476563, -0.000060081481934,  0.000060081481934,  5291, 16384 },
}};

// ─────────────────────────────────────────────────────────────────────────────

#if BENCHMARK_MODE

int main() {
    try {
        constexpr std::size_t nx {1920};
        constexpr std::size_t ny {1080};

        constexpr int bench_period_k {20};

        using clock_type = std::chrono::steady_clock;

        std::cout << "mode,nx,ny,zoom,maxiter,period_k,plane_ms,mandel_ms,total_ms\n";

        // ── CARDIOID baseline (no period check) ──────────────────────────────
        for (const auto& cfg : bench_configs) {
            const auto plane_start  {clock_type::now()};
            fractal_pl plane {cfg.xmin, cfg.xmax, cfg.ymin, cfg.ymax, nx, ny};
            const auto plane_end    {clock_type::now()};

            const auto mandel_start {clock_type::now()};
            mandel_pipeline::mandel_check_cardioid(plane, cfg.maxiter);
            const auto mandel_end   {clock_type::now()};

            const auto plane_ms  {std::chrono::duration_cast<std::chrono::milliseconds>(
                plane_end  - plane_start).count()};
            const auto mandel_ms {std::chrono::duration_cast<std::chrono::milliseconds>(
                mandel_end - mandel_start).count()};

            std::cout << "CARDIOID,"
                      << nx          << ','
                      << ny          << ','
                      << cfg.zoom    << ','
                      << cfg.maxiter << ",0,"
                      << plane_ms    << ','
                      << mandel_ms   << ','
                      << (plane_ms + mandel_ms) << '\n';
        }

        // ── PERIOD sweep (period_k fixed at 20) ──────────────────────────────
        for (const auto& cfg : bench_configs) {
            const auto plane_start  {clock_type::now()};
            fractal_pl plane {cfg.xmin, cfg.xmax, cfg.ymin, cfg.ymax, nx, ny};
            const auto plane_end    {clock_type::now()};

            const auto mandel_start {clock_type::now()};
            mandel_pipeline::mandel_check(plane, cfg.maxiter, bench_period_k);
            const auto mandel_end   {clock_type::now()};

            const auto plane_ms  {std::chrono::duration_cast<std::chrono::milliseconds>(
                plane_end  - plane_start).count()};
            const auto mandel_ms {std::chrono::duration_cast<std::chrono::milliseconds>(
                mandel_end - mandel_start).count()};

            std::cout << "PERIOD,"
                      << nx              << ','
                      << ny              << ','
                      << cfg.zoom        << ','
                      << cfg.maxiter     << ','
                      << bench_period_k  << ','
                      << plane_ms        << ','
                      << mandel_ms       << ','
                      << (plane_ms + mandel_ms) << '\n';
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Errore: " << e.what() << '\n';
        return 1;
    }

    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────

#elif RENDER_ZOOM_MODE

int main() {
    try {
        constexpr std::size_t nx {1920};
        constexpr std::size_t ny {1080};

        const std::string color_scheme {"crazy"};

        for (const auto& cfg : bench_configs) {
            const std::string filename {"zoom_" + std::to_string(cfg.zoom) + ".png"};

            std::cout << "Rendering zoom=" << cfg.zoom
                      << "  maxiter=" << cfg.maxiter
                      << "  -> " << filename << " ..." << std::flush;

            fractal_pl plane {cfg.xmin, cfg.xmax, cfg.ymin, cfg.ymax, nx, ny};
            mandel_pipeline::mandel_check(plane, cfg.maxiter, period_k);

            auto buffer {render::render_color(plane, color_scheme)};
            render::render_to_png(plane, buffer.get(), filename);

            std::cout << " done\n";
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Errore: " << e.what() << '\n';
        return 1;
    }

    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────

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

#endif // BENCHMARK_MODE / RENDER_ZOOM_MODE / normal
