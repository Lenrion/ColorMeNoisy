#ifndef PATCHMATCH_H
#define PATCHMATCH_H
#include <eigen/Dense>
#include "rgba.h"
using namespace Eigen;

class PatchMatch
{
public:
    PatchMatch();
    float DistPatch(const Matrix2d & PatchA, const Matrix2d & PatchB, int PatchSize);
    void GuessAndImprove(const Matrix2d & SourceImage, const Matrix2d & TargetImage, const Matrix2d & mask,
                         int x , int y, int Guees_x, int guess_y, int PatchSize, Matrix2d & NearestNeighbor);
    static void patchmatch(const std::vector<RGBA>& imageA, const std::vector<RGBA>& imageB,
                    int width, int height, int patchSize,
                    std::vector<std::pair<int, int>>& nnf);
};

#endif // PATCHMATCH_H
