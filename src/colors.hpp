#pragma once
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <cmath> 
#include <array>
#include <vector>
#include <algorithm> // <-- AGGIUNGI QUESTA RIGA per std::clamp
#include "fractal_pl.hpp"

class color_registry {
public:
    using color_function = std::function<
        std::unique_ptr<unsigned char[]>(const fractal_pl&, int)
    >;

    static void register_color(const std::string& name, color_function function) {
        registry()[name] = std::move(function);
    }

    static std::unique_ptr<unsigned char[]> apply_color(
        const std::string& name, const fractal_pl& plane, int maxiter)
    {
        const auto it{registry().find(name)};
        if (it == registry().end()) {
            throw std::invalid_argument{"unsupported color scheme"};
        }
        return it->second(plane, maxiter);
    }

private:
    static std::unordered_map<std::string, color_function>& registry() {
        static std::unordered_map<std::string, color_function> color_map{};
        return color_map;
    }
};

class bw {
public:
    static std::unique_ptr<unsigned char[]> apply(const fractal_pl& plane, int maxiter) {
        const auto& data{plane.data()};
        const std::size_t n{data.size()};
        auto buffer{std::make_unique<unsigned char[]>(n * 3)};

        for (std::size_t i{}; i < n; ++i) {
            const unsigned char value = (data[i].escapeiter == maxiter) ? 0u : 255u;
            buffer[i * 3    ] = value;
            buffer[i * 3 + 1] = value;
            buffer[i * 3 + 2] = value;
        }
        return buffer;
    }
private:
    static inline const bool registered_{
        (color_registry::register_color("bw", bw::apply), true)
    };
};

class crazy {
public:
    static std::unique_ptr<unsigned char[]> apply(const fractal_pl& plane, int maxiter) {
        const auto& data{plane.data()};
        const std::size_t n{data.size()};
        auto buffer{std::make_unique<unsigned char[]>(n * 3)};

        for (std::size_t i{}; i < n; ++i) {
            const int iter{data[i].escapeiter};
            if (iter == maxiter) {
                buffer[i * 3    ] = 0;
                buffer[i * 3 + 1] = 0;
                buffer[i * 3 + 2] = 0;
            } else {
                buffer[i * 3    ] = static_cast<unsigned char>((iter * 9)        % 256);
                buffer[i * 3 + 1] = static_cast<unsigned char>((iter * 7  + 80)  % 256);
                buffer[i * 3 + 2] = static_cast<unsigned char>((iter * 13 + 160) % 256);
            }
        }
        return buffer;
    }
private:
    static inline const bool registered_{
        (color_registry::register_color("crazy", crazy::apply), true)
    };
};

class smooth {
private:
    // Generate a 256-color Lookup Table (LUT) approximating Matplotlib's 'inferno'
    // This is computed only once at startup to avoid runtime overhead.
    static std::array<std::array<unsigned char, 3>, 256> generate_lut() {
        std::array<std::array<unsigned char, 3>, 256> lut{};
        
        // Keyframes for the Inferno colormap
        const std::vector<std::array<float, 3>> keys = {
            {0.0f,   0.0f,   4.0f},   // Black/Dark Blue
            {66.0f,  10.0f,  104.0f}, // Dark Purple
            {147.0f, 38.0f,  103.0f}, // Magenta
            {221.0f, 81.0f,  58.0f},  // Red-Orange
            {252.0f, 165.0f, 10.0f},  // Yellow-Orange
            {252.0f, 255.0f, 164.0f}  // Bright Yellow/White
        };
        
        const int num_segments = keys.size() - 1;
        
        // Linearly interpolate between keyframes to build the 256 RGB array
        for (int i = 0; i < 256; ++i) {
            float t = static_cast<float>(i) / 255.0f;
            float scaled_t = t * num_segments;
            int idx = static_cast<int>(scaled_t);
            if (idx >= num_segments) idx = num_segments - 1;
            float frac = scaled_t - static_cast<float>(idx);
            
            lut[i][0] = static_cast<unsigned char>(keys[idx][0] + frac * (keys[idx+1][0] - keys[idx][0]));
            lut[i][1] = static_cast<unsigned char>(keys[idx][1] + frac * (keys[idx+1][1] - keys[idx][1]));
            lut[i][2] = static_cast<unsigned char>(keys[idx][2] + frac * (keys[idx+1][2] - keys[idx][2]));
        }
        return lut;
    }

    // Static initialization of the LUT (zero-cost during the hot render loop)
    static inline const std::array<std::array<unsigned char, 3>, 256> lut_ = generate_lut();

public:
    static std::unique_ptr<unsigned char[]> apply(const fractal_pl& plane, int maxiter) {
        const auto& data{plane.data()};
        const std::size_t n{data.size()};
        auto buffer{std::make_unique<unsigned char[]>(n * 3)};

        float min_mu = 1e9f;
        float max_mu = -1e9f;

        // --- PASS 1: Global Min/Max Discovery ---
        // Find the actual range of smooth iterations to normalize colors dynamically
        for (std::size_t i = 0; i < n; ++i) {
            // Only process escaping points with valid magnitude.
            // mag_sq > 1.0f is a safety net against log2(log2(x)) returning NaN or negatives.
            if (data[i].escapeiter < maxiter && data[i].magnitude_sq > 1.0f) {
                float mu = static_cast<float>(data[i].escapeiter) + 2.0f - std::log2(std::log2(data[i].magnitude_sq));
                if (mu < min_mu) min_mu = mu;
                if (mu > max_mu) max_mu = mu;
            }
        }

        // Safety check: fallback if no points escaped or the plane is completely uniform
        if (max_mu <= min_mu) {
            max_mu = min_mu + 1.0f; 
        }
        
        // Precompute the inverse range for fast multiplication (avoids expensive division per pixel)
        const float mu_range_inv = 1.0f / (max_mu - min_mu);

        // --- PASS 2: Color Mapping and Rendering ---
        for (std::size_t i = 0; i < n; ++i) {
            if (data[i].escapeiter == maxiter || data[i].magnitude_sq <= 1.0f) {
                // Interior points mapped to pure black
                buffer[i * 3    ] = 0;
                buffer[i * 3 + 1] = 0;
                buffer[i * 3 + 2] = 0;
            } else {
                // 1. Calculate smooth iteration
                float mu = static_cast<float>(data[i].escapeiter) + 2.0f - std::log2(std::log2(data[i].magnitude_sq));
                
                // 2. Normalize to [0.0, 1.0] domain
                float norm_mu = (mu - min_mu) * mu_range_inv;
                
                // 3. Map to LUT index [0, 255] with safety clamping
                int lut_idx = static_cast<int>(norm_mu * 255.0f);
                if (lut_idx < 0) lut_idx = 0;
                if (lut_idx > 255) lut_idx = 255;
                
                // 4. Assign O(1) precomputed RGB color
                buffer[i * 3    ] = lut_[lut_idx][0]; // Red
                buffer[i * 3 + 1] = lut_[lut_idx][1]; // Green
                buffer[i * 3 + 2] = lut_[lut_idx][2]; // Blue
            }
        }
        return buffer;
    }
private:
    static inline const bool registered_{
        (color_registry::register_color("smooth", smooth::apply), true)
    };
};


class smooth_lava {
private:
    // Generate a 256-color LUT for a "Lava" gradient
    static std::array<std::array<unsigned char, 3>, 256> generate_lut() {
        std::array<std::array<unsigned char, 3>, 256> lut{};
        
        // Keyframes: Black -> Dark Red -> Bright Orange -> Yellow -> White
        const std::vector<std::array<float, 3>> keys = {
            {0.0f,   0.0f,   0.0f},   // Black
            {120.0f, 0.0f,   0.0f},   // Dark Red
            {255.0f, 60.0f,  0.0f},   // Bright Orange
            {255.0f, 200.0f, 0.0f},   // Yellow-Orange
            {255.0f, 255.0f, 255.0f}  // White
        };
        
        const int num_segments = keys.size() - 1;
        
        // Linearly interpolate between the keyframes
        for (int i = 0; i < 256; ++i) {
            float t = static_cast<float>(i) / 255.0f;
            float scaled_t = t * num_segments;
            int idx = std::min(static_cast<int>(scaled_t), num_segments - 1);
            float frac = scaled_t - static_cast<float>(idx);
            
            for(int c = 0; c < 3; ++c) {
                lut[i][c] = static_cast<unsigned char>(keys[idx][c] + frac * (keys[idx+1][c] - keys[idx][c]));
            }
        }
        return lut;
    }
    
    // Pre-calculate the LUT at compile-time/startup
    static inline const std::array<std::array<unsigned char, 3>, 256> lut_ = generate_lut();

public:
    static std::unique_ptr<unsigned char[]> apply(const fractal_pl& plane, int maxiter) {
        const auto& data{plane.data()};
        const std::size_t n{data.size()};
        auto buffer{std::make_unique<unsigned char[]>(n * 3)};

        float min_mu = 1e9f, max_mu = -1e9f;

        // --- PASS 1: Range detection ---
        for (std::size_t i = 0; i < n; ++i) {
            if (data[i].escapeiter < maxiter && data[i].magnitude_sq > 1.0f) {
                float mu = static_cast<float>(data[i].escapeiter) + 2.0f - std::log2(std::log2(data[i].magnitude_sq));
                if (mu < min_mu) min_mu = mu;
                if (mu > max_mu) max_mu = mu;
            }
        }

        if (max_mu <= min_mu) max_mu = min_mu + 1.0f;
        const float mu_range_inv = 1.0f / (max_mu - min_mu);

        // --- PASS 2: O(1) Rendering ---
        for (std::size_t i = 0; i < n; ++i) {
            if (data[i].escapeiter == maxiter || data[i].magnitude_sq <= 1.0f) {
                buffer[i*3] = buffer[i*3+1] = buffer[i*3+2] = 0;
            } else {
                float mu = static_cast<float>(data[i].escapeiter) + 2.0f - std::log2(std::log2(data[i].magnitude_sq));
                
                // Map the normalized value to the [0, 255] LUT index
                int idx = static_cast<int>((mu - min_mu) * mu_range_inv * 255.0f);
                idx = std::clamp(idx, 0, 255); 
                
                buffer[i*3]   = lut_[idx][0];
                buffer[i*3+1] = lut_[idx][1];
                buffer[i*3+2] = lut_[idx][2];
            }
        }
        return buffer;
    }
private:
    // Auto-register the color scheme
    static inline const bool registered_{
        (color_registry::register_color("smooth_lava", smooth_lava::apply), true)
    };
};
