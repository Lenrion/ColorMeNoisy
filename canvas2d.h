#ifndef CANVAS2D_H
#define CANVAS2D_H

#include <QLabel>
#include <QMouseEvent>
#include <array>
#include "rgba.h"

class Canvas2D : public QLabel {
    Q_OBJECT
public:
    int m_width = 0;
    int m_height = 0;
    std::vector<float> mask = std::vector<float>();
    std::vector<RGBA> smudge_vals = std::vector<RGBA>();

    void init();
    void clearCanvas();
    bool loadImageFromFile(const QString &file);
    bool saveImageToFile(const QString &file);
    void displayImage();
    void resize(int w, int h);

    // This will be called when the settings have changed
    void settingsChanged();

    // Filter TODO: implement
    void filterImage();
    void filterBlur(int blurRadius);
    void filterEdgeDetect(float sensitivity);
    void filterGray();
    void filterScale(float scaleX, float scaleY);
    void filterRotate(float angle);
    void filterDownsampleTest();
    void filterUpsampleTest();
    void filterPyramidResampleTest();
    void filterPyramidProcessTest();
    void patchmatch(const std::vector<RGBA>& imageA, const std::vector<RGBA>& imageB,
                              int width, int height, int patchSize,
                              std::vector<std::pair<int, int>>& nnf);
    void reconstructImage(
        const std::vector<RGBA>& sourceImage,     // Source image (content)
        const std::vector<RGBA>& targetImage,     // Target image (style)
        int width, int height,                    // Source dimensions
        int patchSize,                            // Patch size
        const std::vector<std::pair<int, int>>& nnf, // NNF from source to target
        std::vector<RGBA>& outputImage            // Output image
        );
    QImage padTextureToMatchFrame(const QImage& texture, int frameWidth, int frameHeight);
    std::vector<RGBA> m_data;

    void mouseDown(int x, int y);
    void mouseDragged(int x, int y);
    void mouseUp(int x, int y);

    bool mouseIsDown;

    // These are functions overriden from QWidget that we've provided
    // to prevent you from having to interact with Qt's mouse events.
    // These will pass the mouse coordinates to the above mouse functions
    // that you will have to fill in.
    virtual void mousePressEvent(QMouseEvent* event) override {
        auto [x, y] = std::array{ event->position().x(), event->position().y() };
        mouseDown(static_cast<int>(x), static_cast<int>(y));
    }
    virtual void mouseMoveEvent(QMouseEvent* event) override {
        auto [x, y] = std::array{ event->position().x(), event->position().y() };
        mouseDragged(static_cast<int>(x), static_cast<int>(y));
    }
    virtual void mouseReleaseEvent(QMouseEvent* event) override {
        auto [x, y] = std::array{ event->position().x(), event->position().y() };
        mouseUp(static_cast<int>(x), static_cast<int>(y));
    }

    // TODO: add any member variables or functions you need
    void createMask();
    void paint(int x, int y);
    int posToIndex(int x, int y);
    int posWidthToIndex(int x, int y, int width);
    void pickUpColor(int x, int y);
    void fill(int x, int y);
    bool inBounds(int x, int y);
    bool sameColor(RGBA color1, RGBA color2);
    int getLeft(float c, float scale);
    int getRight(float c, float scale);
    float getRadius(float scale);
    void scaleX(float a);
    void scaleY(float a);
    int getNewWidth(float angle);
    int getNewHeight(float angle);
};

#endif // CANVAS2D_H
