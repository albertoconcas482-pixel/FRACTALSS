# FRACTALSS

A highly optimized, memory-efficient C++ Mandelbrot set renderer focused on clean micro-architectural design, high cache locality, and algorithmic precision.

## Architecture & Pipeline

The project enforces a strict separation of concerns, processing data linearly through independent stages:

$$\text{fractal\_pl (Grid)} \longrightarrow \text{mandel\_pipeline (Kernel)} \longrightarrow \text{render (Color & I/O)}$$

* **`fractal_pl`**: Manages the multi-million pixel memory allocation using a flat row-major layout. It contains no geometric state or floating-point positions.
* **`mandel_pipeline`**: The core execution engine. It handles coordinate generation and streams processing into the mathematical primitives.
* **`render`**: Handles presentation concerns by dynamically generating independent RGB pixel arrays and streaming them to disk.

---

## Technical Specifications & Optimizations

### 1. Ultra-Low Memory Footprint (87% RAM Reduction)
The core data structure `fractal_el` has been stripped of all redundant data, including coordinate points (`std::complex<double>`) and helper flags (`bool inside`). 

* Every pixel occupies exactly **4 bytes** of memory (`int escapeiter`), down from the previous 32-byte representation.
* This optimization increases the maximum reachable resolution exponentially, completely eliminating the risk of `std::bad_alloc` on consumer machines.

### 2. Brent's Cycle Detection Algorithm
To isolate interior points of the Mandelbrot set that evade analytical checks, the kernel implements **Brent's Algorithm** for period cycle detection.

* Uses a geometrically expanding tracking window ($power *= 2$) to track reference states.
* Triggers early termination whenever the squared distance falls below the tight spatial tolerance threshold of $\epsilon = 10^{-20}$.
* Guarantees the interception of orbital periods of any arbitrary length without relying on a fixed, hardcoded period factor $k$.

### 3. Strength Reduction & 1D Flat Loop Locality
Nested coordinate loops (row/column pairs) have been completely refactored into a single, flat 1D traversal over the linear vector.

* Pixel coordinates ($cr, ci$) are tracked incrementally ($cr += dx$) rather than being re-multiplied at every index.
* Eliminating floating-point multiplications at the pixel level significantly reduces CPU instruction count.
* Sequential row-major memory writes ensure maximum L1/L2 CPU cache hit rates.
* *Note on Concurrency*: The current strength-reduction state machine introduces a loop-carried dependency. Transitioning to chunk-based block parallelization will require state isolation via stateless coordinate lookup wrappers.

### 4. Open/Closed Presentation Layer
Color schemes are completely decoupled from the data pipeline via a self-registering static lookup registry. New colorizers can be introduced without modifying the core renderer or the `main` loop execution logic.

---

## Output Format

Images are natively written into compressed, lossless **PNG** format using an embedded implementation of `stb_image_write`. Output directory trees are generated dynamically on demand based on the evaluated image path.

---

## Build & Execution

The project requires a compiler fully compliant with **C++20** and **CMake 3.16+**.

```bash
# Create and navigate to the build directory
mkdir build && cd build

# Configure the project in optimized Release mode
cmake .. -DCMAKE_BUILD_TYPE=Release

# Compile the executable
cmake --build .

# Run the renderer
./mandelbrot
