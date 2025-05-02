#include "patchmatch.h"
#include "canvas2d.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <Eigen/Dense>
using namespace Eigen;

//PatchMatch::PatchMatch() {

//}


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


void PatchMatch::patchmatch(const std::vector<RGBA>& imageA, const std::vector<RGBA>& imageB,
                          int width, int height, int patchSize,
                          std::vector<std::pair<int, int>>& nnf) {
    // Initialize random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distribX(0, width - patchSize);
    std::uniform_int_distribution<> distribY(0, height - patchSize);

    // 1. Initialize NNF randomly
    for (int y = 0; y < height - patchSize + 1; ++y) {
        for (int x = 0; x < width - patchSize + 1; ++x) {
            nnf[y * (width - patchSize + 1) + x] = {distribX(gen), distribY(gen)};
        }
    }

    // 2. Iterate and refine
    const int iterations = 5;  // Increased from 1
    for (int iter = 0; iter < iterations; ++iter) {
        bool reverse = (iter % 2 == 1);

        for (int y = reverse ? height-patchSize : 0;
             reverse ? y >= 0 : y < height-patchSize+1;
             reverse ? y-- : y++) {

            for (int x = reverse ? width-patchSize : 0;
                 reverse ? x >= 0 : x < width-patchSize+1;
                 reverse ? x-- : x++) {

                auto& best_match = nnf[y * (width-patchSize+1) + x];
                int best_dist = patchDistance(
                    extractPatch(imageA, x, y, width, patchSize),
                    extractPatch(imageB, best_match.first, best_match.second, width, patchSize)
                    );

                // Propagation - use neighbor's match directly
                if (x > 0 && !reverse) {
                    const auto& left_match = nnf[y * (width-patchSize+1) + (x-1)];
                    if (left_match.first < width-patchSize) {
                        int dist = patchDistance(
                            extractPatch(imageA, x, y, width, patchSize),
                            extractPatch(imageB, left_match.first, left_match.second, width, patchSize)
                            );
                        if (dist < best_dist) {
                            best_match = left_match;
                            best_dist = dist;
                        }
                    }
                }

                if (y > 0 && !reverse) {
                    const auto& top_match = nnf[(y-1) * (width-patchSize+1) + x];
                    if (top_match.second < height-patchSize) {
                        int dist = patchDistance(
                            extractPatch(imageA, x, y, width, patchSize),
                            extractPatch(imageB, top_match.first, top_match.second, width, patchSize)
                            );
                        if (dist < best_dist) {
                            best_match = top_match;
                            best_dist = dist;
                        }
                    }
                }

                // Random search with exponentially decreasing window
                int search_radius = std::max(width, height)/2;
                while (search_radius >= 1) {
                    int min_x = std::max(0, best_match.first - search_radius);
                    int max_x = std::min(width-patchSize, best_match.first + search_radius);
                    int min_y = std::max(0, best_match.second - search_radius);
                    int max_y = std::min(height-patchSize, best_match.second + search_radius);

                    std::uniform_int_distribution<> distribX(min_x, max_x);
                    std::uniform_int_distribution<> distribY(min_y, max_y);

                    int rx = distribX(gen);
                    int ry = distribY(gen);

                    int dist = patchDistance(
                        extractPatch(imageA, x, y, width, patchSize),
                        extractPatch(imageB, rx, ry, width, patchSize)
                        );

                    if (dist < best_dist) {
                        best_match = {rx, ry};
                        best_dist = dist;
                    }

                    search_radius /= 2;
                }
            }
        }
    }
}

