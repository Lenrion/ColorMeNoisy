#include "canvas2d.h"
// #include "filterutils.h"
#include <iostream>
// #include "noisemaker.h"
#include "patchmatch.h"

void Canvas2D::filterRotate(float angle) {
    // Load source and target images
    QString animationFramePath = "/Users/sherry/cs2240/color-me-noisy/ColorMeNoisy/fun_images/framesruka/ruka.0001.png";
    QString texturePath = "/Users/sherry/cs2240/color-me-noisy/ColorMeNoisy/fun_images/ruka_tex.png";
    QImage animQImage(animationFramePath);
    QImage textureQImage(texturePath);

    if (animQImage.isNull() || textureQImage.isNull()) {
        std::cerr << "Failed to load one or both images" << std::endl;
        return;
    }

    std::cout << "Original source dimensions: " << animQImage.width() << "x" << animQImage.height() << std::endl;
    std::cout << "Original target dimensions: " << textureQImage.width() << "x" << textureQImage.height() << std::endl;


    int width = animQImage.width();
    int height = animQImage.height();

    QImage paddedTextureQImage = padTextureToMatchFrame(textureQImage, width, height);
    std::cout << "Padded texture dimensions: " << paddedTextureQImage.width() << "x" << paddedTextureQImage.height() << std::endl;


    // Convert QImages to RGBA vectors
    std::vector<RGBA> animImage(width * height);
    std::vector<RGBA> textureImage(width * height);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            QRgb sourcePixel = animQImage.pixel(x, y);
            animImage[y * width + x] = {
                static_cast<uint8_t>(qRed(sourcePixel)),
                static_cast<uint8_t>(qGreen(sourcePixel)),
                static_cast<uint8_t>(qBlue(sourcePixel)),
                static_cast<uint8_t>(qAlpha(sourcePixel))
            };

            QRgb targetPixel = paddedTextureQImage.pixel(x, y);
            textureImage[y * width + x] = {
                static_cast<uint8_t>(qRed(targetPixel)),
                static_cast<uint8_t>(qGreen(targetPixel)),
                static_cast<uint8_t>(qBlue(targetPixel)),
                static_cast<uint8_t>(qAlpha(targetPixel))
            };
        }
    }

    std::cout << "Sample source pixel (0,0): R=" << (int)animImage[0].r
              << " G=" << (int)animImage[0].g
              << " B=" << (int)animImage[0].b << std::endl;
    std::cout << "Sample target pixel (0,0): R=" << (int)textureImage[0].r
              << " G=" << (int)textureImage[0].g
              << " B=" << (int)textureImage[0].b << std::endl;

    // TODO widths and heights might be different between animation frame and texture image
    // std::vector<RGBA> resultImage = nm.generateNoisyImage(animImage, width, height, textureImage, width, height);


    int patchSize = 3;

    // to store nearest neighbor field - maps from SOURCE patches to TARGET
    std::vector<std::pair<int, int>> nnf((width - patchSize + 1) * (height - patchSize + 1));

    std::cout << "Running PatchMatch algorithm..." << std::endl;
    // This should map SOURCE to TARGET for style transfer
    PatchMatch::patchmatch(animImage, textureImage, width, height, patchSize, nnf);
    std::cout << "PatchMatch completed." << std::endl;

    // std::cout << "Checking NNF values..." << std::endl;
    // for (int i = 0; i < std::min(10, (int)nnf.size()); ++i) {
    //     std::cout << "NNF[" << i << "]: (" << nnf[i].first << ", " << nnf[i].second << ")" << std::endl;
    // }

    // Reconstruct an image based on the NNF
    std::vector<RGBA> resultImage;
    // but use style from targetImage via the NNF
    reconstructImage(animImage, textureImage, width, height, patchSize, nnf, resultImage);

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
