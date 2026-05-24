#pragma once
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <cmath> // Required for std::log2, std::cos
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
                const float mag_sq = data[i].magnitude_sq;
                
                float mu = static_cast<float>(iter);
                // Safety check: avoid log domain errors
                if (mag_sq > 0.0f) {
                    mu += 2.0f - std::log2(std::log2(mag_sq));
                }

                // Procedural Cosine Palette (Inigo Quilez technique)
                // Multiplier 0.05f controls the frequency of the color bands
                const float t = mu * 0.05f; 

                // Generate smooth RGB sine waves with distinct phase shifts
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
