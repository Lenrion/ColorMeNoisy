#include "noisemaker.h"
#include "filterutils.h"
#include "rgba.h"
#include <vector>

NoiseMaker::NoiseMaker()
{


}

RGBA NoiseMaker::bilinearInterpolation(float x, float y, const std::vector<RGBA>& img_data, int& img_width, int& img_height) {
    RGBA output = {0, 0, 0, 255};

    int xFloor = static_cast<int>(std::floor(x));
    int yFloor = static_cast<int>(std::floor(y));
    int xCeil  = std::min(xFloor + 1, img_width - 1);
    int yCeil  = std::min(yFloor + 1, img_height - 1);

    // checking image bounds
    if (xFloor < 0 || xFloor >= img_width || yFloor < 0 || yFloor >= img_height) {
        return output;
    }

    const RGBA &topLeft     = img_data[yFloor * img_width + xFloor];
    const RGBA &topRight    = img_data[yFloor * img_width + xCeil];
    const RGBA &bottomLeft  = img_data[yCeil  * img_width + xFloor];
    const RGBA &bottomRight = img_data[yCeil  * img_width + xCeil];

    float xFraction = x - xFloor;
    float yFraction = y - yFloor;

    float oneMinusX = 1.0f - xFraction;
    float oneMinusY = 1.0f - yFraction;

    // interpolating the color channels
    float red =
        topLeft.r * oneMinusX * oneMinusY +
        topRight.r * xFraction * oneMinusY +
        bottomLeft.r * oneMinusX * yFraction +
        bottomRight.r * xFraction * yFraction;

    float green =
        topLeft.g * oneMinusX * oneMinusY +
        topRight.g * xFraction * oneMinusY +
        bottomLeft.g * oneMinusX * yFraction +
        bottomRight.g * xFraction * yFraction;

    float blue =
        topLeft.b * oneMinusX * oneMinusY +
        topRight.b * xFraction * oneMinusY +
        bottomLeft.b * oneMinusX * yFraction +
        bottomRight.b * xFraction * yFraction;

    float alpha =
        topLeft.a * oneMinusX * oneMinusY +
        topRight.a * xFraction * oneMinusY +
        bottomLeft.a * oneMinusX * yFraction +
        bottomRight.a * xFraction * yFraction;

    output.r = static_cast<uint8_t>(std::clamp(red,   0.0f, 255.0f));
    output.g = static_cast<uint8_t>(std::clamp(green, 0.0f, 255.0f));
    output.b = static_cast<uint8_t>(std::clamp(blue,  0.0f, 255.0f));
    output.a = static_cast<uint8_t>(std::clamp(alpha, 0.0f, 255.0f));

    return output;
}

void NoiseMaker::downSample(int filterRadius, int scale, std::vector<RGBA>& img_data, int& img_width, int& img_height) {
    scale = 2; // hardcoded to downsample to half size rn
    int kernelSize = 2 * filterRadius + 1;

    std::vector<float> kernel(kernelSize, 1.0f / kernelSize);

    std::vector<RGBAf> new_data(img_data.size(), RGBAf{0.0,0.0,0.0,0.0});
    for (int i = 0; i < img_data.size(); i++) {
        new_data[i] = RGBAf::fromRGBA(img_data[i]);
    }

    if (filterRadius > 0) {
        FilterUtils::convolveHorizontal(new_data, img_width, img_height, kernel);
        FilterUtils::convolveVertical(new_data, img_width, img_height, kernel);
    }

    int newWidth = img_width / scale;
    int newHeight = img_height / scale;
    std::vector<RGBA> downsampled_data(newWidth * newHeight);

    for (int y = 0; y < newHeight; y++) {
        for (int x = 0; x < newWidth; x++) {
            int srcX = x * scale;
            int srcY = y * scale;

            srcX = std::min(srcX, img_width - 1);
            srcY = std::min(srcY, img_height - 1);

            int srcIdx = srcY * img_width + srcX;
            RGBAf filteredPixel = new_data[srcIdx];

            RGBA pixel;
            pixel.r = FilterUtils::clamp(filteredPixel.r);
            pixel.g = FilterUtils::clamp(filteredPixel.g);
            pixel.b = FilterUtils::clamp(filteredPixel.b);
            pixel.a = 255;

            downsampled_data[y * newWidth + x] = pixel;
        }
    }

    img_data = downsampled_data;
    img_width = newWidth;
    img_height = newHeight;
}

void NoiseMaker::createImagePyramids(
    const std::vector<RGBA>& textureImage, int textureWidth, int textureHeight,
    const std::vector<RGBA>& animationFrame, int frameWidth, int frameHeight,
    int pyramidLevels) {

    texturePyramid.clear();

    texturePyramid.resize(pyramidLevels);
    texturePyramidDims.resize(pyramidLevels);

    texturePyramid[0] = textureImage;
    texturePyramidDims[0] = std::make_pair(textureWidth, textureHeight);

    for (int level = 1; level < pyramidLevels; level++) {
        std::vector<RGBA> downsampledTexture = texturePyramid[level - 1];
        int texWidth = texturePyramidDims[level - 1].first;
        int texHeight = texturePyramidDims[level - 1].second;

        // uses 2x scale factor as hardcoded
        downSample(1, 2, downsampledTexture, texWidth, texHeight);

        texturePyramid[level] = downsampledTexture;
        texturePyramidDims[level] = std::make_pair(texWidth, texHeight);
    }

    // for the animation frame, we only need to downsample it to the coarsest level
    // as our starting point for main loop
    currentTargetFrame = animationFrame;
    currentTargetWidth = frameWidth;
    currentTargetHeight = frameHeight;

    // reach the coarsest level
    for (int level = 1; level < pyramidLevels; level++) {
        downSample(1, 2, currentTargetFrame, currentTargetWidth, currentTargetHeight);
    }

}
