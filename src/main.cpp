#include <array>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <exception>
#include <iostream>
#include <string>
#include <atomic>
#include <thread>

#include "fractal_pl.hpp"
#include "mandel_set.hpp"
#include "render.hpp"

// ─── benchmark mode ──────────────────────────────────────────────────────────
// Set to 1 to run a CSV benchmark sweep instead of a normal render.
// Output: mode,nx,ny,zoom,maxiter,period_k,plane_ms,mandel_ms,total_ms
//
//   MAIN — cardioid+bulb2 check + plain iterate(), all inside mandel_set::mandel_check()
//
// Sweep: 6 zoom levels (16 → 16384), FHD resolution (1920×1080).
// Centre fixed at (cx=-1.786440, cy=0.0) — cardioid/bulb-2 junction on real axis.
// maxiter derived per-level from: max(200, floor(1000 * sqrt(2 * log2(zoom))))
// period_k is always 0 for this branch (no period-checking algorithm).
#define BENCHMARK_MODE 0

// ─── render zoom mode ────────────────────────────────────────────────────────
// Set to 1 to render one PNG per zoom level for visual inspection.
//
// Pipeline: mandel_set::mandel_check (cardioid+bulb2 early exit + plain iterate,
//           all inside the monolithic method — no period-check overhead).
//           Uses "crazy" color scheme.
//
// Output files: /home/alberto/Documents/C++/Fractals/images/buff_bench_FHD_N.png
//               where N = 1..6 matching bench_configs order (zoom 16 → 16384).
//
// Timing: plane_ms and mandel_ms are measured and printed to stdout per level.
//         Render + write happen after the measurement window closes.
//
// BENCHMARK_MODE and RENDER_ZOOM_MODE are mutually exclusive — set only one to 1.
#define RENDER_ZOOM_MODE 0

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

        using clock_type = std::chrono::steady_clock;

        std::cout << "mode,nx,ny,zoom,maxiter,period_k,plane_ms,mandel_ms,total_ms\n";

        for (const auto& cfg : bench_configs) {
            const auto plane_start {clock_type::now()};
            fractal_pl plane {cfg.xmin, cfg.xmax, cfg.ymin, cfg.ymax, nx, ny};
            const auto plane_end   {clock_type::now()};

            const auto mandel_start {clock_type::now()};
            mandel_set::mandel_check(plane, cfg.maxiter);
            const auto mandel_end   {clock_type::now()};

            const auto plane_ms  {std::chrono::duration_cast<std::chrono::milliseconds>(
                plane_end  - plane_start).count()};
            const auto mandel_ms {std::chrono::duration_cast<std::chrono::milliseconds>(
                mandel_end - mandel_start).count()};

            std::cout << "MAIN,"
                      << nx           << ','
                      << ny           << ','
                      << cfg.zoom     << ','
                      << cfg.maxiter  << ",0,"
                      << plane_ms     << ','
                      << mandel_ms    << ','
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

/*
 * Renders one PNG per zoom level for visual inspection.
 *
 * Pipeline: mandel_set::mandel_check (cardioid + bulb-2 early exit + plain
 * iterate, monolithic method). Matches the MAIN label used in BENCHMARK_MODE.
 *
 * Timing: plane construction and mandel computation are measured separately.
 * Render + write happen outside the measurement window so they do not
 * contaminate mandel_ms.
 *
 * Output: /home/alberto/Documents/C++/Fractals/images/buff_bench_FHD_N.png
 *         N = 1..6, matching bench_configs order (zoom 16 → 16384).
 */
int main() {
    try {
        constexpr std::size_t nx {1920};
        constexpr std::size_t ny {1080};

        const std::string color_scheme  {"crazy"};
        const std::string images_dir    {"/home/alberto/Documents/C++/Fractals/images/"};

        using clock_type = std::chrono::steady_clock;

        for (std::size_t idx = 0; idx < bench_configs.size(); ++idx) {
            const auto& cfg = bench_configs[idx];

            const std::string filename {
                images_dir + "buff_bench_FHD_" + std::to_string(idx + 1) + ".png"
            };

            std::cout << "[" << (idx + 1) << "/6] zoom=" << cfg.zoom
                      << "  maxiter=" << cfg.maxiter << "\n";

            // ── measure: plane construction ───────────────────────────────────
            const auto plane_start  {clock_type::now()};
            fractal_pl plane {cfg.xmin, cfg.xmax, cfg.ymin, cfg.ymax, nx, ny};
            const auto plane_end    {clock_type::now()};

            // ── measure: mandel computation (monolithic pipeline) ─────────────
            const auto mandel_start {clock_type::now()};
            mandel_set::mandel_check(plane, cfg.maxiter);
            const auto mandel_end   {clock_type::now()};

            const auto plane_ms  {std::chrono::duration_cast<std::chrono::milliseconds>(
                plane_end  - plane_start).count()};
            const auto mandel_ms {std::chrono::duration_cast<std::chrono::milliseconds>(
                mandel_end - mandel_start).count()};

            std::cout << "  plane_ms  = " << plane_ms  << " ms\n";
            std::cout << "  mandel_ms = " << mandel_ms << " ms\n";

            // ── outside measurement window: render + write ────────────────────
            render::render_color(plane, color_scheme);
            render::render_to_ppm(
                plane,
                std::string("/home/alberto/Documents/C++/Fractals/images/buff_bench_FHD_")
                    + std::to_string(idx + 1) + ".ppm"
            );

            std::cout << "  -> " << filename << "  done\n\n";
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
    catch (const std::exception& e) {        timer_running = false;

        if (timer_thread.joinable()) {
            timer_thread.join();
        }

        std::cerr << "\nErrore: " << e.what() << '\n';
        return 1;
    }

    return 0;
}

#endif // BENCHMARK_MODE / RENDER_ZOOM_MODE / normal
