#include "headers/shading/shadingmodel.h"
#include "headers/rendering/attributeinterpolation.h"
#include <algorithm>
using namespace std;

namespace pv {

    ShadingModel::ShadingModel() :
        normal_interpolation_needed_(false),
        normal_vectors_ptr_(nullptr),
        interpolated_normal_vectors_ptr_(nullptr),

        camera_space_interpolation_needed_(false),
        camera_space_pos_ptr_(nullptr),
        interpolated_camera_space_pos_ptr_(nullptr),

        texture_coord_interpolation_needed_(false),
        texture_coords_ptr_(nullptr),
        interpolated_texture_coords_ptr_(nullptr),

        diffuse_texturing_enabled_(false),
        normal_texturing_enabled_(false),
        specular_texturing_enabled_(false) { }

    void ShadingModel::SetDiffuseTexturingEnabled(bool diffuseEnabled) {
        diffuse_texturing_enabled_ = diffuseEnabled;
    }

    void ShadingModel::SetNormalTexturingEnabled(bool normalEnabled) {
        normal_texturing_enabled_ = normalEnabled;
    }

    void ShadingModel::SetSpecularTexturingEnabled(bool specularEnabled) {
        specular_texturing_enabled_ = specularEnabled;
    }

    void ShadingModel::SetDiffuseTexture(TextureHolder diffuseTexture) {
        diffuse_texture_ = std::move(diffuseTexture);
    }

    void ShadingModel::SetNormalTexture(TextureHolder normalTexture) {
        normal_texture_ = std::move(normalTexture);
    }

    void ShadingModel::SetSpecularTexture(TextureHolder specularTexture) {
        specular_texture_ = std::move(specularTexture);
    }

    std::vector<ShadedPixel>
    ShadingModel::GetShadedPixels
    (
            const ViewportPoint& firstPoint, const ViewportPoint& secondPoint, const ViewportPoint& thirdPoint,
            const Polygon& polygon,
            const SceneData& sceneData,
            std::array<uchar, 4> materialColor,
            const std::vector<std::shared_ptr<LightSource>>& lightSources,
            const glm::mat4& model, const glm::mat4& view
     ) const {

        AttributeInterpolation attrInterpolation;
        auto interpolationPoints = GetTriangleInterpolationPoints(firstPoint, secondPoint, thirdPoint);

        if (normal_interpolation_needed_){
            interpolated_normal_vectors_ptr_->resize(interpolationPoints.size());
        }

        if (camera_space_interpolation_needed_){
            interpolated_camera_space_pos_ptr_->resize(interpolationPoints.size());
        }

        if (texture_coord_interpolation_needed_){
            interpolated_texture_coords_ptr_->resize(interpolationPoints.size());
        }

        auto shadedPixels = attrInterpolation.GetPixelsWithInterpolatedDepth(interpolationPoints,
                                                       firstPoint,
                                                       secondPoint,
                                                       thirdPoint,

                                                       normal_interpolation_needed_,
                                                       normal_vectors_ptr_,
                                                       interpolated_normal_vectors_ptr_,

                                                       camera_space_interpolation_needed_,
                                                       camera_space_pos_ptr_,
                                                       interpolated_camera_space_pos_ptr_,

                                                       texture_coord_interpolation_needed_,
                                                       texture_coords_ptr_,
                                                       interpolated_texture_coords_ptr_);
        return shadedPixels;
    }


    std::vector<InterpolationPoint>
    ShadingModel::GetTriangleInterpolationPoints(
            const ViewportPoint &firstPoint,
            const ViewportPoint &secondPoint,
            const ViewportPoint &thirdPoint) const
    {
        ViewportPolygonMargins viewportTriangleMargins = GetViewportTriangleMargins(firstPoint, secondPoint, thirdPoint);
        float currScanlineY = viewportTriangleMargins.minScanlineY;
        const float scanlineIncrement = 1.0;

        auto triangleEdges = GetTriangleEdges(firstPoint, secondPoint, thirdPoint);


        vector<InterpolationPoint> interpolationPoints;

        while (currScanlineY < viewportTriangleMargins.maxScanlineY){
            auto intersectionPoints = GetIntersectionPoints(triangleEdges, currScanlineY);

            sort(intersectionPoints.begin(),
                 intersectionPoints.end(),
                 [](const IntersectionPoint& lhs, const IntersectionPoint& rhs) { return lhs.x < rhs.x; });

            if (intersectionPoints.size() == 2){
                const auto& firstPoint = intersectionPoints[0];
                const auto& secondPoint = intersectionPoints[1];

                size_t minX = static_cast<size_t>(firstPoint.x);
                size_t maxX = static_cast<size_t>(secondPoint.x);

                for (size_t currX = minX; currX <= maxX; ++currX){
                    interpolationPoints.push_back({currX, currScanlineY, -1.0});
                }
            }

            currScanlineY += scanlineIncrement;
            currScanlineY = floor(currScanlineY);
        }

        return interpolationPoints;
    }

    ViewportPolygonMargins
    ShadingModel::GetViewportTriangleMargins(
            const ViewportPoint &firstPoint,
            const ViewportPoint &secondPoint,
            const ViewportPoint &thirdPoint) const
    {
        ViewportPolygonMargins
                viewportTriangleMargins{
                    min(firstPoint.y, min(secondPoint.y, thirdPoint.y)),
                    max(firstPoint.y, max(secondPoint.y, thirdPoint.y))};

        return viewportTriangleMargins;
    }

    std::vector<PolygonEdge>
    ShadingModel::GetTriangleEdges(
            const ViewportPoint &firstPoint,
            const ViewportPoint &secondPoint,
            const ViewportPoint &thirdPoint) const
    {
        vector<PolygonEdge> polygonEdges;
        polygonEdges.reserve(3);

        polygonEdges.push_back(PolygonEdge{firstPoint.x, firstPoint.y,
                                           secondPoint.x, secondPoint.y});

        polygonEdges.push_back(PolygonEdge{secondPoint.x, secondPoint.y,
                                           thirdPoint.x, thirdPoint.y});

        polygonEdges.push_back(PolygonEdge{thirdPoint.x, thirdPoint.y,
                                           firstPoint.x, firstPoint.y});

        return polygonEdges;
    }

    std::vector<IntersectionPoint>
    ShadingModel::GetIntersectionPoints(
            const std::vector<PolygonEdge> &polygonEdges,
            float currScanlineY) const
    {
        vector<IntersectionPoint> intersectionPoints;

        for (const auto& polygonEdge : polygonEdges){
            auto intersectionPoint = polygonEdge.GetIntersectionWithScanline(currScanlineY);
            if (intersectionPoint) {
                intersectionPoints.push_back(intersectionPoint.value());
            }
        }

        if (intersectionPoints.size() == 3){
            TryFixThreePointsIntersectionCase(intersectionPoints);
        }

        return intersectionPoints;
    }

    void
    ShadingModel::TryFixThreePointsIntersectionCase(
            std::vector<IntersectionPoint> &intersectionPoints) const
    {
        sort(intersectionPoints.begin(),
             intersectionPoints.end(),
             [](const IntersectionPoint& lhs,const IntersectionPoint& rhs){return lhs.x < rhs.x;});

        constexpr float eps = 0.1;
        if (abs(intersectionPoints[0].x - intersectionPoints[1].x) < eps &&
            abs(intersectionPoints[0].y - intersectionPoints[1].y) < eps){
            intersectionPoints.erase(intersectionPoints.begin());
        }
            else

        if (abs(intersectionPoints[1].x - intersectionPoints[2].x) < eps &&
            abs(intersectionPoints[1].y - intersectionPoints[2].y) < eps){
            intersectionPoints.erase(intersectionPoints.begin() + 1);
        }
    }


} // namespace pv
