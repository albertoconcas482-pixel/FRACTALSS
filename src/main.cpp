#include <atomic>
#include <chrono>
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
 */
int main() {
    std::atomic<bool> timer_running {false};
    std::thread timer_thread;

    try {
    fractal_pl plane(-0.835, -0.715, 0.065, 0.155, 12000, 9000);
	  constexpr int maxiter {30000};
        std::string name {"crazy"};
    

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
