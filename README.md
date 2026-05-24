# FRACTALSS

A highly optimized, memory-efficient C++ Mandelbrot set renderer focused on clean micro-architectural design, high cache locality, and algorithmic precision.

## Architecture & Pipeline

The project enforces a strict separation of concerns, processing data linearly through independent stages:

$$\text{fractal\_pl (Grid)} \longrightarrow \text{mandel\_pipeline (Kernel)} \longrightarrow \text{render (Color & I/O)}$$

* **`fractal_pl`**: Manages the multi-million pixel memory allocation using a flat row-major layout. It contains no geometric state or floating-point positions.
* **`mandel_pipeline`**: The core execution engine. It handles coordinate generation, loop-invariant code hoisting, and multi-core work distribution.
* **`render`**: Handles presentation concerns by dynamically generating independent RGB pixel arrays and streaming them to disk.

---

## Technical Specifications & Optimizations

### 1. Ultra-Low Memory Footprint & Cache Alignment
The core data structure `fractal_el` has been stripped of all redundant coordinate points (`std::complex<double>`) and helper flags. 

* Every pixel occupies exactly **8 bytes** of memory: an `int escapeiter` and a `float magnitude_sq`.
* The `magnitude_sq` stores the squared magnitude ($|z|^2$) at the time of escape, enabling downstream smooth-coloring algorithms without polluting the hot compute loop with expensive hardware square-root instructions.
* This optimization yields a 75% RAM reduction compared to standard 32-byte implementations, remaining perfectly aligned to standard 64-byte CPU cache lines.

### 2. Brent's Cycle Detection Algorithm
To isolate interior points of the Mandelbrot set that evade analytical checks, the kernel implements **Brent's Algorithm** for period cycle detection.

* Uses a geometrically expanding tracking window to track reference states.
* Triggers early termination whenever the squared distance falls below the tight spatial tolerance threshold of $\epsilon = 10^{-20}$.
* Guarantees the interception of orbital periods of any arbitrary length without relying on a fixed, hardcoded period factor.

### 3. Loop-Invariant Hoisting & Dynamic Parallelization
Nested coordinate loops have been architected to maximize both compiler vectorization and multi-core saturation.

* **Loop-Invariant Code Motion**: The imaginary coordinate (`ci`) is hoisted out of the inner loop and calculated only once per row, drastically reducing the floating-point instruction count per pixel.
* **OpenMP Dynamic Scheduling**: The outer loop is parallelized using `#pragma omp parallel for schedule(dynamic, 1)`. This acts as a highly efficient work-stealing queue, ensuring perfect load balancing across all CPU cores despite the extreme computational variance between the empty cardioid regions and the highly dense fractal filaments.
* Sequential row-major memory writes inside the inner loop ensure maximum L1/L2 CPU cache hit rates and prevent multithreaded false sharing.

### 4. Open/Closed Presentation Layer
Color schemes are completely decoupled from the data pipeline via a self-registering static lookup registry. New colorizers can be introduced without modifying the core renderer or the `main` loop execution logic.

---

## Output Format

Images are natively written into compressed, lossless **PNG** format using an embedded implementation of `stb_image_write`. Output directory trees are generated dynamically on demand based on the evaluated image path.

---

## Build & Execution

The project requires a compiler fully compliant with **C++20**, **CMake 3.16+**, and **OpenMP**.

```bash
# Create and navigate to the build directory
mkdir build && cd build

# Configure the project in optimized Release mode
cmake .. -DCMAKE_BUILD_TYPE=Release

# Compile the executable (utilizing all available cores)
cmake --build . -j

# Run the renderer
./mandelbrot
