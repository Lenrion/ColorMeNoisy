#include "patchmatch.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <cmath>
#include <Eigen/Dense>
using namespace Eigen;

PatchMatch::PatchMatch() {

}

// Structure to represent a pixel
struct Pixel {
    unsigned char r, g, b;
};

// Function to calculate the squared distance between two patches
int patchDistance(const std::vector<Pixel>& patch1, const std::vector<Pixel>& patch2) {
    int distance = 0;
    for (size_t i = 0; i < patch1.size(); ++i) {
        distance += (patch1[i].r - patch2[i].r) * (patch1[i].r - patch2[i].r);
        distance += (patch1[i].g - patch2[i].g) * (patch1[i].g - patch2[i].g);
        distance += (patch1[i].b - patch2[i].b) * (patch1[i].b - patch2[i].b);
    }
    return distance;
}

std::vector<Pixel> extractPatch(const std::vector<Pixel>& image, int x, int y, int width, int patchSize) {
    std::vector<Pixel> patch;
    for (int dy = 0; dy < patchSize; ++dy) {
        patch.insert(patch.end(), image.begin() + (y + dy) * width + x, image.begin() + (y + dy) * width + x + patchSize);
    }
    return patch;
}


void patchmatch(const std::vector<Pixel>& imageA, const std::vector<Pixel>& imageB, int width, int height, int patchSize, std::vector<std::pair<int, int>>& nnf) {
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
                            std::vector<Pixel>(imageA.begin() + y * width + x, imageA.begin() + y * width + x + patchSize * width + patchSize),
                            std::vector<Pixel>(imageB.begin() + newMatchY * width + newMatchX, imageB.begin() + newMatchY * width + newMatchX + patchSize * width + patchSize)
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
                            std::vector<Pixel>(imageA.begin() + y * width + x, imageA.begin() + y * width + x + patchSize * width + patchSize),
                            std::vector<Pixel>(imageB.begin() + newMatchY * width + newMatchX, imageB.begin() + newMatchY * width + newMatchX + patchSize * width + patchSize)
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
                            std::vector<Pixel>(imageA.begin() + y * width + x, imageA.begin() + y * width + x + patchSize * width + patchSize),
                            std::vector<Pixel>(imageB.begin() + newMatchY * width + newMatchX, imageB.begin() + newMatchY * width + newMatchX + patchSize * width + patchSize)
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

// float PatchMatch::DistPatch(const Matrix2d & PatchA, const Matrix2d & PatchB, int PatchSize)
// {
//     return PatchSize * PatchSize * norm(PatchA, PatchB);
// }


// void PatchMatch::GuessAndImprove(const Matrix2d & SourceImage, const Matrix2d & TargetImage, const Matrix2d & mask,
//                      int x , int y, int Guees_x, int guess_y, int PatchSize, Matrix2d & NearestNeighbor)
// {
//     if (x == Guees_x && y == guess_y)
//     {
//         return;
//     }

//     Rect RE2(Guees_x, guess_y, PatchSize, PatchSize);

//     Matrix2d CurMask = mask(RE2);
//     int unValidNum = 0;
//     for (int i = 0; i < CurMask.rows(); i++)
//     {
//         for (int j = 0; j < CurMask.cols(); j++)
//         {
//             if (CurMask(i, j))
//             {
//                 unValidNum++;
//             }
//         }
//     }

//     if (unValidNum  * 10 > CurMask.rows() * CurMask.cols())
//     {
//         return;
//     }

//     Rect RE(x, y, PatchSize, PatchSize);

//     Matrix2d patchA = SourceImage(RE);
//     Matrix2d patchB = TargetImage(RE2);

//     int CurDist = DistPatch(patchA, patchB, PatchSize);

//     int CurBestDist = NearestNeighbor(y, x)[2]; // TODO we want the best dist?

//     if (CurDist < CurBestDist)
//     {
//         NearestNeighbor.at<Eigen::Vector3i>(y, x)[0] = Guees_x;
//         NearestNeighbor.at<Eigen::Vector3i>(y, x)[1] = guess_y;
//         NearestNeighbor.at<Eigen::Vector3i>(y, x)[2] = CurDist;
//     }
// }

// void PatchMatch::PatchMatch(const Matrix2d & SourceImage,const Matrix2d & TargetImage, const Matrix2d & Mask,  int nPatchSize, Matrix2d & NearestNeighbor)
// {
//     // 最近邻数据
//     NearestNeighbor = Matrix2d::zeros(SourceImage.size(), CV_32SC3);

//     int nIterNum = 0;
//     int nIterMaxNum = 5;
//     int32_t nMaxCols = TargetImage.cols - nPatchSize - 1;
//     int32_t nMaxRows = TargetImage.rows - nPatchSize - 1;

//     // 先设置随机位置
//     for (int i = 0; i < SourceImage.rows - nPatchSize; i++)
//     {
//         for (int j = 0; j < SourceImage.cols - nPatchSize; j++)
//         {
//             int nRandX = rand() % (nMaxCols);   // x 坐标
//             int nRandY = rand() % (nMaxRows);   // y 坐标

//             NearestNeighbor.at<Vec3i>(i, j)[0] = nRandX;
//             NearestNeighbor.at<Vec3i>(i, j)[1] = nRandY;

//             Rect RE(j, i, nPatchSize, nPatchSize);
//             Mat patchA = SourceImage(RE);

//             Rect RE2(nRandX, nRandY, nPatchSize, nPatchSize);
//             Mat patchB = TargetImage(RE2);

//             NearestNeighbor.at<Vec3i>(i,j)[2] = DistPatch(patchA, patchB, nPatchSize);

//         }
//     }

//     while (nIterNum < nIterMaxNum)
//     {
//         int nColStart = 1;
//         int nColEnd = SourceImage.cols - nPatchSize ;
//         int nRowStart = 1;
//         int nRowEnd = SourceImage.rows - nPatchSize ;
//         int nStep = 1;

//         if (nIterNum % 2)
//         {
//             nColStart = SourceImage.cols - nPatchSize - 2;
//             nColEnd = -1;
//             nRowStart = SourceImage.rows - nPatchSize - 2;
//             nRowEnd = -1;
//             nStep = -1;
//         }


//         for (int i = nRowStart; i != nRowEnd; i += nStep)
//         {
//             for (int j = nColStart; j != nColEnd; j += nStep)
//             {
//                 // 有效范围内
//                 if (j - nStep < SourceImage.cols - nPatchSize)
//                 {
//                     int nGuessX = NearestNeighbor.at<Vec3i>(i, j - nStep)[0] + nStep;
//                     int nGuessY = NearestNeighbor.at<Vec3i>(i, j - nStep)[1];

//                     if (nGuessX < TargetImage.cols - nPatchSize && nGuessX >= 0)
//                     {
//                         // propagation
//                         GuessAndImprove(SourceImage, TargetImage, Mask, j, i, nGuessX, nGuessY, nPatchSize, NearestNeighbor);
//                     }

//                 }
//                 // 有效范围内
//                 if (i - nStep < SourceImage.rows - nPatchSize)
//                 {
//                     int nGuessX = NearestNeighbor.at<Vec3i>(i - nStep, j)[0];
//                     int nGuessY = NearestNeighbor.at<Vec3i>(i - nStep, j)[1] + nStep;

//                     if (nGuessY < TargetImage.rows - nPatchSize && nGuessY >= 0)
//                     {
//                         // propagation
//                         GuessAndImprove(SourceImage, TargetImage, Mask, j, i, nGuessX, nGuessY, nPatchSize, NearestNeighbor);
//                     }
//                 }

//                 // random guess

//                 int rs_start = max(TargetImage.rows,  TargetImage.cols);

//                 int nBestX = NearestNeighbor.at<Vec3i>(i, j)[0];
//                 int nBestY = NearestNeighbor.at<Vec3i>(i, j)[1];

//                 for (int mag = rs_start; mag >= 1; mag /= 2)
//                 {
//                     /* Sampling window */
//                     int xmin = max(nBestX - mag, 0), xmax = min(nBestX + mag + 1, TargetImage.cols - nPatchSize - 1);
//                     int ymin = max(nBestY - mag, 0), ymax = min(nBestY + mag + 1, TargetImage.rows - nPatchSize - 1);

//                     int xp = xmin + rand() % (xmax - xmin);
//                     int yp = ymin + rand() % (ymax - ymin);

//                     GuessAndImprove(SourceImage, TargetImage, Mask, j, i, xp, yp,  nPatchSize, NearestNeighbor);

//                 }
//             }
//         }

//         nIterNum++;
//     }
// }
