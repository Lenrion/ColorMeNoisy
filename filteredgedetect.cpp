#include "canvas2d.h"
#include "filterutils.h"
#include "iostream"

void Canvas2D::filterEdgeDetect(float sensitivity) {
    filterGray();

//    // sobel filters (separated)
//    std::vector<float> kernelXVertical = {
//        1.0f, 2.0f, 1.0f
//    };
//    std::vector<float> kernelXHorizontal = {
//        -1.0f, 0.0f, 1.0f
//    };

//    std::vector<float> kernelYVertical = {
//        -1.0f, 0.0f, 1.0f
//    };
//    std::vector<float> kernelYHorizontal = {
//        1.0f, 2.0f, 1.0f
//    };

//    Canvas2D::filterGray(); // grayscale the image
//    // copy m_data to new vectors gx and gy
//    std::vector<RGBAf> gx(m_data.size(), RGBAf{0.0f,0.0f,0.0f,0.0f});
//    std::vector<RGBAf> gy(m_data.size(), RGBAf{0.0f,0.0f,0.0f,0.0f});
//    for (int i = 0; i < m_data.size(); i++) {
//        gx[i] = RGBAf::fromRGBA(m_data[i]);
//        gy[i] = RGBAf::fromRGBA(m_data[i]);
//    }

//    // convolve to get gx
//    FilterUtils::convolveVertical(gx, m_width, m_height, kernelXVertical);
//    auto gx_copy = gx;
//    FilterUtils::convolveHorizontal(gx_copy, m_width, m_height, kernelXHorizontal);

//    // convolve to get gy
//    FilterUtils::convolveVertical(gy, m_width, m_height, kernelYHorizontal);
//    FilterUtils::convolveHorizontal(gy, m_width, m_height, kernelYVertical);

//    // apply to m_data
//    for (int i = 0; i < m_data.size(); i++) {
//        // get magnitude
//        m_data[i].r = FilterUtils::clamp(sqrt(pow(gx_copy[i].r,2) + pow(gy[i].r, 2)) * pow(sensitivity, 2)); // squared sensitivity
//        m_data[i].g = FilterUtils::clamp(sqrt(pow(gx_copy[i].g,2) + pow(gy[i].g, 2)) * pow(sensitivity, 2));
//        m_data[i].b = FilterUtils::clamp(sqrt(pow(gx_copy[i].b,2) + pow(gy[i].b, 2)) * pow(sensitivity, 2));
//    }
}
