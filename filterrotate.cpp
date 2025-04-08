#include "canvas2d.h"
#include "filterutils.h"
#include <iostream>

void Canvas2D::filterRotate(float angle) {
    float theta = angle * M_PI / 180.0; // convert to radians

    // get center coords
    int cx = m_width / 2;
    int cy = m_height / 2;

    // make new image vector
    // width and height based on corners of the rotated image
    int newWidth = getNewWidth(angle);
    int newHeight = getNewHeight(angle);
    std::vector<RGBAf> newImage(newWidth * newHeight);

    // get center of new image
    int cx_new = newWidth / 2;
    int cy_new = newHeight / 2;

    // loop thru pixels in new image
    for (int row = 0; row < newHeight; row++) {
        for (int col = 0; col < newWidth; col++) {
            // backmap using inverse of rotation formula from lecture
            // this is because we are starting from the new image and figuring out where in the old image we want to get
            // the pixel from, so we are reverse-rotating it!
            // shift row and col by center of new image to ensure rotation around center instead of (0,0) (i add it back later)
            // round because we are looking for indices. this results in the image getting really scrungly over time,
            // but i hope it is ok for the scope of this project :)
            // if i had more time, i would go in and implement a way to get an average of the surrounding pixels!
            int old_x = round(cos(-theta) * (col - cx_new) - sin(-theta) * (row - cy_new) + cx); // add center back
            int old_y = round(sin(-theta) * (col - cx_new) + cos(-theta) * (row - cy_new) + cy);

            // if the original coordinates are within bounds, then it's part of the original image
            // that means you should update the new image with RGB vals from old index!
            if (old_x >= 0 && old_x < m_width && old_y >= 0 && old_y < m_height) {
                // get closest matching color
                // RGBAf pixel = getClosestPixel(old_x, old_y);
                RGBAf pixel = RGBAf::fromRGBA(m_data[posToIndex(old_x, old_y)]);
                newImage[Canvas2D::posWidthToIndex(col, row, newWidth)] = pixel; // update new image
            } else { // otherwise, it should be black
                newImage[Canvas2D::posWidthToIndex(col, row, newWidth)] = RGBAf({0, 0, 0});
            }
        }
    }

    // update actual image
    m_width = newWidth;
    m_height = newHeight;
    m_data = std::vector<RGBA>(newImage.size());
    for (int i = 0; i < newImage.size(); i++) {
        m_data[i] = newImage[i].toRGBA();
    }
}

int Canvas2D::getNewWidth(float angle) {
    // get x val of each of the 4 corners after rotation
    // use the max x val as the new width
    // x_new = cos(theta) * (x-cx) - sin(theta) * (y-cy) + cx
    float theta = angle * M_PI / 180.0;
    int cx = m_width / 2;
    int cy = m_height / 2;
    // first corner at x = 0, y = 0
    int new_x_1 = round(cos(theta) * (0 - cx) - sin(theta) * (0 - cy) + cx);
    // second corner at x = m_width, y = 0
    int new_x_2 = round(cos(theta) * (m_width - cx) - sin(theta) * (0 - cy) + cx);
    // third corner at x = 0, y = m_height
    int new_x_3 = round(cos(theta) * (0 - cx) - sin(theta) * (m_height - cy) + cx);
    // fourth corner at x = m_width, y = m_height
    int new_x_4 = round(cos(theta) * (m_width - cx) - sin(theta) * (m_height - cy) + cx);

    return std::max({new_x_1, new_x_2, new_x_3, new_x_4}) - std::min({new_x_1, new_x_2, new_x_3, new_x_4});
}
int Canvas2D::getNewHeight(float angle) {
    // get y val of each of the 4 corners after rotation
    // use the max - min y val as the new height
    // y_new = sin(theta) * (x-cx) + cos(theta) * (y-cy) + cy
    float theta = angle * M_PI / 180.0;
    int cx = m_width / 2;
    int cy = m_height / 2;
    // first corner at x = 0, y = 0
    int new_y_1 = round(sin(theta) * (0 - cx) + cos(theta) * (0 - cy) + cy);
    // second corner at x = m_width, y = 0
    int new_y_2 = round(sin(theta) * (m_width - cx) + cos(theta) * (0 - cy) + cy);
    // third corner at x = 0, y = m_height
    int new_y_3 = round(sin(theta) * (0 - cx) + cos(theta) * (m_height - cy) + cy);
    // fourth corner at x = m_width, y = m_height
    int new_y_4 = round(sin(theta) * (m_width - cx) + cos(theta) * (m_height - cy) + cy);

    return std::max({new_y_1, new_y_2, new_y_3, new_y_4}) - std::min({new_y_1, new_y_2, new_y_3, new_y_4});
}
