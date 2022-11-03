#include "display.h"
#include "qevent.h"
#include <cassert>

namespace pv {

    Display::Display(size_t width, size_t height, const SceneData& sceneData, QWidget *parent):
        QLabel{parent},
        width_(width),
        height_(height),
        rend_pipeline_(sceneData),
        mouse_pressed_(false),
        pressed_x_(0),
        pressed_y_(0) { }

    void Display::DeferAnimationType(ANIMATION_TYPE animationType) {
        rend_pipeline_.SetAnimationType(animationType);
    }

    void Display::DeferXCameraView() {
        rend_pipeline_.SetXCameraView();
    }

    void Display::DeferYCameraView() {
        rend_pipeline_.SetYCameraView();
    }

    void Display::DeferZCameraView() {
        rend_pipeline_.SetZCameraView();
    }

    void Display::DeferNewFovYAngleValue(float fovy) {
        rend_pipeline_.SetFOVYDegreeAngle(fovy);
    }

    void Display::DeferNewNearPlaneDistance(float near) {
        rend_pipeline_.SetNearPlaneDistance(near);
    }

    void Display::DeferNewFarPlaneDistance(float far) {
        rend_pipeline_.SetFarPlaneDistance(far);
    }

    void Display::DeferDrawWorldAxis(bool drawWorldAxis) {
        rend_pipeline_.SetDrawWorldAxis(drawWorldAxis);
    }

    void Display::DeferDrawPolygonMesh(bool drawPolygonMesh) {
        rend_pipeline_.SetDrawPolygonMesh(drawPolygonMesh);
    }

    void Display::DeferRasterizePolygons(bool rasterizePolygons) {
        rend_pipeline_.SetRasterizePolygons(rasterizePolygons);
    }

    void Display::DeferNewOrbitCameraDistance(float distance) {
        rend_pipeline_.SetOrbitCameraDistance(distance);
    }

    void Display::DeferNewPenColor(QColor &penColor) {
        rend_pipeline_.SetNewPenColor({255,
                                       static_cast<unsigned char>(penColor.red()),
                                       static_cast<unsigned char>(penColor.green()),
                                       static_cast<unsigned char>(penColor.blue())});
    }

    void Display::DeferNewBrushColor(QColor &penColor) {
        rend_pipeline_.SetNewBrushColor({255,
                                         static_cast<unsigned char>(penColor.red()),
                                         static_cast<unsigned char>(penColor.green()),
                                         static_cast<unsigned char>(penColor.blue())});
    }

    void Display::DeferEnableZBuffering(bool enableZBuffering) {
        rend_pipeline_.SetEnableZBuffering(enableZBuffering);
    }

    void Display::DeferEnableBackfaceCulling(bool enableBackfaceCulling) {
        rend_pipeline_.SetEnableBackfaceCulling(enableBackfaceCulling);
    }

    void Display::DeferNewLightPosition(float lightPositionDegrees) {
        rend_pipeline_.SetNewLightPosition(lightPositionDegrees);
    }

    void Display::DeferEnableLambertianModel(bool enableLambertianModel) {
        rend_pipeline_.SetEnableLambertianModel(enableLambertianModel);
    }

    void renderedImageCleanup(void* renderedImage){
        uchar* toBeDeleted = static_cast<uchar*>(renderedImage);
        delete[] toBeDeleted;
    }

    void Display::paintEvent(QPaintEvent *event) {

        constexpr size_t ARGB32_COMP_COUNT = 4;
        const size_t renderedImageLength = width_ * height_ * ARGB32_COMP_COUNT;
        uchar* renderedImage = new uchar[renderedImageLength];

        rend_pipeline_.DoRender(width_, height_, renderedImage);

        QImage image(renderedImage, width_, height_, QImage::Format::Format_ARGB32, renderedImageCleanup, renderedImage);
        this->setPixmap(QPixmap::fromImage(image));

        QLabel::paintEvent(event);
    }

    void Display::wheelEvent(QWheelEvent *event) {
        static float scaleFactor = 1.0;
        int yScroll = event->angleDelta().y();

        if (yScroll > 0){
            scaleFactor *= 1.5;
        } else {
            scaleFactor /= 1.5;
        }

        rend_pipeline_.SetModelScaleFactor(scaleFactor);
    }

    void Display::mousePressEvent(QMouseEvent *event) {
        mouse_pressed_ = true;

        pressed_x_ = event->pos().x();
        pressed_y_ = event->pos().y();
    }

    void Display::mouseReleaseEvent(QMouseEvent *event) {
        mouse_pressed_ = false;
    }

    void Display::mouseMoveEvent(QMouseEvent *event) {
        if (mouse_pressed_){
            int newX = event->pos().x();
            int newY = event->pos().y();

            int deltaX = newX - pressed_x_;
            int deltaY = newY - pressed_y_;

            rend_pipeline_.UpdateCameraPosition(deltaX, deltaY);

            pressed_x_ = newX;
            pressed_y_ = newY;
        }
    }
}
