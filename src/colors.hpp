#pragma once
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "fractal_pl.hpp"

/*
 * Registry for color functions.
 *
 * Each color function receives the plane (read-only), allocates
 * a buffer of nx*ny*3 bytes, fills it with RGB triplets and
 * returns it as a unique_ptr<unsigned char[]>.
 *
 * The caller (render_color) owns the buffer and passes it to
 * render_to_png which feeds it directly to stbi_write_png.
 */
class color_registry {
public:
    using color_function = std::function<
        std::unique_ptr<unsigned char[]>(const fractal_pl&)
    >;

    static void register_color(const std::string& name, color_function function) {
        registry()[name] = std::move(function);
    }

    static std::unique_ptr<unsigned char[]> apply_color(
        const std::string& name, const fractal_pl& plane)
    {
        const auto it{registry().find(name)};
        if (it == registry().end()) {
            throw std::invalid_argument{"unsupported color scheme"};
        }
        return it->second(plane);
    }

private:
    static std::unordered_map<std::string, color_function>& registry() {
        static std::unordered_map<std::string, color_function> color_map{};
        return color_map;
    }
};

/*
 * Black and white color scheme.
 * Inside  -> black (0, 0, 0)
 * Outside -> white (255, 255, 255)
 */
class bw {
public:
    static std::unique_ptr<unsigned char[]> apply(const fractal_pl& plane) {
        const auto& data{plane.data()};
        const std::size_t n{data.size()};
        auto buffer{std::make_unique<unsigned char[]>(n * 3)};

        for (std::size_t i{}; i < n; ++i) {
            // = instead of {} avoids narrowing warning: 0u/255u are unsigned int,
            // the implicit truncation to unsigned char is intentional and safe.
            const unsigned char value = data[i].inside ? 0u : 255u;
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

/*
 * Psychedelic color scheme based on escape iteration count.
 * Inside  -> black (0, 0, 0)
 * Outside -> RGB computed from escapeiter with different
 *            multipliers and offsets per channel.
 */
class crazy {
public:
    static std::unique_ptr<unsigned char[]> apply(const fractal_pl& plane) {
        const auto& data{plane.data()};
        const std::size_t n{data.size()};
        auto buffer{std::make_unique<unsigned char[]>(n * 3)};

        for (std::size_t i{}; i < n; ++i) {
            if (data[i].inside) {
                buffer[i * 3    ] = 0;
                buffer[i * 3 + 1] = 0;
                buffer[i * 3 + 2] = 0;
            } else {
                const int iter{data[i].escapeiter};
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
