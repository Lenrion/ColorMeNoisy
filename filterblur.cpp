#include "canvas2d.h"
#include "filterutils.h"
#include <iostream>

//void Canvas2D::filterBlur(int blurRadius) {
//    // if blur radius is 0, do nothing
//    if (blurRadius != 0) {
//        // calculate kernel vals
//        int sideLen = 2 * blurRadius + 1;
//        std::vector<float> kernel(sideLen, 0.0);
//        float sum = 0.0;
//        for (int i = 0; i < sideLen; i++) {
//            // get row, col, distance from center of current thing in array
//            float dist = abs(blurRadius - i);

//            // calculate what should be there
//            kernel[i] = 1.0 - (dist / blurRadius);

//            sum += kernel[i];
//        }
//        for (int j = 0; j < kernel.size(); j++) {
//            kernel[j] = kernel[j] / sum; // normalize
//        }
//        // copy image to convolve over
//        std::vector<RGBAf> new_data(m_data.size(), RGBAf{0.0,0.0,0.0,0.0});
//        for (int i = 0; i < m_data.size(); i++) {
//            new_data[i] = RGBAf::fromRGBA(m_data[i]);
//        }
//        FilterUtils::convolveHorizontal(new_data, m_width, m_height, kernel);
//        FilterUtils::convolveVertical(new_data, m_width, m_height, kernel);

//        // apply to m_data
//        for (int i = 0; i < m_data.size(); i++) {
//            m_data[i].r = FilterUtils::clamp(new_data[i].r);
//            m_data[i].g = FilterUtils::clamp(new_data[i].g);
//            m_data[i].b = FilterUtils::clamp(new_data[i].b);

//        }
//    }

//}

void Canvas2D::filterBlur(int filterRadius) {
    int scale = 2;
    // If filter radius is 0, just perform downsampling without blur
    int kernelSize = 2 * filterRadius + 1;

    // Create box filter kernel (all values equal)
    std::vector<float> kernel(kernelSize, 1.0f / kernelSize);

    // Copy image to convolve over
    std::vector<RGBAf> new_data(m_data.size(), RGBAf{0.0,0.0,0.0,0.0});
    for (int i = 0; i < m_data.size(); i++) {
        new_data[i] = RGBAf::fromRGBA(m_data[i]);
    }

    // Apply box filter (if radius > 0)
    if (filterRadius > 0) {
        FilterUtils::convolveHorizontal(new_data, m_width, m_height, kernel);
        FilterUtils::convolveVertical(new_data, m_width, m_height, kernel);
    }

    // Create a new smaller canvas for the downsampled image
    int newWidth = m_width / scale;
    int newHeight = m_height / scale;
    std::vector<RGBA> downsampled_data(newWidth * newHeight);

    // Perform downsampling by taking every nth pixel
    for (int y = 0; y < newHeight; y++) {
        for (int x = 0; x < newWidth; x++) {
            int srcX = x * scale;
            int srcY = y * scale;

            // Clamp source coordinates to prevent out-of-bounds access
            srcX = std::min(srcX, m_width - 1);
            srcY = std::min(srcY, m_height - 1);

            // Get the pixel from the filtered image
            int srcIdx = srcY * m_width + srcX;
            RGBAf filteredPixel = new_data[srcIdx];

            // Convert back to RGBA and store in downsampled image
            RGBA pixel;
            pixel.r = FilterUtils::clamp(filteredPixel.r);
            pixel.g = FilterUtils::clamp(filteredPixel.g);
            pixel.b = FilterUtils::clamp(filteredPixel.b);
            pixel.a = 255; // Assuming alpha should be fully opaque

            downsampled_data[y * newWidth + x] = pixel;
        }
    }

    // Replace the original image with the downsampled one
    m_data = downsampled_data;
    m_width = newWidth;
    m_height = newHeight;
}
