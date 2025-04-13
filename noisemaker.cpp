#include "noisemaker.h"
#include "rgba.h"
#include <vector>

NoiseMaker::NoiseMaker()
{


}



void NoiseMaker::downSample(int filterRadius) {
    int scale = 2;
    int kernelSize = 2 * filterRadius + 1;

    std::vector<float> kernel(kernelSize, 1.0f / kernelSize);

    std::vector<RGBAf> new_data(m_data.size(), RGBAf{0.0,0.0,0.0,0.0});
    for (int i = 0; i < m_data.size(); i++) {
        new_data[i] = RGBAf::fromRGBA(m_data[i]);
    }

    if (filterRadius > 0) {
        FilterUtils::convolveHorizontal(new_data, m_width, m_height, kernel);
        FilterUtils::convolveVertical(new_data, m_width, m_height, kernel);
    }

    int newWidth = m_width / scale;
    int newHeight = m_height / scale;
    std::vector<RGBA> downsampled_data(newWidth * newHeight);

    for (int y = 0; y < newHeight; y++) {
        for (int x = 0; x < newWidth; x++) {
            int srcX = x * scale;
            int srcY = y * scale;

            srcX = std::min(srcX, m_width - 1);
            srcY = std::min(srcY, m_height - 1);

            int srcIdx = srcY * m_width + srcX;
            RGBAf filteredPixel = new_data[srcIdx];

            RGBA pixel;
            pixel.r = FilterUtils::clamp(filteredPixel.r);
            pixel.g = FilterUtils::clamp(filteredPixel.g);
            pixel.b = FilterUtils::clamp(filteredPixel.b);
            pixel.a = 255;

            downsampled_data[y * newWidth + x] = pixel;
        }
    }

    m_data = downsampled_data;
    m_width = newWidth;
    m_height = newHeight;
}


