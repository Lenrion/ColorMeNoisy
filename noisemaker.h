#ifndef NOISEMAKER_H
#define NOISEMAKER_H


#include "rgba.h"
#include <vector>
#include <Eigen/Dense>

class NoiseMaker
{
public:
    NoiseMaker();
    void downSample(int filterRadius, int scale, std::vector<RGBA>& img_data, int& img_width, int& img_height);
    void upsample(const std::vector<RGBA>& coarseLevel, int coarseWidth, int coarseHeight, std::vector<RGBA>& fineLevel, int fineWidth, int fineHeight);
    RGBA bilinearInterpolation(float x, float y, const std::vector<RGBA>& img_data, int& img_width, int& img_height);
    std::vector<std::vector<RGBA>> texturePyramid;
    std::vector<std::pair<int, int>> texturePyramidDims;

    // starts at coarsest level, will be interatively upsampled
    std::vector<RGBA> currentTargetFrame;
    int currentTargetWidth;
    int currentTargetHeight;

    std::vector<RGBA> predeform(
        const std::vector<RGBA>& currentFrame, int currentWidth, int currentHeight,
        const std::vector<RGBA>& previousResult, int previousWidth, int previousHeight,
        const std::vector<Eigen::Vector2f>& motionVectors);

    std::vector<Eigen::Vector2f> estimateMotion(
        const std::vector<RGBA>& currentFrame, int currentWidth, int currentHeight,
        const std::vector<RGBA>& previousFrame, int previousWidth, int previousHeight);

    void createImagePyramids(
        const std::vector<RGBA>& textureImage, int textureWidth, int textureHeight,
        const std::vector<RGBA>& animationFrame, int frameWidth, int frameHeight,
        int pyramidLevels);

    void processImagePyramids(
        const std::vector<RGBA>& textureImage, int textureWidth, int textureHeight,
        const std::vector<RGBA>& originalFrame, int frameWidth, int frameHeight,
        std::vector<RGBA>& outputImage, int numIterations, int pyramidLevels);

    void reconstructImage(
        const std::vector<RGBA>& sourceImage,
        const std::vector<RGBA>& targetImage,
        int width, int height,
        int patchSize,
        const std::vector<std::pair<int, int>>& nnf,
        std::vector<RGBA>& outputImage);
};

#endif // NOISEMAKER_H
