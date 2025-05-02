//#include <cstdint>
//#include "canvas2d.h"
//#include "filterutils.h"

//// Grayscale filter taken from Lab 3.
//std::uint8_t rgbaToGray(const RGBA &pixel) {
//    return 0.299 * pixel.r + 0.587 * pixel.g + 0.114 * pixel.b;
//}

//void Canvas2D::filterGray() {
//    for (int row = 0; row < m_height; ++row) {
//        for (int col = 0; col < m_width; ++col) {
//            size_t currentIndex = m_width * row + col;
//            RGBA &currentPixel = m_data[currentIndex];

//            // call rgbaToGray()
//            uint8_t intensity = rgbaToGray(currentPixel);

//            // update currentPixel's color
//            currentPixel.r = intensity;
//            currentPixel.g = intensity;
//            currentPixel.b = intensity;
//        }
//    }
//}

#include <cstdint>
#include "canvas2d.h"
#include "filterutils.h"
#include "patchmatch.h"
#include <QImage>
#include <QString>
#include <iostream>
#include <vector>

void Canvas2D::filterGray() {
    // Load source and target images
    QString sourceImagePath = "/Users/Lexi_Henrion/Downloads/sunset.jpg";
    QString targetImagePath = "/Users/Lexi_Henrion/Downloads/swatch1.jpg";
    QImage sourceQImage(sourceImagePath);
    QImage targetQImage(targetImagePath);

    if (sourceQImage.isNull() || targetQImage.isNull()) {
        std::cerr << "Failed to load one or both images" << std::endl;
        return;
    }

    std::cout << "Original source dimensions: " << sourceQImage.width() << "x" << sourceQImage.height() << std::endl;
    std::cout << "Original target dimensions: " << targetQImage.width() << "x" << targetQImage.height() << std::endl;


    int width = sourceQImage.width();
    int height = sourceQImage.height();

    // Convert QImages to RGBA vectors
    std::vector<RGBA> sourceImage(width * height);
    std::vector<RGBA> targetImage(targetQImage.width() * targetQImage.height());

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            QRgb sourcePixel = sourceQImage.pixel(x, y);
            sourceImage[y * width + x] = {
                static_cast<uint8_t>(qRed(sourcePixel)),
                static_cast<uint8_t>(qGreen(sourcePixel)),
                static_cast<uint8_t>(qBlue(sourcePixel)),
                static_cast<uint8_t>(qAlpha(sourcePixel))
            };
        }
    }

    for (int y = 0; y < targetQImage.height(); ++y) {
        for (int x = 0; x < targetQImage.width(); ++x) {
            QRgb targetPixel = targetQImage.pixel(x, y);
            targetImage[y * targetQImage.width() + x] = {
                static_cast<uint8_t>(qRed(targetPixel)),
                static_cast<uint8_t>(qGreen(targetPixel)),
                static_cast<uint8_t>(qBlue(targetPixel)),
                static_cast<uint8_t>(qAlpha(targetPixel))
            };
        }
    }


    std::cout << "Sample source pixel (0,0): R=" << (int)sourceImage[0].r
              << " G=" << (int)sourceImage[0].g
              << " B=" << (int)sourceImage[0].b << std::endl;
    std::cout << "Sample target pixel (0,0): R=" << (int)targetImage[0].r
              << " G=" << (int)targetImage[0].g
              << " B=" << (int)targetImage[0].b << std::endl;

    int patchSize = 5;

    // to store nearest neighbor field - maps from SOURCE patches to TARGET
    std::vector<std::pair<int, int>> nnf((width - patchSize + 1) * (height - patchSize + 1));

    std::cout << "Running PatchMatch algorithm..." << std::endl;
    // This should map SOURCE to TARGET for style transfer
    patchmatch(sourceImage, targetImage, width, height, patchSize, nnf);
    std::cout << "PatchMatch completed." << std::endl;

    std::cout << "Checking NNF values..." << std::endl;
    for (int i = 0; i < std::min(10, (int)nnf.size()); ++i) {
        std::cout << "NNF[" << i << "]: (" << nnf[i].first << ", " << nnf[i].second << ")" << std::endl;
    }

    // Reconstruct an image based on the NNF
    std::vector<RGBA> resultImage;
    // but use style from targetImage via the NNF
    reconstructImage(sourceImage, targetImage, width, height, patchSize, nnf, resultImage);

    // Update canvas
    resize(width, height);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = y * width + x;
            if (index < resultImage.size()) {
                m_data[index] = resultImage[index];
            } else {
                // Safeguard against out of bounds
                m_data[index] = {255, 0, 0, 255}; // Red to indicate error
            }
        }
    }

    update();
    std::cout << "PatchMatch test completed. Result displayed on canvas." << std::endl;
}


void Canvas2D::reconstructImage(
    const std::vector<RGBA>& sourceImage,
    const std::vector<RGBA>& targetImage,
    int width, int height,
    int patchSize,
    const std::vector<std::pair<int, int>>& nnf,
    std::vector<RGBA>& outputImage)
{
    outputImage.resize(width * height);

    // Define hash function and equality comparator for RGBA colors
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

    // For each pixel in source image, store frequency of colors from overlapping patches
    std::vector<std::unordered_map<RGBA, int, ColorHash, ColorEqual>> pixelColors(width * height);

    int nnfWidth = width - patchSize + 1;
    int nnfHeight = height - patchSize + 1;

    // Process each source patch and its corresponding target patch
    for (int sy = 0; sy < nnfHeight; ++sy) {
        for (int sx = 0; sx < nnfWidth; ++sx) {
            int srcPatchIdx = sy * nnfWidth + sx;

            // Skip if NNF entry is out of bounds
            if (srcPatchIdx >= nnf.size()) continue;

            // Get the corresponding target patch coordinates
            int tx = nnf[srcPatchIdx].first;
            int ty = nnf[srcPatchIdx].second;

            // Process each pixel in the patch
            for (int dy = 0; dy < patchSize; ++dy) {
                for (int dx = 0; dx < patchSize; ++dx) {
                    // Source pixel coordinates
                    int sourceX = sx + dx;
                    int sourceY = sy + dy;

                    // Target pixel coordinates (from the matched patch)
                    int targetX = tx + dx;
                    int targetY = ty + dy;

                    // Ensure both source and target pixels are within bounds
                    if (sourceX >= 0 && sourceX < width && sourceY >= 0 && sourceY < height &&
                        targetX >= 0 && targetX < width && targetY >= 0 && targetY < height)
                    {
                        int sourceIdx = sourceY * width + sourceX;
                        int targetIdx = targetY * width + targetX;

                        // Add color from target patch to the frequency map of this source pixel
                        RGBA color = targetImage[targetIdx];
                        pixelColors[sourceIdx][color]++;
                    }
                }
            }
        }
    }

    // For each pixel in the output image, select the most frequent color
    for (int i = 0; i < width * height; ++i) {
        if (!pixelColors[i].empty()) {
            // Find color with maximum count (the mode)
            auto mode = std::max_element(
                pixelColors[i].begin(),
                pixelColors[i].end(),
                [](const auto& a, const auto& b) {
                    return a.second < b.second;
                }
                );
            outputImage[i] = mode->first;
        } else {
            // If no patches contributed to this pixel, use the original source color
            outputImage[i] = sourceImage[i];
        }
    }
}
