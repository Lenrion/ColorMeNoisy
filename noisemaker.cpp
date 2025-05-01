#include "noisemaker.h"
#include "patchmatch.h"
#include "filterutils.h"
#include "rgba.h"
#include <vector>
#include "iostream"
#include <cmath>

NoiseMaker::NoiseMaker()
{


}

// generates one noisy image given animation frame and texture image
// TODO loop that runs through all frames in animation
void NoiseMaker::generateNoisyImage(std::vector<RGBA>& animationFrame, int frameWidth, int frameHeight,
                                    std::vector<RGBA>& textureImage, int textureWidth, int textureHeight) {
    // "source" refers to animationFrame and "target" refers to textureImage
    // (1) predeform the textureImage
    // TODO currently just use the original as current and previous, which will result in random deformation
    // if we want to consider the previous frame in predeformation, need to pass in previous frame's texture iamge
    std::vector<RGBA> deformedTexture = predeform(textureImage, textureWidth, textureHeight, textureImage, textureWidth, textureHeight, estimateMotion(textureImage, textureWidth, textureHeight, textureImage, textureWidth, textureHeight));

    // (2) create image pyramids with gaussian blur of textureImage
    createImagePyramids(textureImage, textureWidth, textureHeight, animationFrame, frameWidth, frameHeight, PYRAMID_LEVELS);

    // (3) create blurred animationFrame? 2^PYRAMID_LEVELS for now to get equally blurry animation frame
    downSample(1, int(pow(2, PYRAMID_LEVELS)), animationFrame, frameWidth, frameHeight);

    // (4) loop through image pyrmaid array (start with most gaussian blurred)

    // (5) for each, run patchmatch on low pass filtered animationFrame and low pass filtered textureImage

    // (6) upsample the output of patchmatch on the animationFrame and repeat with next thing in texture image pyramid
    //     (as this continues, the details get finer and finer)

    // (7) yippee
}

void NoiseMaker::processImagePyramids(
    const std::vector<RGBA>& textureImage, int textureWidth, int textureHeight,
    const std::vector<RGBA>& originalFrame, int frameWidth, int frameHeight,
    std::vector<RGBA>& outputImage, int numIterations, int pyramidLevels) {

    // creates texturePyramid and texturePyramidDims, downsizes output frame to the smallest level to get ready for main loop of scaling up
    createImagePyramids(textureImage, textureWidth, textureHeight,
                        originalFrame, frameWidth, frameHeight, pyramidLevels);

    std::vector<RGBA> currentResult = currentTargetFrame;
    int currentWidth = currentTargetWidth;
    int currentHeight = currentTargetHeight;

    // coarsest to finest --bottom-up through the pyramid levels
    for (int level = pyramidLevels - 1; level >= 0; level--) {

        // texture at current level
        const std::vector<RGBA>& currLevelTexture = texturePyramid[level];
        int currTexWidth = texturePyramidDims[level].first;
        int currTexHeight = texturePyramidDims[level].second;

        // upsample our current result image to match current texture pyramid level
        //if not at highest level
        if (level != pyramidLevels - 1) {
            std::vector<RGBA> upsampledResult;
            upsample(currentResult, currentWidth, currentHeight, upsampledResult, currTexWidth, currTexHeight);
            currentResult = upsampledResult;
            currentWidth = currTexWidth;
            currentHeight = currTexHeight;
        } else {
            // already at coarsest level
            currentWidth = currTexWidth;
            currentHeight = currTexHeight;
        }

        std::cout << "Running PatchMatch at level " << level << "...\n";

        int patchSize = 5;
        std::vector<std::pair<int, int>> nnf((currentWidth - patchSize + 1) * (currentHeight - patchSize + 1));

        PatchMatch::patchmatch(currentResult, currLevelTexture, currentWidth, currentHeight, patchSize, nnf);
        std::cout << "PatchMatch completed.\n";

        for (int i = 0; i < std::min(10, static_cast<int>(nnf.size())); ++i) {
            std::cout << "NNF[" << i << "]: (" << nnf[i].first << ", " << nnf[i].second << ")" << std::endl;
        }

        std::vector<RGBA> levelResult;

        reconstructImage(currentResult, currLevelTexture, currentWidth, currentHeight, patchSize, nnf, levelResult);

        currentResult = levelResult;
    }
    outputImage = currentResult;
}

std::vector<RGBA> NoiseMaker::predeform(
    const std::vector<RGBA>& currentFrame, int currentWidth, int currentHeight,
    const std::vector<RGBA>& previousResult, int previousWidth, int previousHeight,
    const std::vector<Eigen::Vector2f>& motionVectors) {

    // Deformation result
    std::vector<RGBA> deformedTarget(currentWidth * currentHeight);

    // (black with alpha = 255)
    for (auto& pixel : deformedTarget) {
        pixel = {0, 0, 0, 255};
    }

    // For each pixel in the current frame
    for (int y = 0; y < currentHeight; y++) {
        for (int x = 0; x < currentWidth; x++) {
            int pixelIndex = y * currentWidth + x;

            Eigen::Vector2f motion = Eigen::Vector2f::Zero();
            if (!motionVectors.empty()) {
                motion = motionVectors[pixelIndex];
            }

            float srcX = x + motion(0);
            float srcY = y + motion(1);

            if (srcX >= 0 && srcX < previousWidth && srcY >= 0 && srcY < previousHeight) {
                RGBA color = bilinearInterpolation(srcX, srcY, previousResult, previousWidth, previousHeight);
                deformedTarget[pixelIndex] = color;
            } else {
                deformedTarget[pixelIndex] = currentFrame[pixelIndex];
            }
        }
    }

    return deformedTarget;
}


void NoiseMaker::reconstructImage(
    const std::vector<RGBA>& sourceImage,
    const std::vector<RGBA>& targetImage,
    int width, int height,
    int patchSize,
    const std::vector<std::pair<int, int>>& nnf,
    std::vector<RGBA>& outputImage)
{
    outputImage.resize(width * height);

    // unique ID per color to check frqs
    struct ColorHash {
        size_t operator()(const RGBA& c) const {
            return (c.r << 16) | (c.g << 8) | c.b;
        }
    };
    struct ColorEqual {
        bool operator()(const RGBA& a, const RGBA& b) const {
            return a.r == b.r && a.g == b.g && a.b == b.b;
        }
    };

    std::vector<std::unordered_map<RGBA, int, ColorHash, ColorEqual>> pixelColors(width * height);

    int nnfWidth = width - patchSize + 1;

    // count color frequencies
    for (int y = 0; y < height - patchSize + 1; ++y) {
        for (int x = 0; x < width - patchSize + 1; ++x) {
            int srcPatchIdx = y * nnfWidth + x;
            if (srcPatchIdx >= nnf.size()) continue;

            auto [targetX, targetY] = nnf[srcPatchIdx];

            for (int dy = 0; dy < patchSize; ++dy) {
                for (int dx = 0; dx < patchSize; ++dx) {
                    int srcX = x + dx;
                    int srcY = y + dy;
                    int targetX = nnf[srcPatchIdx].first + dx;
                    int targetY = nnf[srcPatchIdx].second + dy;

                    if (srcX >= 0 && srcX < width && srcY >= 0 && srcY < height &&
                        targetX >= 0 && targetX < width && targetY >= 0 && targetY < height)
                    {
                        int srcIdx = srcY * width + srcX;
                        int targetIdx = targetY * width + targetX;
                        RGBA color = targetImage[targetIdx];
                        pixelColors[srcIdx][color]++;
                    }
                }
            }
        }
    }

    //  mode for each pixel
    for (int i = 0; i < width * height; ++i) {
        if (!pixelColors[i].empty()) {
            // Find color with maximum count
            auto mode = std::max_element(
                pixelColors[i].begin(),
                pixelColors[i].end(),
                [](const auto& a, const auto& b) {
                    return a.second < b.second;
                }
                );
            outputImage[i] = mode->first;
        } else {
            // if no patches contributed
            outputImage[i] = sourceImage[i];
        }
    }
}

std::vector<Eigen::Vector2f> NoiseMaker::estimateMotion(
    const std::vector<RGBA>& currentFrame, int currentWidth, int currentHeight,
    const std::vector<RGBA>& previousFrame, int previousWidth, int previousHeight) {

    const int blockSize = 8; // Size of block for matching
    const int searchRange = 16; // Maximum search distance

    // Initialize motion vectors (default to zero motion)
    std::vector<Eigen::Vector2f> motionVectors(currentWidth * currentHeight, Eigen::Vector2f::Zero());

    for (int blockY = 0; blockY < currentHeight; blockY += blockSize) {
        for (int blockX = 0; blockX < currentWidth; blockX += blockSize) {

            int actualBlockWidth = std::min(blockSize, currentWidth - blockX);
            int actualBlockHeight = std::min(blockSize, currentHeight - blockY);

            float bestMatch = std::numeric_limits<float>::max();
            Eigen::Vector2f bestMotion = Eigen::Vector2f::Zero();

            for (int dy = -searchRange; dy <= searchRange; dy++) {
                for (int dx = -searchRange; dx <= searchRange; dx++) {

                    int refBlockX = blockX + dx;
                    int refBlockY = blockY + dy;

                    if (refBlockX < 0 || refBlockX + actualBlockWidth > previousWidth ||
                        refBlockY < 0 || refBlockY + actualBlockHeight > previousHeight) {
                        continue;
                    }

                    // Compute sum of absolute differences (SAD)
                    float sad = 0.0f;
                    for (int y = 0; y < actualBlockHeight; y++) {
                        for (int x = 0; x < actualBlockWidth; x++) {
                            int currIdx = (blockY + y) * currentWidth + (blockX + x);
                            int refIdx = (refBlockY + y) * previousWidth + (refBlockX + x);

                            const RGBA& currPixel = currentFrame[currIdx];
                            const RGBA& refPixel = previousFrame[refIdx];

                            // Compute pixel difference
                            sad += std::abs(currPixel.r - refPixel.r) +
                                   std::abs(currPixel.g - refPixel.g) +
                                   std::abs(currPixel.b - refPixel.b);
                        }
                    }

                    if (sad < bestMatch) {
                        bestMatch = sad;
                        bestMotion = Eigen::Vector2f(dx, dy);
                    }
                }
            }

            for (int y = 0; y < actualBlockHeight; y++) {
                for (int x = 0; x < actualBlockWidth; x++) {
                    int pixelIndex = (blockY + y) * currentWidth + (blockX + x);
                    motionVectors[pixelIndex] = bestMotion;
                }
            }
        }
    }

    return motionVectors;
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

void NoiseMaker::upsample(const std::vector<RGBA>& coarseLevel, int coarseWidth, int coarseHeight, std::vector<RGBA>& fineLevel, int fineWidth, int fineHeight) {
    fineLevel.resize(fineWidth * fineHeight);

    float scaleX = static_cast<float>(coarseWidth) / fineWidth;
    float scaleY = static_cast<float>(coarseHeight) / fineHeight;

    for (int y = 0; y < fineHeight; y++) {
        for (int x = 0; x < fineWidth; x++) {
            float srcX = x * scaleX;
            float srcY = y * scaleY;

            RGBA value = bilinearInterpolation(srcX, srcY, coarseLevel, coarseWidth, coarseHeight);
            fineLevel[y * fineWidth + x] = value;
        }
    }
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
