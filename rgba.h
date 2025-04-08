#pragma once

#include <cstdint>

struct RGBA {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    std::uint8_t a = 255;
};

// RGBAf struct from lab
struct RGBAf {
    float r;
    float g;
    float b;
    float a = 255;


    RGBA toRGBA() {
        RGBA result;
        result.r = r;
        result.g = g;
        result.b = b;
        result.a = a;


        if (r > 255) {
            result.r = 255;
        }
        if (g > 255) {
            result.g = 255;
        }
        if (b > 255) {
            result.b = 255;
        }



        return result;
    }

    static RGBAf fromRGBA(RGBA pixel) {
        // TODO: Uncomment after Task 7!
        return RGBAf{ float(pixel.r), float(pixel.g), float(pixel.b), float(pixel.a) };
    }

};
