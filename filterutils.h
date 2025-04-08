#ifndef FILTERUTILS_H
#define FILTERUTILS_H

#endif // FILTERUTILS_H

#pragma once

#include "rgba.h"
#include <cstdint>
#include <vector>

// FilterUtils header file from lab
namespace FilterUtils {

inline std::uint8_t floatToUint8(float x);

void convolve2D(std::vector<RGBA>& data,
                int width,
                int height,
                const std::vector<float>& kernel);

void convolveHorizontal(std::vector<RGBAf>& data, int width, int height, const std::vector<float>& kernel);

void convolveVertical(std::vector<RGBAf>& data, int width, int height, const std::vector<float>& kernel);

std::uint8_t clamp(float num);

RGBA getPixelReflected(std::vector<RGBA> &data, int width, int height, int x, int y);

float triangle(float radius, int x);

}
