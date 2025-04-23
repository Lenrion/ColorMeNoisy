#include "patchmatch.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <cmath>
#include <Eigen/Dense>
#include <RGBA.h>
using namespace Eigen;

PatchMatch::PatchMatch() {

}


// calculate the squared distance between two patches
int patchDistance(const std::vector<RGBA>& patch1, const std::vector<RGBA>& patch2) {
    int distance = 0;
    for (size_t i = 0; i < patch1.size(); ++i) {
        distance += (patch1[i].r - patch2[i].r) * (patch1[i].r - patch2[i].r);
        distance += (patch1[i].g - patch2[i].g) * (patch1[i].g - patch2[i].g);
        distance += (patch1[i].b - patch2[i].b) * (patch1[i].b - patch2[i].b);
    }
    return distance;
}

std::vector<RGBA> extractPatch(const std::vector<RGBA>& image, int x, int y, int width, int patchSize) {
    std::vector<RGBA> patch;
    for (int dy = 0; dy < patchSize; ++dy) {
        patch.insert(patch.end(), image.begin() + (y + dy) * width + x, image.begin() + (y + dy) * width + x + patchSize);
    }
    return patch;
}


void patchmatch(const std::vector<RGBA>& imageA, const std::vector<RGBA>& imageB, int width, int height, int patchSize, std::vector<std::pair<int, int>>& nnf) {
    // Initialize the NNF randomly
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distribX(0, width - patchSize);
    std::uniform_int_distribution<> distribY(0, height - patchSize);

    for (int y = 0; y < height - patchSize + 1; ++y) {
        for (int x = 0; x < width - patchSize + 1; ++x) {
            nnf[y * (width - patchSize + 1) + x] = std::make_pair(distribX(gen), distribY(gen));
        }
    }

    // Iterate and refine the NNF
    int iterations = 5;
    for (int iter = 0; iter < iterations; ++iter) {
        // Alternate the order of propagation
        int yStart = (iter % 2 == 0) ? 0 : height - patchSize;
        int yEnd = (iter % 2 == 0) ? height - patchSize : 0;
        int yStep = (iter % 2 == 0) ? 1 : -1;

        int xStart = (iter % 2 == 0) ? 0 : width - patchSize;
        int xEnd = (iter % 2 == 0) ? width - patchSize : 0;
        int xStep = (iter % 2 == 0) ? 1 : -1;

        for (int y = yStart; y != yEnd; y += yStep) {
            for (int x = xStart; x != xEnd; x += xStep) {
                // Current best match
                int bestMatchX = nnf[y * (width - patchSize + 1) + x].first;
                int bestMatchY = nnf[y * (width - patchSize + 1) + x].second;
                int bestDistance = patchDistance(
                    extractPatch(imageA, x, y, width, patchSize),
                    extractPatch(imageB, bestMatchX, bestMatchY, width, patchSize)
                    // std::vector<Pixel>(imageA.begin() + y * width + x, imageA.begin() + y * width + x + patchSize * width + patchSize),
                    // std::vector<Pixel>(imageB.begin() + bestMatchY * width + bestMatchX, imageB.begin() + bestMatchY * width + bestMatchX + patchSize * width + patchSize)
                    );

                // Propagation
                if (x - xStep >= 0 && x - xStep <= width - patchSize) {
                    int newMatchX = nnf[y * (width - patchSize + 1) + (x - xStep)].first + xStep;
                    int newMatchY = nnf[y * (width - patchSize + 1) + (x - xStep)].second;

                    if (newMatchX >= 0 && newMatchX <= width - patchSize) {
                        int newDistance = patchDistance(
                            std::vector<RGBA>(imageA.begin() + y * width + x, imageA.begin() + y * width + x + patchSize * width + patchSize),
                            std::vector<RGBA>(imageB.begin() + newMatchY * width + newMatchX, imageB.begin() + newMatchY * width + newMatchX + patchSize * width + patchSize)
                            );
                        if (newDistance < bestDistance) {
                            bestDistance = newDistance;
                            bestMatchX = newMatchX;
                            bestMatchY = newMatchY;
                        }
                    }
                }

                if (y - yStep >= 0 && y - yStep <= height - patchSize) {
                    int newMatchX = nnf[(y - yStep) * (width - patchSize + 1) + x].first;
                    int newMatchY = nnf[(y - yStep) * (width - patchSize + 1) + x].second + yStep;

                    if (newMatchY >= 0 && newMatchY <= height - patchSize) {
                        int newDistance = patchDistance(
                            std::vector<RGBA>(imageA.begin() + y * width + x, imageA.begin() + y * width + x + patchSize * width + patchSize),
                            std::vector<RGBA>(imageB.begin() + newMatchY * width + newMatchX, imageB.begin() + newMatchY * width + newMatchX + patchSize * width + patchSize)
                            );
                        if (newDistance < bestDistance) {
                            bestDistance = newDistance;
                            bestMatchX = newMatchX;
                            bestMatchY = newMatchY;
                        }
                    }
                }

                // Random search
                int searchRadius = std::max(width, height);
                while (searchRadius > 1) {
                    std::uniform_int_distribution<> distribSearchX(-searchRadius, searchRadius);
                    std::uniform_int_distribution<> distribSearchY(-searchRadius, searchRadius);

                    int newMatchX = bestMatchX + distribSearchX(gen);
                    int newMatchY = bestMatchY + distribSearchY(gen);

                    if (newMatchX >= 0 && newMatchX <= width - patchSize && newMatchY >= 0 && newMatchY <= height - patchSize) {
                        int newDistance = patchDistance(
                            std::vector<RGBA>(imageA.begin() + y * width + x, imageA.begin() + y * width + x + patchSize * width + patchSize),
                            std::vector<RGBA>(imageB.begin() + newMatchY * width + newMatchX, imageB.begin() + newMatchY * width + newMatchX + patchSize * width + patchSize)
                            );
                        if (newDistance < bestDistance) {
                            bestDistance = newDistance;
                            bestMatchX = newMatchX;
                            bestMatchY = newMatchY;
                        }
                    }
                    searchRadius /= 2;
                }
                nnf[y * (width - patchSize + 1) + x].first = bestMatchX;
                nnf[y * (width - patchSize + 1) + x].second = bestMatchY;
            }
        }
    }
}

// method to reconstruct an image given NNFs of best matches
void reconstructImage(
    const std::vector<RGBA>& imageB,
    int width,
    int height,
    int patchSize,
    const std::vector<std::pair<int, int>>& nnf,
    std::vector<RGBA>& outputImage
    ) {
    outputImage.resize(width * height);

    // Initialize output to black
    for (auto& pixel : outputImage) {
        pixel = {0, 0, 0};
    }

    // To handle overlapping patches, we'll also track the count of how many times a pixel is written
    std::vector<int> pixelCount(width * height, 0);

    int nnfStride = width - patchSize + 1;

    for (int y = 0; y < height - patchSize + 1; ++y) {
        for (int x = 0; x < width - patchSize + 1; ++x) {
            int idx = y * nnfStride + x;
            int matchX = nnf[idx].first;
            int matchY = nnf[idx].second;

            for (int dy = 0; dy < patchSize; ++dy) {
                for (int dx = 0; dx < patchSize; ++dx) {
                    int targetX = x + dx;
                    int targetY = y + dy;
                    int sourceX = matchX + dx;
                    int sourceY = matchY + dy;

                    if (targetX < width && targetY < height &&
                        sourceX < width && sourceY < height) {
                        int outputIdx = targetY * width + targetX;
                        int sourceIdx = sourceY * width + sourceX;

                        // Accumulate pixel values
                        outputImage[outputIdx].r += imageB[sourceIdx].r;
                        outputImage[outputIdx].g += imageB[sourceIdx].g;
                        outputImage[outputIdx].b += imageB[sourceIdx].b;
                        pixelCount[outputIdx]++;
                    }
                }
            }
        }
    }

    // Average overlapping pixels
    for (int i = 0; i < width * height; ++i) {
        if (pixelCount[i] > 0) {
            outputImage[i].r /= pixelCount[i];
            outputImage[i].g /= pixelCount[i];
            outputImage[i].b /= pixelCount[i];
        }
    }
}

