#ifndef PV_RENDERINGPIPELINE_H
#define PV_RENDERINGPIPELINE_H

#include "animation.h"
#include "scenedata.h"
#include "framebuffer.h"
#include "camera.h"
#include "polygonedge.h"
#include "viewportpolygonmargins.h"
#include "lightsource.h"
#include <vector>
#include <memory>
#include <optional>
#include <glm/mat4x4.hpp>

namespace pv {

    using AnimationHolder = std::unique_ptr<Animation>;
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

        void SetNewLightPosition(float lightPosition);

        void SetEnableLambertianModel(bool enableLambertianModel);

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
        std::vector<InterpolationPoint> GetTriangleInterpolationPoints(const ViewportPoint& firstPoint, const ViewportPoint& secondPoint, const ViewportPoint& thirdPoint);

        bool AllPolygonVerticesVisible(const std::vector<std::optional<ViewportPoint> > &viewportPoints, const std::vector<int> &vertexIndices);
        ViewportPolygonMargins GetViewportPolygonMargins(const std::vector<std::optional<ViewportPoint> > &viewportPoints, const std::vector<int> &vertexIndices);
        ViewportPolygonMargins GetViewportTriangleMargins(const ViewportPoint& firstPoint, const ViewportPoint& secondPoint, const ViewportPoint& thirdPoint);

        std::vector<PolygonEdge> GetPolygonEdges(const std::vector<std::optional<ViewportPoint> > &viewportPoints, const std::vector<int> &vertexIndices);
        std::vector<PolygonEdge> GetTriangleEdges(const ViewportPoint& firstPoint, const ViewportPoint& secondPoint, const ViewportPoint& thirdPoint);

        std::vector<IntersectionPoint> GetIntersectionPoints(const std::vector<PolygonEdge>& polygonEdges, float currScanlineY);

        void TryFixThreePointsIntersectionCase(std::vector<IntersectionPoint>&);

        bool PolygonIsBackFacing(const Polygon& polygon, const std::vector<glm::vec3>& vertices);

        void SetAnimationHolder(AnimationHolder animationHolder);

        const SceneData& scene_data_;
        float fovy_;
        float near_;
        float far_;
        float model_scale_factor_;

        AnimationHolder animation_holder_;
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

        LightSource light_source_;

        bool lambertian_model_enabled_;
    };

} // namespace pv

#endif // PV_RENDERINGPIPELINE_H
