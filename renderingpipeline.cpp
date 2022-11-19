#include "renderingpipeline.h"
#include <glm/vec3.hpp>
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>
#include "attributeinterpolation.h"
#include "lambertianmodel.h"

using namespace std;

namespace pv {

RenderingPipeline::RenderingPipeline(const SceneData& sceneData) :
    scene_data_(sceneData),
    fovy_{GetRadianAngle(30.0)},
    near_{2.0},
    far_ {500.0},
    model_scale_factor_{1.0},
    animation_holder_{std::make_unique<NoAnimation>()},
    shading_model_holder_{std::make_unique<NoShading>()},
    draw_polygon_mesh_(false),
    argb_pen_color_({255, 255, 255, 0}),
    rasterize_polygons_(false),
    argb_brush_color_({255, 127, 127, 127}),
    draw_world_axes_(false),
    z_buffer_enabled_(false),
    backface_culling_enabled_(false),
    light_sources_() { }

glm::mat4 RenderingPipeline::GetFrustumProjection(float aspectRatio) {
    float fovyAngleInRadians = GetRadianAngle(fovy_);

    float g = 1.0F / tan(fovyAngleInRadians * 0.5F);
    float k = far_ / (far_ - near_);

    return glm::mat4(
                {g / aspectRatio, 0.0F, 0.0F, 0.0F},
                {0.0F, g, 0.0F, 0.0F},
                {0.0F, 0.0F, k, 1},
                {0.0F, 0.0F, -k * near_, 0.0F});
}

glm::mat4 RenderingPipeline::GetViewportTransform(size_t width, size_t height) {
    return glm::mat4(
                {width / 2.0, 0, 0, 0},
                {0, height / 2.0, 0, 0},
                {0, 0, 1, 0},
                {width / 2.0, height / 2.0, 0, 1});
}

float RenderingPipeline::GetRadianAngle(float degreeAngle) {
    return degreeAngle * M_PI / 180.0;
}

void RenderingPipeline::RenderWorldAxes(FrameBuffer& frameBuffer) {
    size_t width = frameBuffer.GetWidth();
    size_t height = frameBuffer.GetHeight();

    constexpr size_t ORIGIN_INDEX = 3;
    constexpr glm::vec3 xAxis{1.0, 0.0, 0.0};
    constexpr glm::vec3 yAxis{0.0, 1.0, 0.0};
    constexpr glm::vec3 zAxis{0.0, 0.0, 1.0};
    constexpr glm::vec3 origin{0.0, 0.0, 0.0};

    using argbArray = array<uchar, 4>;
    constexpr argbArray blueColor {255, 0, 0, 255};
    constexpr argbArray greenColor{255, 0, 255, 0};
    constexpr argbArray redColor  {255, 255, 0, 0};

    constexpr array<argbArray, 3> axisColors { redColor, greenColor, blueColor };

    float oldModelScale = model_scale_factor_;
    this->SetModelScaleFactor(0.65);

    AnimationHolder oldAnimationHolder = std::move(animation_holder_);
    this->SetAnimationType(ANIMATION_TYPE::NO_ANIMATION);

    {
        auto viewportPoints = GetViewPortPoints({xAxis, yAxis, zAxis, origin},
                                                static_cast<float>(width) / height,
                                                width,
                                                height);

        auto& originViewportPoint = viewportPoints[ORIGIN_INDEX];

        if (originViewportPoint){
            for (size_t i = 0; i < viewportPoints.size() - 1; ++i){
                auto& axisViewportPoint = viewportPoints[i];

                if (axisViewportPoint) {
                    frameBuffer.DrawLine(originViewportPoint.value().x,
                                         originViewportPoint.value().y,
                                         axisViewportPoint.value().x,
                                         axisViewportPoint.value().y,
                                         axisColors[i]);
                }
            }
        }
    }

    this->SetModelScaleFactor(oldModelScale);
    this->SetAnimationHolder(std::move(oldAnimationHolder));
}

void RenderingPipeline::RenderPolygonMesh(
        FrameBuffer &frameBuffer,
        const std::vector<std::optional<ViewportPoint>>& viewportPoints,
        const SceneData& sceneData)
{
    const std::vector<Polygon> &polygons = sceneData.polygons;

    for (const auto& polygon : polygons){
        const auto& vertexIndices = polygon.vertex_indices;

        if (backface_culling_enabled_){
            if (PolygonIsBackFacing(polygon, sceneData.vertices)) {
                continue;
            }
        }

        for (size_t idx = 0; idx < vertexIndices.size() - 1; ++idx){
            const auto& pointOne = viewportPoints[ vertexIndices[idx] ];
            const auto& pointTwo = viewportPoints[ vertexIndices[idx + 1] ];
            if (pointOne && pointTwo){
                frameBuffer.DrawLine(pointOne.value().x,
                                     pointOne.value().y,
                                     pointTwo.value().x,
                                     pointTwo.value().y,
                                     argb_pen_color_
                 );
            }
        }

        size_t lastVertexInPolygonIndex = vertexIndices.size() - 1;
        const auto& pointOne = viewportPoints[ vertexIndices[0] ];
        const auto& pointTwo = viewportPoints[ vertexIndices[lastVertexInPolygonIndex] ];
        if (pointOne && pointTwo){
            frameBuffer.DrawLine(pointOne.value().x,
                                 pointOne.value().y,
                                 pointTwo.value().x,
                                 pointTwo.value().y,
                                 argb_pen_color_
             );
        }
    }
}

void RenderingPipeline::ZBufferRenderPolygonMesh(
        FrameBuffer &frameBuffer,
        const std::vector<std::optional<ViewportPoint> > &viewportPoints,
        const SceneData& sceneData)
{
    const std::vector<Polygon> &polygons = sceneData.polygons;
    constexpr double POLYGON_MESH_VISIBILITY_Z_OFFSET = 0.01;
    AttributeInterpolation attrInterpolation;

    for (const auto& polygon : polygons){
        const auto& vertexIndices = polygon.vertex_indices;

        if (backface_culling_enabled_){
            if (PolygonIsBackFacing(polygon, sceneData.vertices)) {
                continue;
            }
        }

        for (size_t idx = 0; idx < vertexIndices.size() - 1; ++idx){
            const auto& pointOne = viewportPoints[ vertexIndices[idx] ];
            const auto& pointTwo = viewportPoints[ vertexIndices[idx + 1] ];
            if (pointOne && pointTwo){
                auto interpolationPoints = GetLineInterpolationPoints(pointOne.value(), pointTwo.value());
                attrInterpolation.InterpolateDepthOverLine(interpolationPoints, pointOne.value(), pointTwo.value());
                for (const auto& interpolationPoint : interpolationPoints){
                    frameBuffer.ZBufferDrawPixel(interpolationPoint.x,
                                                 interpolationPoint.y,
                                                 interpolationPoint.z - POLYGON_MESH_VISIBILITY_Z_OFFSET,
                                                 argb_pen_color_);
                }
            }
        }

        size_t lastVertexInPolygonIndex = vertexIndices.size() - 1;
        const auto& pointOne = viewportPoints[ vertexIndices[0] ];
        const auto& pointTwo = viewportPoints[ vertexIndices[lastVertexInPolygonIndex] ];
        if (pointOne && pointTwo){
            auto interpolationPoints = GetLineInterpolationPoints(pointOne.value(), pointTwo.value());
            attrInterpolation.InterpolateDepthOverLine(interpolationPoints, pointOne.value(), pointTwo.value());

            for (const auto& interpolationPoint : interpolationPoints){
                frameBuffer.ZBufferDrawPixel(interpolationPoint.x,
                                             interpolationPoint.y,
                                             interpolationPoint.z - POLYGON_MESH_VISIBILITY_Z_OFFSET,
                                             argb_pen_color_);
            }
        }
    }
}

std::vector<RenderingPipeline::InterpolationPoint>
RenderingPipeline::GetLineInterpolationPoints(
        const ViewportPoint &firstPoint,
        const ViewportPoint &secondPoint)
{
    size_t integerX1 = firstPoint.x, integerY1 = firstPoint.y, integerX2 = secondPoint.x, integerY2 = secondPoint.y;
    int deltaX = integerX2 - integerX1;
    int deltaY = integerY2 - integerY1;

    size_t rasterizationSteps = abs(deltaX) > abs(deltaY) ? abs(deltaX) : abs(deltaY);
    rasterizationSteps++;

    float x = firstPoint.x;
    float y = firstPoint.y;

    float xAccretion = deltaX / static_cast<float>(rasterizationSteps);
    float yAccretion = deltaY / static_cast<float>(rasterizationSteps);

    vector<InterpolationPoint> interpolationPoints;
    interpolationPoints.reserve(rasterizationSteps);
    for (size_t step = 1; step <= rasterizationSteps; ++step){
        x += xAccretion;
        y += yAccretion;
        interpolationPoints.push_back({round(x), round(y), -1.0});
    }

    return interpolationPoints;
}

void RenderingPipeline::RenderVertices(FrameBuffer &frameBuffer, const std::vector<std::optional<ViewportPoint>>& viewportPoints) {
    for (const auto& viewportPoint : viewportPoints){
        if (viewportPoint){
            const auto& viewportPointValue = viewportPoint.value();
            frameBuffer.DrawPixel(viewportPointValue.x, viewportPointValue.y, argb_pen_color_);
        }
    }
}

void RenderingPipeline::RenderRasterizedPolygons(
        FrameBuffer &frameBuffer,
        const std::vector<std::optional<ViewportPoint> > &viewportPoints,
        const SceneData& sceneData)
{
    const std::vector<Polygon> &polygons = sceneData.polygons;

    for (const auto& polygon : polygons){
        const auto& vertexIndices = polygon.vertex_indices;

        if (backface_culling_enabled_){
            if (PolygonIsBackFacing(polygon, sceneData.vertices)) {
                continue;
            }
        }

        if (AllPolygonVerticesVisible(viewportPoints, vertexIndices)){
            ViewportPolygonMargins viewportPolygonMargins = GetViewportPolygonMargins(viewportPoints, vertexIndices);
            float currScanlineY = viewportPolygonMargins.minScanlineY;
            const float scanlineIncrement = 1.0;

            auto polygonEdges = GetPolygonEdges(viewportPoints, vertexIndices);

            while (currScanlineY < viewportPolygonMargins.maxScanlineY){
                auto intersectionPoints = GetIntersectionPoints(polygonEdges, currScanlineY);

                sort(intersectionPoints.begin(),
                     intersectionPoints.end(),
                     [](const IntersectionPoint& lhs, const IntersectionPoint& rhs) { return lhs.x < rhs.x; });

                if (intersectionPoints.size() % 2 == 0){
                    size_t pairsCount = intersectionPoints.size() / 2;
                    size_t pairIdx = 0;

                    while (pairsCount != 0){
                        const auto& firstPoint = intersectionPoints[pairIdx];
                        const auto& secondPoint = intersectionPoints[pairIdx + 1];
                        frameBuffer.DrawLine(firstPoint.x, firstPoint.y,
                                             secondPoint.x, secondPoint.y,
                                             argb_brush_color_);

                        --pairsCount;
                        pairIdx += 2;
                    }
                }

                currScanlineY += scanlineIncrement;
                currScanlineY = floor(currScanlineY);
            }
        }
    }
}

void RenderingPipeline::ZBufferRenderRasterizedPolygons(
        FrameBuffer &frameBuffer,
        const std::vector<std::optional<ViewportPoint> > &viewportPoints,
        const SceneData& sceneData)
{
    const std::vector<Polygon> &polygons = sceneData.polygons;

    for (const auto& polygon : polygons){
        const auto& vertexIndices = polygon.vertex_indices;

        if (backface_culling_enabled_){
            if (PolygonIsBackFacing(polygon, sceneData.vertices)) {
                continue;
            }
        }

        if (AllPolygonVerticesVisible(viewportPoints, vertexIndices)){
            const ViewportPoint& firstPoint = viewportPoints[vertexIndices[0]].value();
            const ViewportPoint& secondPoint = viewportPoints[vertexIndices[1]].value();
            const ViewportPoint& thirdPoint = viewportPoints[vertexIndices[2]].value();

            auto materialColor = argb_brush_color_;
            auto shadedPixels
                    = shading_model_holder_->GetShadedPixels(firstPoint,
                                                             secondPoint,
                                                             thirdPoint,
                                                             polygon,
                                                             sceneData,
                                                             materialColor,
                                                             light_sources_,
                                                             curr_model_matrix_,
                                                             curr_view_matrix_);
            for (const auto& shadedPixel : shadedPixels){
                frameBuffer.ZBufferDrawPixel(shadedPixel.interpolatedPoint.x,
                                             shadedPixel.interpolatedPoint.y,
                                             shadedPixel.interpolatedPoint.z,
                                             shadedPixel.shadeColor);
            }
        }
    }
}

/*
std::vector<RenderingPipeline::InterpolationPoint>
RenderingPipeline::GetTriangleInterpolationPoints(
        const ViewportPoint &firstPoint,
        const ViewportPoint &secondPoint,
        const ViewportPoint &thirdPoint)
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
}*/

bool RenderingPipeline::AllPolygonVerticesVisible(
        const std::vector<std::optional<ViewportPoint> > &viewportPoints,
        const std::vector<int> &vertexIndices)
{
    for (size_t idx = 0; idx < vertexIndices.size(); ++idx){
        const auto& point = viewportPoints[ vertexIndices[idx] ];
        if (!point) return false;
    }

    return true;
}

ViewportPolygonMargins
RenderingPipeline::GetViewportPolygonMargins(
        const std::vector<std::optional<ViewportPoint> > &viewportPoints,
        const std::vector<int> &vertexIndices)
{
    float yStartCompValue = viewportPoints[ vertexIndices[0] ]->y;
    ViewportPolygonMargins viewportPolygonMargins{yStartCompValue, yStartCompValue};

    for (size_t idx = 1; idx < vertexIndices.size(); ++idx){
        const auto& point = viewportPoints[ vertexIndices[idx] ];

        if (point->y < viewportPolygonMargins.minScanlineY){
            viewportPolygonMargins.minScanlineY = point->y;
        }

        if (point->y > viewportPolygonMargins.maxScanlineY){
            viewportPolygonMargins.maxScanlineY = point->y;
        }
    }

    return viewportPolygonMargins;
}

/*
ViewportPolygonMargins
RenderingPipeline::GetViewportTriangleMargins(
        const ViewportPoint &firstPoint,
        const ViewportPoint &secondPoint,
        const ViewportPoint &thirdPoint)
{
    ViewportPolygonMargins
            viewportTriangleMargins{
                min(firstPoint.y, min(secondPoint.y, thirdPoint.y)),
                max(firstPoint.y, max(secondPoint.y, thirdPoint.y))};

    return viewportTriangleMargins;
}*/

std::vector<PolygonEdge> RenderingPipeline::GetPolygonEdges(
        const std::vector<std::optional<ViewportPoint> > &viewportPoints,
        const std::vector<int> &vertexIndices)
{
    vector<PolygonEdge> polygonEdges;
    polygonEdges.reserve(vertexIndices.size());

    for (size_t idx = 0; idx < vertexIndices.size() - 1; ++idx){
        const auto& pointOne = viewportPoints[ vertexIndices[idx] ];
        const auto& pointTwo = viewportPoints[ vertexIndices[idx + 1] ];

        polygonEdges.push_back(PolygonEdge{pointOne.value().x, pointOne.value().y,
                                           pointTwo.value().x, pointTwo.value().y});
    }

    size_t lastVertexInPolygonIndex = vertexIndices.size() - 1;
    const auto& pointOne = viewportPoints[ vertexIndices[0] ];
    const auto& pointTwo = viewportPoints[ vertexIndices[lastVertexInPolygonIndex] ];

    polygonEdges.push_back(PolygonEdge{pointOne.value().x, pointOne.value().y,
                                       pointTwo.value().x, pointTwo.value().y});

    return polygonEdges;
}

/*
std::vector<PolygonEdge>
RenderingPipeline::GetTriangleEdges(
        const ViewportPoint &firstPoint,
        const ViewportPoint &secondPoint,
        const ViewportPoint &thirdPoint)
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
}*/


std::vector<IntersectionPoint> RenderingPipeline::GetIntersectionPoints(
        const std::vector<PolygonEdge>& polygonEdges,
        float currScanlineY)
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


void RenderingPipeline::TryFixThreePointsIntersectionCase(std::vector<IntersectionPoint> &intersectionPoints) {
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

bool RenderingPipeline::PolygonIsBackFacing(
        const Polygon &polygon,
        const std::vector<glm::vec3> &vertices)
{

    glm::vec4 point0(vertices[polygon.vertex_indices[0]], 1);
    glm::vec4 point1(vertices[polygon.vertex_indices[1]], 1);
    glm::vec4 point2(vertices[polygon.vertex_indices[2]], 1);

    glm::vec4 point0World = curr_model_matrix_ * point0;
    glm::vec4 point1World = curr_model_matrix_ * point1;
    glm::vec4 point2World = curr_model_matrix_ * point2;

    glm::vec4 vecAWorld = point0World - point1World;
    glm::vec4 vecBWorld = point2World - point1World;

    glm::vec3 polygonNormalWorld = glm::cross(glm::vec3{vecAWorld.x, vecAWorld.y, vecAWorld.z},
                                              glm::vec3{vecBWorld.x, vecBWorld.y, vecBWorld.z});

    constexpr double DOT_PRODUCT_EPS = 0.0001;
    if (glm::dot(polygonNormalWorld, camera_.GetWorldViewDirection()) < DOT_PRODUCT_EPS){
        return true;
    }

    return false;
}

void RenderingPipeline::SetAnimationHolder(AnimationHolder animationHolder) {
    animation_holder_ = std::move(animationHolder);
}

void RenderingPipeline::SetShadingModelHolder(ShadingModelHolder shadingModelHolder) {
    shading_model_holder_ = std::move(shadingModelHolder);
}

void RenderingPipeline::SetLightSources(vector<shared_ptr<LightSource>> lightSources) {
    light_sources_ = lightSources;
}


std::vector<std::optional<ViewportPoint>>
RenderingPipeline::GetViewPortPoints
(
        const std::vector<glm::vec3>& points,
        float aspectRatio,
        size_t width,
        size_t height
) {
    std::vector<std::optional<ViewportPoint>> viewportPoints;
    viewportPoints.reserve(points.size());
    {
        glm::mat4 Model = animation_holder_->GetModelMatrix();
        ApplyScaleFactor(Model);
        curr_model_matrix_ = Model;
        glm::mat4 Camera = camera_.GetCameraMatrix();
        glm::mat4 View = glm::inverse(Camera);
        curr_view_matrix_ = View;

        glm::mat4 ViewportTransform = GetViewportTransform(width, height);
        glm::mat4 Projection = GetFrustumProjection(aspectRatio);

        auto MVP = Projection * View * Model;

        for (const auto& objectPoint : points){
            glm::vec4 homoPoint (objectPoint, 1);
            auto clipSpacePoint = MVP * homoPoint;

            if (WCoordinateIsNonZero(clipSpacePoint.w)){
                float inverseW = 1.0 / clipSpacePoint.w;
                auto deviceSpacePoint = clipSpacePoint * inverseW;

                if (PointIsWithinCanonicalViewVolume(deviceSpacePoint)){
                    auto viewportPoint = ViewportTransform * deviceSpacePoint;

                    if (PointIsWithinViewportBoundaries(width, height, viewportPoint)) {
                        viewportPoints.push_back(std::optional<ViewportPoint>{
                                                    {viewportPoint.x,
                                                     viewportPoint.y,
                                                     viewportPoint.z,
                                                     inverseW}
                                                 });
                        continue;
                    }
                }
            }
            viewportPoints.push_back(std::nullopt);
        }
    }
    return viewportPoints;
}

void RenderingPipeline::DoRender(size_t width, size_t height, uchar *renderedImage) {
    FrameBuffer frameBuffer(width, height, COLOR_MODEL::ARGB32);
    frameBuffer.Clear(0x00);
    if (z_buffer_enabled_) { frameBuffer.EnableZBuffer(); frameBuffer.ClearZBuffer(); }

    if (!scene_data_.vertices.empty()){
        auto viewportPoints = GetViewPortPoints(
                                    scene_data_.vertices,
                                    static_cast<float>(width) / height,
                                    width, height);

        if (!draw_polygon_mesh_ && !rasterize_polygons_){
            RenderVertices(frameBuffer, viewportPoints);
        }
            else

        if (z_buffer_enabled_){

            if (draw_polygon_mesh_) {
                ZBufferRenderPolygonMesh(frameBuffer, viewportPoints, scene_data_);
            }

            if (rasterize_polygons_){
                ZBufferRenderRasterizedPolygons(frameBuffer, viewportPoints, scene_data_);
            }

        } else {

            if (rasterize_polygons_){
                RenderRasterizedPolygons(frameBuffer, viewportPoints, scene_data_);
            }

            if (draw_polygon_mesh_) {
                RenderPolygonMesh(frameBuffer, viewportPoints, scene_data_);
            }
        }
    }

    if (draw_world_axes_) {
        RenderWorldAxes(frameBuffer);
    }

    frameBuffer.CopyToUcharArray(renderedImage);
}

void RenderingPipeline::SetNearPlaneDistance(float near) {
    near_ = near;
}

void RenderingPipeline::SetFarPlaneDistance(float far) {
    far_ = far;
}

void RenderingPipeline::SetFOVYDegreeAngle(float fovyDegrees) {
    fovy_ = GetRadianAngle(fovyDegrees);
}

void RenderingPipeline::SetModelScaleFactor(float scaleFactor) {
    model_scale_factor_ = scaleFactor;
}

void RenderingPipeline::SetAnimationType(ANIMATION_TYPE animationType) {
    switch (animationType){
        case ANIMATION_TYPE::NO_ANIMATION:
        animation_holder_ = make_unique<NoAnimation>();
            break;
        case ANIMATION_TYPE::X_ROTATION:
        animation_holder_ = make_unique<XAnimation>();
            break;
        case ANIMATION_TYPE::Y_ROTATION:
        animation_holder_ = make_unique<YAnimation>();
            break;
        case ANIMATION_TYPE::Z_ROTATION:
        animation_holder_ = make_unique<ZAnimation>();
            break;
        case ANIMATION_TYPE::CAROUSEL:
        animation_holder_ = make_unique<CarouselAnimation>();
            break;
        default:
        animation_holder_ = make_unique<NoAnimation>();
    }
}

void RenderingPipeline::SetShadingModelType(SHADING_MODEL shadingType) {
    switch (shadingType){
        case SHADING_MODEL::NO_SHADING:
        shading_model_holder_ = make_unique<NoShading>();
            break;
        case SHADING_MODEL::LAMBERTIAN_SHADING:
        shading_model_holder_ = make_unique<LambertianShading>();
            break;
        case SHADING_MODEL::PHONG_SHADING:
        shading_model_holder_ = make_unique<PhongShading>();
            break;
        default:
        shading_model_holder_ = make_unique<NoShading>();
    }
}

void RenderingPipeline::SetDrawWorldAxis(bool drawWorldAxis) {
    draw_world_axes_ = drawWorldAxis;
}

void RenderingPipeline::SetDrawPolygonMesh(bool drawPolygonMesh) {
    draw_polygon_mesh_ = drawPolygonMesh;
}

void RenderingPipeline::SetRasterizePolygons(bool rasterizePolygons) {
    rasterize_polygons_ = rasterizePolygons;
}

void RenderingPipeline::SetXCameraView() {
    camera_.SetViewFromX();
}

void RenderingPipeline::SetYCameraView() {
    camera_.SetViewFromY();
}

void RenderingPipeline::SetZCameraView() {
    camera_.SetViewFromZ();
}

void RenderingPipeline::UpdateCameraPosition(int deltaX, int deltaY) {
    if (deltaX != 0) deltaX /= abs(deltaX);
    if (deltaY != 0) deltaY /= abs(deltaY);

    constexpr float azimuthDegrees = 2.0, inclinationDegrees = 2.0;
    camera_.UpdateCameraPosition(-deltaX * azimuthDegrees,
                                 -deltaY * inclinationDegrees);
}

void RenderingPipeline::SetOrbitCameraDistance(float distance) {
    camera_.SetWorldOriginDistanceR(distance);
}

void RenderingPipeline::SetNewPenColor(const std::array<uchar, 4> &argbPenColor) {
    argb_pen_color_ = argbPenColor;
}

void RenderingPipeline::SetNewBrushColor(const std::array<uchar, 4> &argbBrushColor) {
    argb_brush_color_ = argbBrushColor;
}

void RenderingPipeline::SetEnableZBuffering(bool enableZBuffering) {
    z_buffer_enabled_ = enableZBuffering;
}

void RenderingPipeline::SetEnableBackfaceCulling(bool enableBackfaceCulling) {
    backface_culling_enabled_ = enableBackfaceCulling;
}

void RenderingPipeline::ApplyScaleFactor(glm::mat4 &modelMatrix) {
    for (int row = 0; row < 3; ++row){
        for (int column = 0; column < 3; ++column){
            modelMatrix[row][column] *= model_scale_factor_;
        }
    }
}

bool RenderingPipeline::WCoordinateIsNonZero(float w) {
    constexpr float EPSILON = 0.0001;
    return abs(w) > EPSILON;
}

bool RenderingPipeline::PointIsWithinCanonicalViewVolume(const glm::vec4 &point) {
    return (-1 <= point.x) && (point.x <= 1) &&
           (-1 <= point.y) && (point.y <= 1) &&
           ( 0 <= point.z) && (point.z <= 1);
}

bool RenderingPipeline::PointIsWithinViewportBoundaries(size_t width, size_t height, const glm::vec3 &point) {
    return point.x < width && point.y < height;
}

} // namespace pv
