#include <cstdint>
#include "canvas2d.h"
#include "filterutils.h"

// Grayscale filter taken from Lab 3.
std::uint8_t rgbaToGray(const RGBA &pixel) {
    return 0.299 * pixel.r + 0.587 * pixel.g + 0.114 * pixel.b;
}

void Canvas2D::filterGray() {
    for (int row = 0; row < m_height; ++row) {
        for (int col = 0; col < m_width; ++col) {
            size_t currentIndex = m_width * row + col;
            RGBA &currentPixel = m_data[currentIndex];

            // call rgbaToGray()
            uint8_t intensity = rgbaToGray(currentPixel);

            // update currentPixel's color
            currentPixel.r = intensity;
            currentPixel.g = intensity;
            currentPixel.b = intensity;
        }
    }
}
