#include "patchmatch.h"
#include "canvas2d.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <eigen/Dense>
using namespace Eigen;

//PatchMatch::PatchMatch() {

//}


// calculate the squared distance between two patches
int patchDistance(const std::vector<RGBA>& imageA, const std::vector<RGBA>& imageB,
                  int ax, int ay, int bx, int by, int width, int patchSize)
{
    int dist = 0;
    for (int dy = 0; dy < patchSize; ++dy) {
        for (int dx = 0; dx < patchSize; ++dx) {
            int idxA = (ay + dy) * width + (ax + dx);
            int idxB = (by + dy) * width + (bx + dx);
            const auto& a = imageA[idxA];
            const auto& b = imageB[idxB];
            dist += (a.r - b.r) * (a.r - b.r);
            dist += (a.g - b.g) * (a.g - b.g);
            dist += (a.b - b.b) * (a.b - b.b);
        }
    }
    return dist;
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
    const int wLimit = width - patchSize + 1;
    const int hLimit = height - patchSize + 1;
    nnf.resize(wLimit * hLimit);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distribX(0, wLimit - 1);
    std::uniform_int_distribution<> distribY(0, hLimit - 1);

    // Initialize NNF randomly with clamping
    for (int y = 0; y < hLimit; ++y) {
        for (int x = 0; x < wLimit; ++x) {
            int tx = distribX(gen);
            int ty = distribY(gen);
            tx = std::clamp(tx, 0, wLimit - 1);
            ty = std::clamp(ty, 0, hLimit - 1);
            nnf[y * wLimit + x] = {tx, ty};
        }
    }

    // Iterate and refine
    const int iterations = 3;
    for (int iter = 0; iter < iterations; ++iter) {
        bool reverse = (iter % 2 == 1);

        int yStart = reverse ? hLimit - 1 : 0;
        int yEnd = reverse ? -1 : hLimit;
        int yStep = reverse ? -1 : 1;

#pragma omp parallel for
        for (int y = yStart; y != yEnd; y += yStep) {
            int xStart = reverse ? wLimit - 1 : 0;
            int xEnd = reverse ? -1 : wLimit;
            int xStep = reverse ? -1 : 1;

            for (int x = xStart; x != xEnd; x += xStep) {
                auto& best_match = nnf[y * wLimit + x];
                best_match.first  = std::clamp(best_match.first,  0, wLimit - 1);
                best_match.second = std::clamp(best_match.second, 0, hLimit - 1);

                int best_dist = patchDistance(imageA, imageB, x, y,
                                              best_match.first, best_match.second,
                                              width, patchSize);

                // Propagation
                if (!reverse) {
                    if (x > 0) {
                        auto left = nnf[y * wLimit + (x - 1)];
                        left.first  = std::clamp(left.first,  0, wLimit - 1);
                        left.second = std::clamp(left.second, 0, hLimit - 1);

                        int dist = patchDistance(imageA, imageB, x, y,
                                                 left.first, left.second, width, patchSize);
                        if (dist < best_dist) {
                            best_match = left;
                            best_dist = dist;
                        }
                    }
                    if (y > 0) {
                        auto top = nnf[(y - 1) * wLimit + x];
                        top.first  = std::clamp(top.first,  0, wLimit - 1);
                        top.second = std::clamp(top.second, 0, hLimit - 1);

                        int dist = patchDistance(imageA, imageB, x, y,
                                                 top.first, top.second, width, patchSize);
                        if (dist < best_dist) {
                            best_match = top;
                            best_dist = dist;
                        }
                    }
                }

                // Random search
                int radius = std::max(width, height) / 2;
                while (radius >= 1) {
                    int min_x = std::max(0, best_match.first - radius);
                    int max_x = std::min(wLimit - 1, best_match.first + radius);
                    int min_y = std::max(0, best_match.second - radius);
                    int max_y = std::min(hLimit - 1, best_match.second + radius);

                    std::uniform_int_distribution<> randX(min_x, max_x);
                    std::uniform_int_distribution<> randY(min_y, max_y);

                    int rx = randX(gen);
                    int ry = randY(gen);

                    rx = std::clamp(rx, 0, wLimit - 1);
                    ry = std::clamp(ry, 0, hLimit - 1);

                    int dist = patchDistance(imageA, imageB, x, y, rx, ry, width, patchSize);
                    if (dist < best_dist) {
                        best_match = {rx, ry};
                        best_dist = dist;
                    }

                    radius /= 2;
                }
            }
        }
    }
}
