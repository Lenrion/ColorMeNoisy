#include "filterutils.h"
#include <algorithm>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <vector>

// FilterUtils from lab
namespace FilterUtils {

inline std::uint8_t floatToUint8(float x) {
    return round(x * 255.f);
}

// getPixelRepeated(), getPixelReflected(), and getPixelWrapped() all
// have the same input arguments:
//
// data   - the image's data
// width  - the image's width
// height - the image's height
// x      - the x coordinate of the pixel you're attempting to access
// y      - the y coordinate of the pixel you're attempting to access

// Repeats the pixel on the edge of the image such that A,B,C,D looks like ...A,A,A,B,C,D,D,D...
RGBA getPixelRepeated(std::vector<RGBA> &data, int width, int height, int x, int y) {
    int newX = (x < 0) ? 0 : std::min(x, width  - 1);
    int newY = (y < 0) ? 0 : std::min(y, height - 1);
    return data[width * newY + newX];
}

// Flips the edge of the image such that A,B,C,D looks like ...C,B,A,B,C,D,C,B...
RGBA getPixelReflected(std::vector<RGBA> &data, int width, int height, int x, int y) {
    int newX = (x < 0) ? -x : std::min(x, (width - 1) - (x - (width - 1)));
    int newY = (y < 0) ? -y : std::min(y, (height - 1) - (y - (height - 1)));
    return data[width * newY + newX];
}

// RGBAf version
RGBAf getPixelReflected(std::vector<RGBAf> &data, int width, int height, int x, int y) {
    int newX = (x < 0) ? -x : std::min(x, (width - 1) - (x - (width - 1)));
    int newY = (y < 0) ? -y : std::min(y, (height - 1) - (y - (height - 1)));
    return data[width * newY + newX];
}

// Wraps the image such that A,B,C,D looks like ...C,D,A,B,C,D,A,B...
RGBA getPixelWrapped(std::vector<RGBA> &data, int width, int height, int x, int y) {
    int newX = (x < 0) ? x + width  : x % width;
    int newY = (y < 0) ? y + height : y % height;
    return data[width * newY + newX];
}

// Assumes the input kernel is square, and has an odd-numbered side length
void convolve2D(std::vector<RGBA> &data, int width, int height, const std::vector<float> &kernel) {
    // Task 9: initialize a vector, called `result`, to temporarily store your output image data
    std::vector<RGBA> result(data.size());
    // Task 10: obtain the kernel's dimensions
    int kernelSideLen = std::sqrt(kernel.size());
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            size_t centerIndex = r * width + c; // index of current pixel in image

            // Task 11:
            // 1. Initialize redAcc, greenAcc, and blueAcc float variables
            float redAcc = 0.0;
            float greenAcc = 0.0;
            float blueAcc = 0.0;
            // 2. Iterate over the kernel using its dimensions from task 10.
            for (int i = kernel.size() - 1; i >= 0; i--) {
                // Get the value, called `weight`, of the kernel at some position.
                float weight = kernel[i];

                // calculating index on original image
                int kernelRow = i / kernelSideLen;
                int kernelCol = i % kernelSideLen;
                int relativeRow = kernelRow - (kernelSideLen / 2);
                int relativeCol = kernelCol - (kernelSideLen / 2);
                int newRow = r + relativeRow;
                int newCol = c + relativeCol;

                // Get the value, called `pixel`, of the corresponding pixel in the canvas.
                RGBAf pixel;
                // bounds check
                if (newRow < 0 || newRow >= height || newCol < 0 || newCol >= width) {
                    pixel = RGBAf::fromRGBA(getPixelReflected(data, width, height, newCol, newRow));
                } else {
                    pixel = RGBAf::fromRGBA(data[newRow * width + newCol]);
                }

                // 3. Accumulate `weight * pixel` for each channel in redAcc, greenAcc, and blueAcc accordingly
                redAcc += weight * (pixel.r / 255.0);
                greenAcc += weight * (pixel.g / 255.0);
                blueAcc += weight * (pixel.b / 255.0);
            }
            // Task 13: Update buffer with the new RGBA pixel value created from
            //          redAcc, greenAcc, and blueAcc
            result[centerIndex] = RGBA({floatToUint8(redAcc),floatToUint8(greenAcc),floatToUint8(blueAcc), 255});
        }
    }

    // Task 14: Copy the RGBA data from `result` (task 9) to `data`
    for (int j = 0; j < result.size(); j++) {
        data[j] = result[j];
    }
}

// Assumes horizontal 1D vector for kernel
void convolveHorizontal(std::vector<RGBAf> &data, int width, int height, const std::vector<float> &kernel) {
    // Initialize a vector, called `result`, to temporarily store your output image data
    std::vector<RGBAf> result(data.size());
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            size_t centerIndex = r * width + c; // index of current pixel in image

            // Initialize redAcc, greenAcc, and blueAcc float variables
            float redAcc = 0.0f;
            float greenAcc = 0.0f;
            float blueAcc = 0.0f;
            // Iterate over the kernel
            for (int i = 0 ; i < kernel.size(); i++) {
                // Get the value, called `weight`, of the kernel at some position.
                float weight = kernel[i];

                // calculating index on original image
                int newRow = r;
                int newCol = c + i - (kernel.size() / 2);

                // Get the value, called `pixel`, of the corresponding pixel in the canvas.
                RGBAf pixel;
                // bounds check
                if (newRow < 0 || newRow >= height || newCol < 0 || newCol >= width) {
                    pixel = getPixelReflected(data, width, height, newCol, newRow);
                } else {
                    pixel = data[newRow * width + newCol];
                }

                // Accumulate `weight * pixel` for each channel in redAcc, greenAcc, and blueAcc accordingly
                redAcc += weight * pixel.r;
                greenAcc += weight * pixel.g;
                blueAcc += weight * pixel.b;
            }
            // Update buffer with the new RGBA pixel value created from
            // redAcc, greenAcc, and blueAcc
            result[centerIndex] = RGBAf({redAcc,greenAcc,blueAcc, 255.0});
        }
    }

    // Copy the RGBA data from `result` to `data`
    for (int j = 0; j < result.size(); j++) {
        data[j] = result[j];
    }
}

// Assumes vertical 1D vector for kernel
void convolveVertical(std::vector<RGBAf> &data, int width, int height, const std::vector<float> &kernel) {
    // Initialize a vector, called `result`, to temporarily store your output image data
    std::vector<RGBAf> result(data.size());
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            size_t centerIndex = r * width + c; // index of current pixel in image

            // Initialize redAcc, greenAcc, and blueAcc float variables
            float redAcc = 0.0f;
            float greenAcc = 0.0f;
            float blueAcc = 0.0f;
            // Iterate over the kernel
            for (int i = 0 ; i < kernel.size(); i++) {
                // Get the value, called `weight`, of the kernel at some position.
                float weight = kernel[i];

                // calculating index on original image
                int newCol = c;
                int newRow = r + i - (kernel.size() / 2);

                // Get the value, called `pixel`, of the corresponding pixel in the canvas.
                RGBAf pixel;
                // bounds check
                if (newRow < 0 || newRow >= height || newCol < 0 || newCol >= width) {
                    pixel = getPixelReflected(data, width, height, newCol, newRow);
                } else {
                    pixel = data[newRow * width + newCol];
                }

                // Accumulate `weight * pixel` for each channel in redAcc, greenAcc, and blueAcc accordingly
                redAcc += weight * pixel.r;
                greenAcc += weight * pixel.g;
                blueAcc += weight * pixel.b;
            }
            // Update buffer with the new RGBA pixel value created from
            // redAcc, greenAcc, and blueAcc
            result[centerIndex] = RGBAf({redAcc,greenAcc,blueAcc, 255.0});
        }
    }

    // Copy the RGBA data from `result` to `data`
    for (int j = 0; j < result.size(); j++) {
        data[j] = result[j];
    }
}

// Clamp under 255
std::uint8_t clamp(float num) {
    if (num >= 255.0f) {
        return 255;
    }
    return (std::uint8_t) num;
}

float triangle(float radius, int x) {
    // if (x < -radius || x > radius) {
    //     return 0.0;
    // } else {
    return (1 - fabs(x) / radius) / radius;
    // }
}

}

