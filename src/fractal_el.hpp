#pragma once
#include <complex>

struct fractal_el {
    std::complex<double> c{};
    bool inside{};
    int escapeiter{};
    unsigned char r{};
    unsigned char g{};
    unsigned char b{};
};
