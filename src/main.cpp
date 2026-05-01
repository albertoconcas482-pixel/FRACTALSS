#include <atomic>
#include <chrono>
#include <cmath>
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
 * This file is intentionally kept flexible: viewport, resolution,
 * maxiter and color scheme are configured here and change frequently.
 *
 * maxiter is derived from zoom level using the empirical formula:
 *   zoom   = 3.5 / (xmax - xmin)
 *   maxiter = base * sqrt(2 * log2(zoom))
 * where base = 200 is a tunable quality factor.
 */
int main() {
    std::atomic<bool> timer_running {false};
    std::thread timer_thread;

    try {
        // --- viewport: giunzione cardioide/bulbo-2, coda a (-1.786, 0) ---
        constexpr double xmin  {-1.790};
        constexpr double xmax  {-1.782};
        constexpr double ymin  {-0.004};
        constexpr double ymax  {  0.004};

        // maxiter calibrated on zoom level
        constexpr double base_quality {200.0};
        const double zoom    { 3.5 / (xmax - xmin) };
        const int    maxiter { static_cast<int>(base_quality * std::sqrt(2.0 * std::log2(zoom))) };

        fractal_pl plane(xmin, xmax, ymin, ymax, 12000, 9000);
        std::string name {"cardioide_tail"};

        std::cout << "zoom   = " << zoom    << "\n";
        std::cout << "maxiter= " << maxiter << "\n";

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
        render::render_color(plane, name);
        render::render_to_ppm(plane, name);

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

        std::cout << "Render completato: " << name << ".ppm\n";
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
