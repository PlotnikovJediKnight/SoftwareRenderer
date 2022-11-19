#ifndef DISPLAY_H
#define DISPLAY_H

#include <QLabel>
#include "animation.h"
#include "shadingmodel.h"
#include "scenedata.h"
#include "renderingpipeline.h"
#include "lightsourcelistmodel.h"

namespace pv {

    class Display : public QLabel {
        Q_OBJECT
    public:
        explicit Display(size_t width, size_t height, const SceneData& sceneData, QWidget *parent = nullptr);

        void DeferAnimationType(ANIMATION_TYPE animationType);
        void DeferShadingType(SHADING_MODEL shadingType);

        void DeferXCameraView();
        void DeferYCameraView();
        void DeferZCameraView();

        void DeferNewFovYAngleValue(float fovy);
        void DeferNewNearPlaneDistance(float near);
        void DeferNewFarPlaneDistance(float far);

        void DeferDrawWorldAxis(bool drawWorldAxis);
        void DeferDrawPolygonMesh(bool drawPolygonMesh);
        void DeferRasterizePolygons(bool rasterizePolygons);

        void DeferNewOrbitCameraDistance(float distance);

        void DeferNewPenColor(QColor& penColor);
        void DeferNewBrushColor(QColor& penColor);

        void DeferEnableZBuffering(bool enableZBuffering);
        void DeferEnableBackfaceCulling(bool enableBackfaceCulling);

        void DeferUpdatedLightSourceListModel(const LightSourceListModel* model);

    signals:

        // QWidget interface
    protected:
        virtual void paintEvent(QPaintEvent *event) override;
        virtual void wheelEvent(QWheelEvent *event) override;

    private:
        size_t width_;
        size_t height_;
        RenderingPipeline rend_pipeline_;
        bool mouse_pressed_;

        int pressed_x_;
        int pressed_y_;

        // QWidget interface
    protected:
        virtual void mousePressEvent(QMouseEvent *event) override;
        virtual void mouseReleaseEvent(QMouseEvent *event) override;
        virtual void mouseMoveEvent(QMouseEvent *event) override;
    };

}

#endif // DISPLAY_H
