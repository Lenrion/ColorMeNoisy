#include "canvas2d.h"
#include "filterutils.h"
#include <iostream>

void Canvas2D::filterScale(float scaleX, float scaleY) {
    Canvas2D::scaleX(scaleX);
    Canvas2D::scaleY(scaleY);
}

int Canvas2D::getLeft(float c, float scale) {
    // if scale up
    if (scale >= 1.0) {
        return ceil(c - 1);
    }
    // if scale down
    else {
        return ceil(c - (1/scale));
    }
}

int Canvas2D::getRight(float c, float scale) {
    // if scale up
    if (scale >= 1.0) {
        return floor(c + 1);
    }
    // if scale down
    else {
        return floor(c + (1/scale));
    }
}

float Canvas2D::getRadius(float scale) {
    if (scale >= 1.0) {
        return 1.0;
    } else {
        return 1.0 / scale;
    }
}

void Canvas2D::scaleX(float a) {
    // make the new image vector based on scale amount of X
    int newWidth = floor(m_width * a); // scale columns
    std::vector<RGBAf> newImage(newWidth * m_height); // new width, old height
    // SCALE X
    for (int row = 0; row < m_height; row++) { // for row in new image
        for (int k = 0; k < newWidth; k++) { // for col in new image
            // get center on old image c = (k/a) + (1-a)/2a
            float c = (k/a) + ((1.0-a)/(2.0*a));
            // window radius = 1 (scaling up); 1/a (scaling down)
            float radius = getRadius(a);
            int left = getLeft(c, a);
            int right = getRight(c, a);

            float weights_sum = 0.0;
            float sum_r = 0.0;
            float sum_g = 0.0;
            float sum_b = 0.0;

            for (int pixel = left; pixel <= right; pixel++) {
                // int pixel_index = row * m_width + pixel;
                int pixel_index = Canvas2D::posToIndex(pixel, row); // index of pixel on old image
                RGBAf old_pixel; // get rgbaf value of old pixel
                if (!Canvas2D::inBounds(pixel, row)) { // not in bounds
                    old_pixel = RGBAf::fromRGBA(FilterUtils::getPixelReflected(m_data, m_width, m_height, pixel, row));
                } else { // in bounds
                    old_pixel = RGBAf::fromRGBA(m_data[pixel_index]);
                }
                float weight = FilterUtils::triangle(radius, c - pixel); // weight is output of triangle function

                // multiply each rgb val by weight and add to corresponding sum
                sum_r += weight * old_pixel.r;
                sum_g += weight * old_pixel.g;
                sum_b += weight * old_pixel.b;
                weights_sum += weight;
            }
            // finally sum up rgb vals, normalize by weight sum, add to newimage
            int index = row * newWidth + k;
            // int index = Canvas2D::posToIndex(k, row);
            newImage[index] = RGBAf({sum_r / weights_sum, sum_g / weights_sum, sum_b / weights_sum});
        }
    }

    m_width = newWidth; // UPDATE M_WIDTH
    // update image
    m_data = std::vector<RGBA>(newImage.size(), RGBA({0,0,0}));
    // Copy the RGBA data from `result` to `data`
    for (int i = 0; i < newImage.size(); i++) {
        // std::cout << newImage[i].r << std::endl;
        m_data[i] = newImage[i].toRGBA();
    }
}

void Canvas2D::scaleY(float a) {
    // make the new image vector based on scale amount of Y
    int newHeight = floor(m_height * a); // scale rows
    std::vector<RGBAf> newImage(m_width * newHeight); // new width, new height

    // SCALE Y
    for (int col = 0; col < m_width; col++) { // for col in new image
        for (int k = 0; k < newHeight; k++) { // for row in new image
            // get center on old image c = (k/a) + (1-a)/2a
            float c = (k/a) + ((1.0-a)/(2.0*a));
            // window radius = 1 (scaling up); 1/a (scaling down)
            float radius = getRadius(a);
            int top = getLeft(c, a);
            int bottom = getRight(c, a);

            float weights_sum = 0.0;
            float sum_r = 0.0;
            float sum_g = 0.0;
            float sum_b = 0.0;
            for (int pixel = top; pixel <= bottom; pixel++) {
                int pixel_index = Canvas2D::posToIndex(col, pixel); // index of pixel on old image
                RGBAf old_pixel; // get rgbaf value of old pixel
                if (!Canvas2D::inBounds(col, pixel)) { // not in bounds
                    old_pixel = RGBAf::fromRGBA(FilterUtils::getPixelReflected(m_data, m_width, m_height, col, pixel));
                } else { // in bounds
                    old_pixel = RGBAf::fromRGBA(m_data[pixel_index]);
                }
                float weight = FilterUtils::triangle(radius, c - pixel); // weight is output of triangle function
                // multiply each rgb val by weight and add to corresponding sum
                sum_r += weight * old_pixel.r;
                sum_g += weight * old_pixel.g;
                sum_b += weight * old_pixel.b;
                weights_sum += weight;
            }
            // finally sum up rgb vals, normalize by weight sum, add to newimage
            int index = k * m_width + col;
            // int index = Canvas2D::posToIndex(col, k);
            newImage[index] = RGBAf({sum_r / weights_sum, sum_g / weights_sum, sum_b / weights_sum});
        }
    }
    m_height = newHeight; // UPDATE M_HEIGHT
    // update image
    m_data = std::vector<RGBA>(newImage.size(), RGBA({0,0,0}));
    // Copy the RGBA data from `result` to `data`
    for (int i = 0; i < newImage.size(); i++) {
        // std::cout << result[j].r << std::endl;
        m_data[i] = newImage[i].toRGBA();
    }
}
