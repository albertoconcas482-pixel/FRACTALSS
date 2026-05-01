#pragma once
#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "fractal_pl.hpp"

class color_registry {
public:
    using color_function = std::function<void(fractal_pl&)>;

    static void register_color(const std::string& name, color_function function) {
        registry()[name] = std::move(function);
    }

    static void apply_color(const std::string& name, fractal_pl& plane) {
        const auto it{registry().find(name)};

        if (it == registry().end()) {
            throw std::invalid_argument{"colorazione non supportata"};
        }

        it->second(plane);
    }

private:
    static std::unordered_map<std::string, color_function>& registry() {
        static std::unordered_map<std::string, color_function> color_map{};
        return color_map;
    }
};

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
    static inline const bool registered_{
        (color_registry::register_color("bw", bw::apply), true)
    };
};

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
    static inline const bool registered_{
        (color_registry::register_color("crazy", crazy::apply), true)
    };
};
