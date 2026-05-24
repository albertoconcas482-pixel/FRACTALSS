#include "mandel_pipeline.hpp"
#include "mandel_set.hpp"

#include <omp.h>      // Required for OpenMP API functions
#include <iostream>   // Required for console telemetry output
#include <vector>     // Required for the thread workload tracker
#include <iomanip>    // Required for pretty-printing percentages

void mandel_pipeline::mandel_check(fractal_pl& plane, int maxiter) {
    auto& data{plane.data()};
    const std::size_t nx{plane.nx()};
    const std::size_t ny{plane.ny()};

    // Cache limits to avoid repetitive function calls
    const double xmin = plane.xmin();
    const double ymax = plane.ymax();
    const double dx = (plane.xmax() - xmin) / static_cast<double>(nx - 1);
    const double dy = (ymax - plane.ymin()) / static_cast<double>(ny - 1);

    // Retrieve the maximum number of hardware threads OpenMP will allocate
    const int num_threads = omp_get_max_threads();
    
    // Allocate a zero-initialized vector to track the workload (rows processed) per thread.
    // Thread safety: Each thread will uniquely write to its own index.
    // False sharing is not an issue here because the counter is updated 
    // only once per row, not per pixel.
    std::vector<int> rows_per_thread(num_threads, 0);

    // Enter the parallel region
    #pragma omp parallel
    {
        // Get the unique hardware ID of the current thread (0 to num_threads - 1)
        const int thread_id = omp_get_thread_num();

        // Distribute the loop iterations (rows) dynamically among the spawned threads.
        // The '1' specifies the chunk size: one row requested per idle thread.
        #pragma omp for schedule(dynamic, 1)
        for (std::size_t row = 0; row < ny; ++row) {
            
            // Telemetry: Track that this specific thread picked up this row
            rows_per_thread[thread_id]++;

            // Loop-Invariant Code Motion (Hoisting)
            const double ci = ymax - static_cast<double>(row) * dy;

            for (std::size_t col = 0; col < nx; ++col) {
                
                // Linear flat index computation for contiguous memory writes
                const std::size_t i = row * nx + col;
                const double cr = xmin + static_cast<double>(col) * dx;

                if (mandel_set::is_in_cardioid(cr, ci) || mandel_set::is_in_period2_bulb(cr, ci)) {
                    data[i].escapeiter = maxiter;
                    data[i].magnitude_sq = 0.0f; // Interior points do not escape
                } else {
                    auto result = mandel_set::iterate_with_period(cr, ci, maxiter);
                    data[i].escapeiter = result.escapeiter;
                    data[i].magnitude_sq = result.magnitude_sq;
                }
            }
        }
    } // Implicit OpenMP barrier: all threads wait here until the entire grid is finished

    // --- Telemetry Report Generation ---
    std::cout << "\n--- OpenMP Thread Workload Distribution ---\n";
    int total_rows = 0;
    
    for (int t = 0; t < num_threads; ++t) {
        const double percentage = (static_cast<double>(rows_per_thread[t]) / ny) * 100.0;
        
        std::cout << "Thread [" << std::setw(2) << t << "] processed: " 
                  << std::setw(5) << rows_per_thread[t] << " rows "
                  << "(" << std::fixed << std::setprecision(2) << std::setw(5) << percentage << "%)\n";
                  
        total_rows += rows_per_thread[t];
    }
    
    std::cout << "-------------------------------------------\n";
    std::cout << "Total rows successfully computed: " << total_rows << " / " << ny << "\n\n";
}
