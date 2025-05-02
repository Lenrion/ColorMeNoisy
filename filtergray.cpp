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
#include "QtCore/qdir.h"
#include "QtGui/qpainter.h"
#include "QtWidgets/qapplication.h"
#include "canvas2d.h"
#include "filterutils.h"
#include "patchmatch.h"
#include "noisemaker.h"
#include <QImage>
#include <QString>
#include <iostream>
#include <vector>


void Canvas2D::filterGray() {
    // Load target texture image
    QString texturePath = "/Users/Lexi_Henrion/Downloads/texswatch.png";
    QImage textureQImage(texturePath);

    if (textureQImage.isNull()) {
        std::cerr << "Failed to load texture image" << std::endl;
        return;
    }

    // Input and output directories
    QString inputDir = "/Users/Lexi_Henrion/Downloads/frameszach";
    QString outputDir = "/Users/Lexi_Henrion/Downloads/anitestoutputs";

    // Create output directory if it doesn't exist
    QDir dir;
    if (!dir.exists(outputDir)) {
        if (!dir.mkpath(outputDir)) {
            std::cerr << "Failed to create output directory" << std::endl;
            return;
        }
    }

    // Get list of image files from the input directory
    QDir inputDirObj(inputDir);
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.tif";
    QStringList imageFiles = inputDirObj.entryList(filters, QDir::Files, QDir::Name);

    int imageCount = imageFiles.size();
    std::cout << "Found " << imageCount << " images in " << inputDir.toStdString() << ". Now processing..." << std::endl;

    if (imageCount == 0) {
        std::cerr << "No image files found in the input directory." << std::endl;
        return;
    }

    // Initialize NoiseMaker
    NoiseMaker nm;

    // Process each image
    int imageIndex = 0;

    std::cout << "Starting image processing..." << std::endl;

    foreach (const QString &imageFile, imageFiles) {
        // Load the image
        QString imagePath = inputDir + "/" + imageFile;
        QImage sourceQImage(imagePath);

        if (sourceQImage.isNull()) {
            std::cerr << "Failed to load image from: " << imagePath.toStdString() << std::endl;
            continue;
        }

        int width = sourceQImage.width();
        int height = sourceQImage.height();

        std::cout << "Processing image " << imageIndex+1 << "/" << imageCount
                  << ": " << imageFile.toStdString()
                  << " (size: " << width << "x" << height << ")" << std::endl;

        // Pad texture to match image dimensions
        QImage paddedTextureQImage = padTextureToMatchFrame(textureQImage, width, height);

        // Convert QImages to RGBA vectors
        std::vector<RGBA> sourceImage(width * height);
        std::vector<RGBA> textureImage(width * height);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                QRgb sourcePixel = sourceQImage.pixel(x, y);
                sourceImage[y * width + x] = {
                    static_cast<uint8_t>(qRed(sourcePixel)),
                    static_cast<uint8_t>(qGreen(sourcePixel)),
                    static_cast<uint8_t>(qBlue(sourcePixel)),
                    static_cast<uint8_t>(qAlpha(sourcePixel) == 0 ? 255 : qAlpha(sourcePixel))
                };

                QRgb targetPixel = paddedTextureQImage.pixel(x, y);
                textureImage[y * width + x] = {
                    static_cast<uint8_t>(qRed(targetPixel)),
                    static_cast<uint8_t>(qGreen(targetPixel)),
                    static_cast<uint8_t>(qBlue(targetPixel)),
                    static_cast<uint8_t>(qAlpha(targetPixel) == 0 ? 255 : qAlpha(targetPixel))
                };
            }
        }

        // Apply noise effect to the image
        std::vector<RGBA> resultImage = nm.generateNoisyImage(sourceImage, width, height, textureImage, width, height);

        // Convert result back to QImage
        QImage resultQImage(width, height, QImage::Format_RGBA8888);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int index = y * width + x;
                if (index < resultImage.size()) {
                    resultQImage.setPixel(x, y, qRgba(
                                                    resultImage[index].r,
                                                    resultImage[index].g,
                                                    resultImage[index].b,
                                                    resultImage[index].a
                                                    ));
                }
            }
        }

        // Save the processed image (keep original filename but add prefix)
        QString outputFilename = QString("%1/processed_%2").arg(outputDir).arg(imageFile);
        if (!resultQImage.save(outputFilename)) {
            std::cerr << "Failed to save processed image: " << outputFilename.toStdString() << std::endl;
        }

        // Display the current image on the canvas
        resize(width, height);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int index = y * width + x;
                if (index < resultImage.size()) {
                    m_data[index] = resultImage[index];
                } else {
                    m_data[index] = {255, 0, 0, 255}; // Red to indicate error
                }
            }
        }
        update();
        QApplication::processEvents(); // Allow UI to update

        imageIndex++;
    }

    std::cout << "Image processing completed. " << imageIndex << " images processed and saved to "
              << outputDir.toStdString() << std::endl;
}

//void Canvas2D::filterGray() {
//    // Load source and target images
//    QString animationFramePath = "/Users/Lexi_Henrion/Downloads/zach.png";
//    QString texturePath = "/Users/Lexi_Henrion/Downloads/texswatch.png";
//    QImage animQImage(animationFramePath);
//    QImage textureQImage(texturePath);

//    if (animQImage.isNull() || textureQImage.isNull()) {
//        std::cerr << "Failed to load one or both images" << std::endl;
//        return;
//    }

//    std::cout << "Original source dimensions: " << animQImage.width() << "x" << animQImage.height() << std::endl;
//    std::cout << "Original target dimensions: " << textureQImage.width() << "x" << textureQImage.height() << std::endl;


//    int width = animQImage.width();
//    int height = animQImage.height();

//    QImage paddedTextureQImage = padTextureToMatchFrame(textureQImage, width, height);
//    std::cout << "Padded texture dimensions: " << paddedTextureQImage.width() << "x" << paddedTextureQImage.height() << std::endl;


//    // Convert QImages to RGBA vectors
//    std::vector<RGBA> animImage(width * height);
//    std::vector<RGBA> textureImage(width * height);

//    for (int y = 0; y < height; ++y) {
//        for (int x = 0; x < width; ++x) {
//            QRgb sourcePixel = animQImage.pixel(x, y);
//            animImage[y * width + x] = {
//                static_cast<uint8_t>(qRed(sourcePixel)),
//                static_cast<uint8_t>(qGreen(sourcePixel)),
//                static_cast<uint8_t>(qBlue(sourcePixel)),
//                static_cast<uint8_t>(qAlpha(sourcePixel))
//            };

//            QRgb targetPixel = paddedTextureQImage.pixel(x, y);
//            textureImage[y * width + x] = {
//                static_cast<uint8_t>(qRed(targetPixel)),
//                static_cast<uint8_t>(qGreen(targetPixel)),
//                static_cast<uint8_t>(qBlue(targetPixel)),
//                static_cast<uint8_t>(qAlpha(targetPixel))
//            };
//        }
//    }

//    std::cout << "Sample source pixel (0,0): R=" << (int)animImage[0].r
//              << " G=" << (int)animImage[0].g
//              << " B=" << (int)animImage[0].b << std::endl;
//    std::cout << "Sample target pixel (0,0): R=" << (int)textureImage[0].r
//              << " G=" << (int)textureImage[0].g
//              << " B=" << (int)textureImage[0].b << std::endl;

//    // TODO dont just make a new instance every time lol
//    NoiseMaker nm;
//    // TODO widths and heights might be different between animation frame and texture image
//    std::vector<RGBA> resultImage = nm.generateNoisyImage(animImage, width, height, textureImage, width, height);


//    // int patchSize = 9;

//    // // to store nearest neighbor field - maps from SOURCE patches to TARGET
//    // std::vector<std::pair<int, int>> nnf((width - patchSize + 1) * (height - patchSize + 1));

//    // std::cout << "Running PatchMatch algorithm..." << std::endl;
//    // // This should map SOURCE to TARGET for style transfer
//    // patchmatch(sourceImage, targetImage, width, height, patchSize, nnf);
//    // std::cout << "PatchMatch completed." << std::endl;

//    // std::cout << "Checking NNF values..." << std::endl;
//    // for (int i = 0; i < std::min(10, (int)nnf.size()); ++i) {
//    //     std::cout << "NNF[" << i << "]: (" << nnf[i].first << ", " << nnf[i].second << ")" << std::endl;
//    // }

//    // // Reconstruct an image based on the NNF
//    // std::vector<RGBA> resultImage;
//    // // but use style from targetImage via the NNF
//    // reconstructImage(sourceImage, targetImage, width, height, patchSize, nnf, resultImage);

//    // Update canvas
//    resize(width, height);
//    for (int y = 0; y < height; ++y) {
//        for (int x = 0; x < width; ++x) {
//            int index = y * width + x;
//            if (index < resultImage.size()) {
//                m_data[index] = resultImage[index];
//            } else {
//                // Safeguard against out of bounds
//                m_data[index] = {255, 0, 0, 255}; // Red to indicate error
//            }
//        }
//    }

//    update();
//    std::cout << "NoiseMaker test completed. Result displayed on canvas." << std::endl;
//}

QImage Canvas2D::padTextureToMatchFrame(const QImage& texture, int frameWidth, int frameHeight) {
    QImage paddedTexture(frameWidth, frameHeight, QImage::Format_ARGB32);
    paddedTexture.fill(Qt::transparent); // Fill with transparent pixels (zeros)

    int xOffset = (frameWidth - texture.width()) / 2;
    int yOffset = (frameHeight - texture.height()) / 2;

    xOffset = std::max(0, xOffset);
    yOffset = std::max(0, yOffset);

    // to center of the padded image
    QPainter painter(&paddedTexture);
    painter.drawImage(xOffset, yOffset, texture);
    painter.end();

    return paddedTexture;
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
    nm.processImagePyramids(imageData, width, height, imageData, width, height, outputImage); // /*numIterations=*/1, /*pyramidLevels=*/3);

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
