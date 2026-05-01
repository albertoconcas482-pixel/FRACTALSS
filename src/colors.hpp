#pragma once
#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "fractal_pl.hpp"

/*
 * Registry for color functions.
 *
 * color_registry is a static map that associates a name (string)
 * to a coloring function. New color schemes register themselves
 * automatically at program startup via the static member trick
 * used in each colorizer class (see bw, crazy below).
 *
 * Usage:
 *   - register_color: adds a named color function to the registry
 *   - apply_color:    looks up the name and applies the function to the plane
 *
 * To add a new color scheme, define a new class with a static apply()
 * method and a static registered_ member that calls register_color.
 */
class color_registry {
public:
    using color_function = std::function<void(fractal_pl&)>;

    static void register_color(const std::string& name, color_function function) {
        registry()[name] = std::move(function);
    }

    static void apply_color(const std::string& name, fractal_pl& plane) {
        const auto it{registry().find(name)};

        if (it == registry().end()) {
            throw std::invalid_argument{"unsupported color scheme"};
        }

        it->second(plane);
    }

private:
    /*
     * Returns the singleton map. Defined as a static local variable
     * to guarantee initialization before first use.
     */
    static std::unordered_map<std::string, color_function>& registry() {
        static std::unordered_map<std::string, color_function> color_map{};
        return color_map;
    }
};

/*
 * Black and white color scheme.
 * Inside points -> black (0, 0, 0)
 * Outside points -> white (255, 255, 255)
 */
class bw {
public:
    static void apply(fractal_pl& plane) {
        auto& data{plane.data()};

        for (auto& el : data) {
            if (el.inside) {
                el.r = {};
                el.g = {};
                el.b = {};
            } else {
                el.r = 255;
                el.g = 255;
                el.b = 255;
            }
        }
    }

private:
    /*
     * Self-registration: this static member is initialized at program startup,
     * which triggers the call to register_color before main() runs.
     * This pattern allows adding new color schemes without modifying
     * any existing code — just define the class and it registers itself.
     */
    static inline const bool registered_{
        (color_registry::register_color("bw", bw::apply), true)
    };
};

/*
 * Psychedelic color scheme based on escape iteration count.
 * Inside points -> black (0, 0, 0)
 * Outside points -> RGB computed from escapeiter with different
 *                   multipliers and offsets per channel to create
 *                   a colorful cycling effect.
 */
class crazy {
public:
    static void apply(fractal_pl& plane) {
        auto& data{plane.data()};

        for (auto& el : data) {
            if (el.inside) {
                el.r = {};
                el.g = {};
                el.b = {};
            } else {
                const int iter{el.escapeiter};

                el.r = static_cast<unsigned char>((iter * 9) % 256);
                el.g = static_cast<unsigned char>((iter * 7 + 80) % 256);
                el.b = static_cast<unsigned char>((iter * 13 + 160) % 256);
            }
        }
    }

private:
    // Self-registration (see bw::registered_ for explanation)
    static inline const bool registered_{
        (color_registry::register_color("crazy", crazy::apply), true)
    };
};
