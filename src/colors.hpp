#pragma once
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <cmath> // Required for std::log2, std::cos
#include "fractal_pl.hpp"

// ... [Mantieni il codice esistente di color_registry, bw e crazy] ...

class smooth {
public:
    static std::unique_ptr<unsigned char[]> apply(const fractal_pl& plane, int maxiter) {
        const auto& data{plane.data()};
        const std::size_t n{data.size()};
        auto buffer{std::make_unique<unsigned char[]>(n * 3)};

        for (std::size_t i{}; i < n; ++i) {
            const int iter{data[i].escapeiter};
            
            if (iter == maxiter) {
                // Interior points mapped to pure black
                buffer[i * 3    ] = 0;
                buffer[i * 3 + 1] = 0;
                buffer[i * 3 + 2] = 0;
            } else {
                // Continuous Potential Algorithm (Smooth Coloring)
                // We use magnitude_sq (|z|^2) to completely avoid a costly std::sqrt.
                // Math proof: log2(log2(|z|)) = log2(0.5 * log2(|z|^2)) = log2(log2(|z|^2)) - 1
                // Therefore: iter + 1 - log2(log2(|z|)) becomes iter + 2 - log2(log2(|z|^2))
                
                const float mag_sq = data[i].magnitude_sq;
                
                float mu = static_cast<float>(iter);
                // Safety check: mag_sq should always be > 4.0 for escaped points
                if (mag_sq > 0.0f) {
                    mu += 2.0f - std::log2(std::log2(mag_sq));
                }

                // Procedural Cosine Palette (Inigo Quilez technique)
                // We map the continuous variable 'mu' to a cyclic color space.
                // The multiplier (0.05f) controls the frequency/density of the color bands.
                const float t = mu * 0.05f; 

                // Generate smooth RGB sine waves with distinct phase shifts.
                // Output is safely mapped to [0, 255] range.
                buffer[i * 3    ] = static_cast<unsigned char>(127.5f * (1.0f + std::cos(t + 0.0f))); // Red
                buffer[i * 3 + 1] = static_cast<unsigned char>(127.5f * (1.0f + std::cos(t + 1.0f))); // Green
                buffer[i * 3 + 2] = static_cast<unsigned char>(127.5f * (1.0f + std::cos(t + 2.0f))); // Blue
            }
        }
        return buffer;
    }
private:
    static inline const bool registered_{
        (color_registry::register_color("smooth", smooth::apply), true)
    };
};

} // Assicurati di chiudere eventuali namespace se ne hai
