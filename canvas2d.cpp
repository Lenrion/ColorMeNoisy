#include "canvas2d.h"
#include <QPainter>
#include <QMessageBox>
#include <QFileDialog>
#include <iostream>
#include "settings.h"
#include <queue>

/**
 * @brief Initializes new 500x500 canvas
 */
void Canvas2D::init() {
    setMouseTracking(true);
    m_width = 500;
    m_height = 500;
    createMask();
    clearCanvas();
}

void Canvas2D::createMask() {
    mask = std::vector<float>(pow((2 * settings.brushRadius) + 1, 2), 0.0); // create vector of correct size
    smudge_vals = std::vector<RGBA>(pow((2 * settings.brushRadius) + 1, 2), RGBA{0,0,0,0});
    // center is (radius, radius)
    for (int i = 0; i < mask.size(); i++) {
        int row = std::floor(i / (settings.brushRadius * 2 + 1));
        int col = i % (settings.brushRadius * 2 + 1);
        float dist = sqrt(pow((settings.brushRadius - row), 2) + pow((settings.brushRadius - col), 2)); // dist from radius

        // mask is >0 where the distance is less than or equal to radius
        if (dist <= settings.brushRadius) {
            if (settings.brushType == BRUSH_CONSTANT || settings.brushType == BRUSH_SPRAY) {
                // intensity is 1 for constant brush
                mask[i] = 1.0;
            } else if (settings.brushType == BRUSH_LINEAR) {
                // opacity = 1 - (dist / radius)
                mask[i] = 1 - dist / settings.brushRadius;
            } else if (settings.brushType == BRUSH_QUADRATIC) {
                mask[i] = pow(1- dist / settings.brushRadius, 2);
            } else if (settings.brushType == BRUSH_SMUDGE) { // for smudge, use quadratic
                mask[i] = pow(1- dist / settings.brushRadius, 2);
            }

        } else {
            mask[i] = 0.0;
        }
    }
}

/**
 * @brief Canvas2D::clearCanvas sets all canvas pixels to blank white
 */
void Canvas2D::clearCanvas() {
    m_data.assign(m_width * m_height, RGBA{255, 255, 255, 255});
    settings.imagePath = "";
    displayImage();
}

/**
 * @brief Stores the image specified from the input file in this class's
 * `std::vector<RGBA> m_image`.
 * Also saves the image width and height to canvas width and height respectively.
 * @param file: file path to an image
 * @return True if successfully loads image, False otherwise.
 */
bool Canvas2D::loadImageFromFile(const QString &file) {
    QImage myImage;
    if (!myImage.load(file)) {
        std::cout<<"Failed to load in image"<<std::endl;
        return false;
    }
    myImage = myImage.convertToFormat(QImage::Format_RGBX8888);
    m_width = myImage.width();
    m_height = myImage.height();
    QByteArray arr = QByteArray::fromRawData((const char*) myImage.bits(), myImage.sizeInBytes());

    m_data.clear();
    m_data.reserve(m_width * m_height);
    for (int i = 0; i < arr.size() / 4; i++){
        m_data.push_back(RGBA{(std::uint8_t) arr[4*i], (std::uint8_t) arr[4*i+1], (std::uint8_t) arr[4*i+2], (std::uint8_t) arr[4*i+3]});
    }
    displayImage();
    return true;
}

/**
 * @brief Saves the current canvas image to the specified file path.
 * @param file: file path to save image to
 * @return True if successfully saves image, False otherwise.
 */
bool Canvas2D::saveImageToFile(const QString &file) {
    QImage myImage = QImage(m_width, m_height, QImage::Format_RGBX8888);
    for (int i = 0; i < m_data.size(); i++){
        myImage.setPixelColor(i % m_width, i / m_width, QColor(m_data[i].r, m_data[i].g, m_data[i].b, m_data[i].a));
    }
    if (!myImage.save(file)) {
        std::cout<<"Failed to save image"<<std::endl;
        return false;
    }
    return true;
}


/**
 * @brief Get Canvas2D's image data and display this to the GUI
 */
void Canvas2D::displayImage() {
    QByteArray img(reinterpret_cast<const char *>(m_data.data()), 4 * m_data.size());
    QImage now = QImage((const uchar*)img.data(), m_width, m_height, QImage::Format_RGBX8888);
    setPixmap(QPixmap::fromImage(now));
    setFixedSize(m_width, m_height);
    update();
}

/**
 * @brief Canvas2D::resize resizes canvas to new width and height
 * @param w
 * @param h
 */
void Canvas2D::resize(int w, int h) {
    m_width = w;
    m_height = h;
    m_data.resize(w * h);
    displayImage();
}

/**
 * @brief Called when the filter button is pressed in the UI
 */
void Canvas2D::filterImage() {
    // Filter TODO: apply the currently selected filter to the loaded image
    switch (settings.filterType) {
        case FILTER_BLUR:
            break;
        case FILTER_EDGE_DETECT:
            Canvas2D::noiseSetup(settings.edgeDetectSensitivity);
            break;
        case FILTER_SCALE:
            break;
        case FILTER_ROTATION:
            Canvas2D::filterRotate(settings.rotationAngle);
            break;
        default:
            break;
    }

    displayImage();
}

/**
 * @brief Called when any of the parameters in the UI are modified.
 */
void Canvas2D::settingsChanged() {
    // this saves your UI settings locally to load next time you run the program
    settings.saveSettings();

    // TODO: fill in what you need to do when brush or filter parameters change
    createMask();
}

/**
 * @brief Convert (x,y) position to index on canvas.
 * @param x location
 * @param y location
 * @return index on canvas
 */
int Canvas2D::posToIndex(int x, int y) {
    // convert (x, y) position into index
    int index = y * m_width + x;
    return index;
}

int Canvas2D::posWidthToIndex(int x, int y, int width) {
    // convert (x, y) position into index
    int index = y * width + x;
    return index;
}

/**
 * @brief Paints the area around (x,y), taking brush size, color, and alpha into account.
 * @param x: x location
 * @param y: y location
 */
void Canvas2D::paint(int x, int y) {
    for (int i = 0; i < mask.size(); i++) {
        // find pos on canvas to paint
        int row = std::floor(i / (settings.brushRadius * 2 + 1));
        int col = i % (settings.brushRadius * 2 + 1);
        int absolute_x = x - (settings.brushRadius - col);
        int absolute_y = y - (settings.brushRadius - row);

        // bounds checking
        if (inBounds(absolute_x, absolute_y)) {
            // set canvas to color
            float alpha_norm = settings.brushColor.a / 255.0; // normalize alpha value by 255.0!!
            float intensity = alpha_norm * mask[i]; // M * a
            float smudge_intensity = mask[i] * (smudge_vals[i].a / 255.0);
            RGBA canvas_color = m_data[posToIndex(absolute_x, absolute_y)];
            // store as floats to do ops
            float r_val;
            float g_val;
            float b_val;
            if (settings.brushType == BRUSH_SMUDGE) {
                // calculate new color based on mix of previous & current canvas color + intensity (should always be full for smudge)
                r_val = (smudge_intensity * (smudge_vals[i].r / 255.0) + ((1 - smudge_intensity) * (canvas_color.r / 255.0)) ) * 255;
                g_val = (smudge_intensity * (smudge_vals[i].g / 255.0) + ((1 - smudge_intensity) * (canvas_color.g / 255.0)) ) * 255;
                b_val = (smudge_intensity * (smudge_vals[i].b / 255.0) + ((1 - smudge_intensity) * (canvas_color.b / 255.0)) ) * 255;
                // assign one by one (darkening + casting issue when you assign RGBA all at once)
                m_data[posToIndex(absolute_x, absolute_y)].r = r_val + 0.5f;
                m_data[posToIndex(absolute_x, absolute_y)].g = g_val + 0.5f;
                m_data[posToIndex(absolute_x, absolute_y)].b = b_val + 0.5f;
            } else {
                // calculate new color based on mix of brush and canvas color and intensity
                r_val = (intensity * (settings.brushColor.r / 255.0) + ((1 - intensity) * (canvas_color.r / 255.0)) ) * 255;
                g_val = (intensity * (settings.brushColor.g / 255.0) + ((1 - intensity) * (canvas_color.g / 255.0)) ) * 255;
                b_val = (intensity * (settings.brushColor.b / 255.0) + ((1 - intensity) * (canvas_color.b / 255.0)) ) * 255;

                if (settings.brushType != BRUSH_SPRAY) {
                    // set canvas to color
                    m_data[posToIndex(absolute_x, absolute_y)].r = r_val + 0.5f;
                    m_data[posToIndex(absolute_x, absolute_y)].g = g_val + 0.5f;
                    m_data[posToIndex(absolute_x, absolute_y)].b = b_val + 0.5f;
                } else {
                    // spray brush functionality!
                    if (arc4random() % 100 < settings.brushDensity) {
                        m_data[posToIndex(absolute_x, absolute_y)].r = r_val + 0.5f;
                        m_data[posToIndex(absolute_x, absolute_y)].g = g_val + 0.5f;
                        m_data[posToIndex(absolute_x, absolute_y)].b = b_val + 0.5f;
                    }
                }
            }



        }
    }
}

/**
 * @brief Pick up color from a given (x,y) loc on canvas for use with smudge brush.
 * @param x loc
 * @param y loc
 */
void Canvas2D::pickUpColor(int x, int y) {
    // bound check
    if (inBounds(x, y)) {
        for (int i = 0; i < smudge_vals.size(); i++) {
            // find pos on canvas to pick up color
            int row = std::floor(i / (settings.brushRadius * 2 + 1));
            int col = i % (settings.brushRadius * 2 + 1);
            int absolute_x = x - (settings.brushRadius - col);
            int absolute_y = y - (settings.brushRadius - row);
            smudge_vals[i] = m_data[posToIndex(absolute_x, absolute_y)];

        }
    }
}

/**
 * @brief Fill relevant area given (x,y) of click
 * @param x loc of click
 * @param y loc of click
 */
void Canvas2D::fill(int x, int y) {
    RGBA firstColor = m_data[posToIndex(x, y)]; // color of first pixel
    RGBA newColor = settings.brushColor; // color to fill (also used to check for visited)
    std::queue<std::pair<int, int>> q; // store all pixels to visit

    q.push(std::make_pair(x, y)); // push first pixel
    m_data[posToIndex(x, y)] = newColor; // mark visited

    while (!q.empty()) {
        // pop pixel from queue
        std::pair<int, int> curr = q.front();
        q.pop();



        // if pixel is in bounds, the same color as the first pixel, and not visited (not the color we want to fill), add it to the queue
        if (inBounds(curr.first+1, curr.second) && sameColor(m_data[posToIndex(curr.first+1, curr.second)], firstColor) && !sameColor(m_data[posToIndex(curr.first+1, curr.second)], newColor)) {
            q.push(std::make_pair(curr.first+1, curr.second));
            // change color of pixel on canvas/mark as visited
            m_data[posToIndex(curr.first+1, curr.second)] = newColor;
        }
        if (inBounds(curr.first, curr.second+1) && sameColor(m_data[posToIndex(curr.first, curr.second+1)], firstColor)&& !sameColor(m_data[posToIndex(curr.first, curr.second+1)], newColor)) {
            q.push(std::make_pair(curr.first, curr.second+1));
            // change color of pixel on canvas/mark as visited
            m_data[posToIndex(curr.first, curr.second+1)] = newColor;
        }
        if (inBounds(curr.first-1, curr.second) && sameColor(m_data[posToIndex(curr.first-1, curr.second)], firstColor)&& !sameColor(m_data[posToIndex(curr.first-1, curr.second)], newColor)) {
            q.push(std::make_pair(curr.first-1, curr.second));
            // change color of pixel on canvas/mark as visited
            m_data[posToIndex(curr.first-1, curr.second)] = newColor;
        }
        if (inBounds(curr.first, curr.second-1) && sameColor(m_data[posToIndex(curr.first, curr.second-1)], firstColor)&& !sameColor(m_data[posToIndex(curr.first, curr.second-1)], newColor)) {
            q.push(std::make_pair(curr.first, curr.second-1));
            // change color of pixel on canvas/mark as visited
            m_data[posToIndex(curr.first, curr.second-1)] = newColor;
        }

    }

}

/**
 * @brief Helper function to check if loc is in bounds
 * @param x loc
 * @param y loc
 * @return
 */
bool Canvas2D::inBounds(int x, int y) {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        return true;
    }
    return false;
}

/**
 * @brief Helper function to check if two colors are the same
 * @param color1
 * @param color2
 * @return
 */
bool Canvas2D::sameColor(RGBA color1, RGBA color2) {
    if (color1.r == color2.r
        && color1.g == color2.g
        && color1.b == color2.b
        && color1.a == color2.a) {
        return true;
    }
    return false;
}

/**
 * @brief These functions are called when the mouse is clicked and dragged on the canvas
 */
void Canvas2D::mouseDown(int x, int y) {
    mouseIsDown = true;
    if (settings.brushType == BRUSH_FILL) {
        fill(x, y);
    } else {
        if (settings.brushType == BRUSH_SMUDGE) {
            pickUpColor(x, y);
        } else {
            paint(x, y);
        }
    }

    displayImage();

}

void Canvas2D::mouseDragged(int x, int y) {
    if (mouseIsDown) {
        paint(x, y);
        if (settings.brushType == BRUSH_SMUDGE) {
            pickUpColor(x, y);
        }
        displayImage();
    }
}

void Canvas2D::mouseUp(int x, int y) {
    mouseIsDown = false;
}
