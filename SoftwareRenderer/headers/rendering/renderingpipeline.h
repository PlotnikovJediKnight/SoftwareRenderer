#ifndef PV_RENDERINGPIPELINE_H
#define PV_RENDERINGPIPELINE_H

#include "headers/matrix_transform/animation.h"
#include "headers/rendering/scenedata.h"
#include "headers/rendering/framebuffer.h"
#include "headers/matrix_transform/camera.h"
#include "headers/rendering/polygonedge.h"
#include "headers/rendering/viewportpolygonmargins.h"
#include "headers/shading/lightsource.h"
#include "headers/shading/shadingmodel.h"
#include <vector>
#include <memory>
#include <optional>
#include <QVector>
#include <glm/mat4x4.hpp>

namespace pv {

    using AnimationHolder = std::unique_ptr<Animation>;
    using ShadingModelHolder = std::unique_ptr<ShadingModel>;
    using ViewportPoint = glm::vec4;

    class RenderingPipeline {
    public:
        RenderingPipeline(const SceneData&);

        void DoRender(size_t width, size_t height, uchar* renderedImage);

        void SetNearPlaneDistance(float near);
        void SetFarPlaneDistance(float far);
        void SetFOVYDegreeAngle(float fovyDegrees);
        void SetModelScaleFactor(float scaleFactor);

        void SetAnimationType(ANIMATION_TYPE animationType);
        void SetShadingModelType(SHADING_MODEL shadingType);

        void SetDrawWorldAxis(bool drawWorldAxis);
        void SetDrawPolygonMesh(bool drawPolygons);
        void SetRasterizePolygons(bool rasterizePolygons);

        void SetXCameraView();
        void SetYCameraView();
        void SetZCameraView();

        void UpdateCameraPosition(int deltaX, int deltaY);
        void SetOrbitCameraDistance(float distance);

        void SetNewPenColor(const std::array<uchar, 4>& argbPenColor);
        void SetNewBrushColor(const std::array<uchar, 4>& argbBrushColor);

        void SetEnableZBuffering(bool enableZBuffering);
        void SetEnableBackfaceCulling(bool enableBackfaceCulling);

        void SetLightSources(std::vector<std::shared_ptr<LightSource>> lightSources);

        void SetEnableDiffuseTexturing(bool diffuseEnable);
        void SetEnableNormalTexturing(bool normalEnable);
        void SetEnableSpecularTexturing(bool specularEnable);

        void SetDiffuseTexture(TextureHolder texture);
        void SetNormalTexture(TextureHolder texture);
        void SetSpecularTexture(TextureHolder texture);

    private:
        void ApplyScaleFactor(glm::mat4& modelMatrix);

        bool WCoordinateIsNonZero(float w);
        bool PointIsWithinCanonicalViewVolume(const glm::vec4& point);
        bool PointIsWithinViewportBoundaries(size_t width, size_t height, const glm::vec3& point);

        std::vector<std::optional<ViewportPoint>> GetViewPortPoints(const std::vector<glm::vec3>& points, float aspectRatio, size_t width, size_t height);
        glm::mat4 GetFrustumProjection(float aspectRatio);
        glm::mat4 GetViewportTransform(size_t width, size_t height);
        float GetRadianAngle(float degreeAngle);

        void RenderWorldAxes(FrameBuffer& frameBuffer);
        void RenderPolygonMesh(FrameBuffer& frameBuffer, const std::vector<std::optional<ViewportPoint>>& viewportPoints, const SceneData&);
        void ZBufferRenderPolygonMesh(FrameBuffer& frameBuffer, const std::vector<std::optional<ViewportPoint>>& viewportPoints, const SceneData&);
        using InterpolationPoint = glm::vec<3,double>;
        std::vector<InterpolationPoint> GetLineInterpolationPoints(const ViewportPoint& firstPoint, const ViewportPoint& secondPoint);

        void RenderVertices(FrameBuffer& frameBuffer, const std::vector<std::optional<ViewportPoint>>& viewportPoints);
        void RenderRasterizedPolygons(FrameBuffer& frameBuffer, const std::vector<std::optional<ViewportPoint>>& viewportPoints, const SceneData&);
        void ZBufferRenderRasterizedPolygons(FrameBuffer& frameBuffer, const std::vector<std::optional<ViewportPoint>>& viewportPoints, const SceneData&);

        bool AllPolygonVerticesVisible(const std::vector<std::optional<ViewportPoint> > &viewportPoints, const std::vector<int> &vertexIndices);
        ViewportPolygonMargins GetViewportPolygonMargins(const std::vector<std::optional<ViewportPoint> > &viewportPoints, const std::vector<int> &vertexIndices);

        std::vector<PolygonEdge> GetPolygonEdges(const std::vector<std::optional<ViewportPoint> > &viewportPoints, const std::vector<int> &vertexIndices);

        std::vector<IntersectionPoint> GetIntersectionPoints(const std::vector<PolygonEdge>& polygonEdges, float currScanlineY);
        void TryFixThreePointsIntersectionCase(std::vector<IntersectionPoint>&);

        bool PolygonIsBackFacing(const Polygon& polygon, const std::vector<glm::vec3>& vertices);

        void SetAnimationHolder(AnimationHolder animationHolder);
        void SetShadingModelHolder(ShadingModelHolder shadingModelHolder);

        const SceneData& scene_data_;
        float fovy_;
        float near_;
        float far_;
        float model_scale_factor_;

        AnimationHolder animation_holder_;
        ShadingModelHolder shading_model_holder_;
        Camera camera_;

        bool draw_polygon_mesh_;
        std::array<uchar, 4> argb_pen_color_;

        bool rasterize_polygons_;
        std::array<uchar, 4> argb_brush_color_;

        bool draw_world_axes_;

        bool z_buffer_enabled_;
        bool backface_culling_enabled_;

        glm::mat4 curr_model_matrix_;
        glm::mat4 curr_view_matrix_;

        std::vector<std::shared_ptr<LightSource>> light_sources_;
    };

} // namespace pv

#endif // PV_RENDERINGPIPELINE_H
