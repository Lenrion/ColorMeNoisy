#ifndef NOISEMAKER_H
#define NOISEMAKER_H


#include "rgba.h"
#include <vector>
class NoiseMaker
{
public:
    NoiseMaker();
    void downSample(int filterRadius, int scale, std::vector<RGBA>& img_data, int& img_width, int& img_height);
    RGBA bilinearInterpolation(float x, float y, const std::vector<RGBA> &input, int& width, int& height);
    std::vector<std::vector<RGBA>> texturePyramid;
    std::vector<std::pair<int, int>> texturePyramidDims;

    // starts at coarsest level, will be interatively upsampled
    std::vector<RGBA> currentTargetFrame;
    int currentTargetWidth;
    int currentTargetHeight;

    void createImagePyramids(
        const std::vector<RGBA>& textureImage, int textureWidth, int textureHeight,
        const std::vector<RGBA>& animationFrame, int frameWidth, int frameHeight,
        int pyramidLevels);

    void colorMeNoisy(
        const std::vector<RGBA>& textureImage, int textureWidth, int textureHeight,
        const std::vector<RGBA>& originalFrame, int frameWidth, int frameHeight,
        std::vector<RGBA>& outputImage, int numIterations, int pyramidLevels);
};

#endif // NOISEMAKER_H
