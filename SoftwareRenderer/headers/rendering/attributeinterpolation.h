#ifndef PV_ATTRIBUTEINTERPOLATION_H
#define PV_ATTRIBUTEINTERPOLATION_H


#include <glm/mat4x4.hpp>
#include <vector>
#include <array>
#include "headers/shading/shadedpixel.h"

namespace pv {

    using InterpolationPoint = glm::vec<3, double>;
    using uchar = unsigned char;
    using ShadeColor = std::array<uchar, 4>;


class AttributeInterpolation{
public:
    using InterpolationPoint = glm::vec<3, double>;
    using ViewportPoint = glm::vec4;
    using CameraSpacePoint = glm::vec4;

    void InterpolateDepthOverLine(std::vector<InterpolationPoint>& interpolationPoints,
                                  const ViewportPoint& firstPoint,
                                  const ViewportPoint& secondPoint);

    std::vector<ShadedPixel> GetPixelsWithInterpolatedDepth(
            const std::vector<InterpolationPoint>& interpolationPoints,
            const ViewportPoint& firstPoint,
            const ViewportPoint& secondPoint,
            const ViewportPoint& thirdPoint,

            bool normalInterpolationNeeded = false,
            std::vector<glm::vec3>* normalVectorsPtr = nullptr,
            std::vector<glm::vec3>* interpolatedNormalVectorsPtr = nullptr,

            bool cameraSpaceInterpolationNeeded = false,
            std::vector<glm::vec3>* cameraSpacePosPtr = nullptr,
            std::vector<glm::vec3>* interpolatedCameraSpacePosPtr = nullptr,

            bool textureCoordInterpolationNeeded = false,
            std::vector<glm::vec3>* textureCoordsPtr = nullptr,
            std::vector<glm::vec3>* interpolatedTextureCoordsPtr = nullptr);
};

} // namespace pv

#endif // PV_ATTRIBUTEINTERPOLATION_H
