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
#include "noisemaker.h"
#include <QImage>
#include <QString>
#include <iostream>
#include <vector>

void Canvas2D::filterGray() {
    // Load source and target images
    QString sourceImagePath = "/Users/sherry/Downloads/ocean_real.jpg";
    QString targetImagePath = "/Users/sherry/Downloads/ocean_painting.jpg";
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

    // TODO dont just make a new instance every time lol
    NoiseMaker nm;
    // TODO widths and heights might be different between animation frame and texutre image
    // also rename from source/target to animationframe/textureimage pls :)
    std::vector<RGBA> resultImage = nm.generateNoisyImage(sourceImage, width, height, targetImage, width, height);


    // int patchSize = 9;

    // // to store nearest neighbor field - maps from SOURCE patches to TARGET
    // std::vector<std::pair<int, int>> nnf((width - patchSize + 1) * (height - patchSize + 1));

    // std::cout << "Running PatchMatch algorithm..." << std::endl;
    // // This should map SOURCE to TARGET for style transfer
    // patchmatch(sourceImage, targetImage, width, height, patchSize, nnf);
    // std::cout << "PatchMatch completed." << std::endl;

    // std::cout << "Checking NNF values..." << std::endl;
    // for (int i = 0; i < std::min(10, (int)nnf.size()); ++i) {
    //     std::cout << "NNF[" << i << "]: (" << nnf[i].first << ", " << nnf[i].second << ")" << std::endl;
    // }

    // // Reconstruct an image based on the NNF
    // std::vector<RGBA> resultImage;
    // // but use style from targetImage via the NNF
    // reconstructImage(sourceImage, targetImage, width, height, patchSize, nnf, resultImage);

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

void Canvas2D::filterDownsampleTest() {
    QString sourceImagePath = "/Users/vivli/Downloads/test.jpeg";
    QImage sourceQImage(sourceImagePath);

    if (sourceQImage.isNull()) {
        std::cerr << "Failed to load image" << std::endl;
        return;
    }

    int origWidth = sourceQImage.width();
    int origHeight = sourceQImage.height();
    std::vector<RGBA> originalImage(origWidth * origHeight);

    for (int y = 0; y < origHeight; ++y) {
        for (int x = 0; x < origWidth; ++x) {
            QRgb pixel = sourceQImage.pixel(x, y);
            originalImage[y * origWidth + x] = {
                static_cast<uint8_t>(qRed(pixel)),
                static_cast<uint8_t>(qGreen(pixel)),
                static_cast<uint8_t>(qBlue(pixel)),
                static_cast<uint8_t>(qAlpha(pixel))
            };
        }
    }

    std::vector<RGBA> downsampled = originalImage;
    int dsWidth = origWidth;
    int dsHeight = origHeight;
    NoiseMaker testMaker;
    testMaker.downSample(1, 2, downsampled, dsWidth, dsHeight);  // 2x downsampling

    // stretching the downsampled image to fit original canvas size for viewing purposes
    resize(origWidth, origHeight);
    for (int y = 0; y < origHeight; ++y) {
        for (int x = 0; x < origWidth; ++x) {
            int srcX = x / 2;
            int srcY = y / 2;
            int srcIdx = std::min(srcY, dsHeight - 1) * dsWidth + std::min(srcX, dsWidth - 1);
            m_data[y * origWidth + x] = downsampled[srcIdx];
        }
    }

    update();
    std::cout << "Displayed downsampled image (stretched for view)." << std::endl;
}

void Canvas2D::filterUpsampleTest() {
    QString sourceImagePath = "/Users/vivli/Downloads/downsample1.png";
    QImage sourceQImage(sourceImagePath);

    if (sourceQImage.isNull()) {
        std::cerr << "Failed to load image" << std::endl;
        return;
    }

    int smallWidth = sourceQImage.width();
    int smallHeight = sourceQImage.height();
    std::vector<RGBA> smallImage(smallWidth * smallHeight);

    for (int y = 0; y < smallHeight; ++y) {
        for (int x = 0; x < smallWidth; ++x) {
            QRgb pixel = sourceQImage.pixel(x, y);
            smallImage[y * smallWidth + x] = {
                static_cast<uint8_t>(qRed(pixel)),
                static_cast<uint8_t>(qGreen(pixel)),
                static_cast<uint8_t>(qBlue(pixel)),
                static_cast<uint8_t>(qAlpha(pixel))
            };
        }
    }

    int upWidth = smallWidth * 2;
    int upHeight = smallHeight * 2;
    std::vector<RGBA> upsampled;

    NoiseMaker testMaker;
    testMaker.upsample(smallImage, smallWidth, smallHeight, upsampled, upWidth, upHeight);

    resize(upWidth, upHeight);
    for (int y = 0; y < upHeight; ++y) {
        for (int x = 0; x < upWidth; ++x) {
            int idx = y * upWidth + x;
            m_data[idx] = upsampled[idx];
        }
    }

    update();
    std::cout << "Displayed result of upsampling " << smallWidth << "x" << smallHeight
              << " image to " << upWidth << "x" << upHeight << std::endl;
}

void Canvas2D::filterPyramidResampleTest() {
    QString sourceImagePath = "/Users/vivli/Downloads/test.jpeg";
    QImage sourceQImage(sourceImagePath);

    if (sourceQImage.isNull()) {
        std::cerr << "Failed to load image" << std::endl;
        return;
    }

    int origWidth = sourceQImage.width();
    int origHeight = sourceQImage.height();
    std::vector<RGBA> originalImage(origWidth * origHeight);

    for (int y = 0; y < origHeight; ++y) {
        for (int x = 0; x < origWidth; ++x) {
            QRgb pixel = sourceQImage.pixel(x, y);
            originalImage[y * origWidth + x] = {
                static_cast<uint8_t>(qRed(pixel)),
                static_cast<uint8_t>(qGreen(pixel)),
                static_cast<uint8_t>(qBlue(pixel)),
                static_cast<uint8_t>(qAlpha(pixel))
            };
        }
    }

    // downsampling by 2x
    std::vector<RGBA> downsampled = originalImage;
    int dsWidth = origWidth;
    int dsHeight = origHeight;
    NoiseMaker testMaker;
    testMaker.downSample(1, 2, downsampled, dsWidth, dsHeight);

    // upsampling back to original resolution
    std::vector<RGBA> upsampled;
    testMaker.upsample(downsampled, dsWidth, dsHeight, upsampled, origWidth, origHeight);

    // displaying
    resize(origWidth, origHeight);
    for (int y = 0; y < origHeight; ++y) {
        for (int x = 0; x < origWidth; ++x) {
            int idx = y * origWidth + x;
            if (idx < upsampled.size()) {
                m_data[idx] = upsampled[idx];
            }
        }
    }

    update();
    std::cout << "Simulated pyramid resample test (downsample + upsample)." << std::endl;
}

void Canvas2D::filterPyramidProcessTest() {
    QString imagePath = "/Users/vivli/Downloads/test.jpeg";
    QImage sourceQImage(imagePath);

    if (sourceQImage.isNull()) {
        std::cerr << "Failed to load image" << std::endl;
        return;
    }

    int width = sourceQImage.width();
    int height = sourceQImage.height();
    std::vector<RGBA> imageData(width * height);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            QRgb pixel = sourceQImage.pixel(x, y);
            imageData[y * width + x] = {
                static_cast<uint8_t>(qRed(pixel)),
                static_cast<uint8_t>(qGreen(pixel)),
                static_cast<uint8_t>(qBlue(pixel)),
                static_cast<uint8_t>(qAlpha(pixel))
            };
        }
    }

    // Use the same image for both texture and target
    std::vector<RGBA> outputImage;
    NoiseMaker nm;
    nm.processImagePyramids(imageData, width, height, imageData, width, height, outputImage, /*numIterations=*/1, /*pyramidLevels=*/3);

    // Display output
    resize(width, height);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = y * width + x;
            m_data[idx] = outputImage[idx];
        }
    }

    update();
    std::cout << "Displayed result of pyramid downsample + upsample pipeline." << std::endl;
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
