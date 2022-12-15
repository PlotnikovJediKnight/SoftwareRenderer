#include "headers/rendering/attributeinterpolation.h"
#include <stdexcept>
using namespace std;

namespace pv {

  void AttributeInterpolation::InterpolateDepthOverLine(
      std::vector<InterpolationPoint> & interpolationPoints,
      const ViewportPoint &firstPoint, const ViewportPoint &secondPoint) {
    const double inverseW1 = firstPoint.w;
    const double inverseW2 = secondPoint.w;

    double u = 0.0;
    double uAccretion = 1.0 / static_cast<double>(interpolationPoints.size());
    for (auto& interpolationPoint : interpolationPoints){
        if (u <= 1.0){
            double inverseDepthValue = inverseW1 * (1 - u) + inverseW2 * u;
            interpolationPoint.z = 1.0 / static_cast<double>(inverseDepthValue);
        }
        u += uAccretion;
    }

}

std::vector<ShadedPixel> AttributeInterpolation::GetPixelsWithInterpolatedDepth(
        const std::vector<InterpolationPoint>& interpolationPoints,
        const ViewportPoint &firstPoint,
        const ViewportPoint &secondPoint,
        const ViewportPoint &thirdPoint,

        bool normalInterpolationNeeded,
        std::vector<glm::vec3>* normalVectorsPtr,
        std::vector<glm::vec3>* interpolatedNormalVectorsPtr,

        bool cameraSpaceInterpolationNeeded,
        std::vector<glm::vec3>* cameraSpacePosPtr,
        std::vector<glm::vec3>* interpolatedCameraSpacePosPtr,

        bool textureCoordInterpolationNeeded,
        std::vector<glm::vec3>* textureCoordsPtr,
        std::vector<glm::vec3>* interpolatedTextureCoordsPtr)
{
    vector<ShadedPixel> toReturn; toReturn.reserve(interpolationPoints.size());
    constexpr array<uchar, 4> blackColor{255, 0, 0, 0};

    const double inverseW1 = firstPoint.w;
    const double inverseW2 = secondPoint.w;
    const double inverseW3 = thirdPoint.w;

    const double x0 = firstPoint.x;
    const double y0 = firstPoint.y;

    const double x1 = secondPoint.x;
    const double y1 = secondPoint.y;

    const double x2 = thirdPoint.x;
    const double y2 = thirdPoint.y;

    const double triangleArea = 0.5 * abs(static_cast<double>((x2 - x0) * (y1 - y0) - (y2 - y0) * (x1 - x0)));

    size_t interpPointIdx = 0;
    for (auto& interpolationPoint : interpolationPoints) {
        double interpolatedDepth = std::numeric_limits<double>::max();
        const double qx = interpolationPoint.x;
        const double qy = interpolationPoint.y;

        const double uArea = 0.5 * abs(static_cast<double>(( (x2 - x0) * (qy - y0) - (y2 - y0) * (qx - x0) )));
        const double vArea = 0.5 * abs(static_cast<double>(( (qx - x0) * (y1 - y0) - (qy - y0) * (x1 - x0) )));

        double u = uArea / triangleArea;
        double v = vArea / triangleArea;

        constexpr double RELEVANT_BARYCENTRIC_COORD = 0.0000001;
        if (u < RELEVANT_BARYCENTRIC_COORD ||
            v < RELEVANT_BARYCENTRIC_COORD ||
            1.0 - u - v < RELEVANT_BARYCENTRIC_COORD){

            interpolatedDepth = std::numeric_limits<double>::max();
            toReturn.push_back(
                        ShadedPixel {
                                InterpolationPoint{interpolationPoint.x, interpolationPoint.y, interpolatedDepth},
                                ShadeColor{blackColor}
                        });

            interpPointIdx++;
            continue;
        }

        double inverseDepthValue = inverseW1 * (1 - u - v) + inverseW2 * u + inverseW3 * v;

        if (inverseDepthValue < 0.0){
            interpolatedDepth = std::numeric_limits<double>::max();
            toReturn.push_back(
                        ShadedPixel {
                                InterpolationPoint{interpolationPoint.x, interpolationPoint.y, interpolatedDepth},
                                ShadeColor{blackColor}
                        });
        } else {
            interpolatedDepth = 1.0 / inverseDepthValue;
            toReturn.push_back(
                        ShadedPixel {
                                InterpolationPoint{interpolationPoint.x, interpolationPoint.y, interpolatedDepth},
                                ShadeColor{blackColor}
                        });
        }

        //===========================================================================================
        if (normalInterpolationNeeded) {
            if  ( (normalVectorsPtr->size() != 3) ||
                  (interpolatedNormalVectorsPtr->size() != interpolationPoints.size()) )
            {
                throw std::runtime_error("Normal Interpolation Invariant violated!");
            }

            glm::vec3 normal1 = (*normalVectorsPtr)[0];
            glm::vec3 normal2 = (*normalVectorsPtr)[1];
            glm::vec3 normal3 = (*normalVectorsPtr)[2];

            glm::vec3 interpolatedNormal =
                    normal1 * static_cast<float>(inverseW1) * static_cast<float>(1 - u - v) +
                    normal2 * static_cast<float>(inverseW2) * static_cast<float>(u) +
                    normal3 * static_cast<float>(inverseW3) * static_cast<float>(v);

            interpolatedNormal *= interpolatedDepth;

            (*interpolatedNormalVectorsPtr)[interpPointIdx] = interpolatedNormal;
        }
        //===========================================================================================
        if (cameraSpaceInterpolationNeeded) {
            if  ( (cameraSpacePosPtr->size() != 3) ||
                  (interpolatedCameraSpacePosPtr->size() != interpolationPoints.size()) )
            {
                throw std::runtime_error("Camera Space Interpolation Invariant violated!");
            }

            glm::vec3 camera1 = (*cameraSpacePosPtr)[0];
            glm::vec3 camera2 = (*cameraSpacePosPtr)[1];
            glm::vec3 camera3 = (*cameraSpacePosPtr)[2];

            glm::vec3 interpolatedPointCamera =
                    camera1 * static_cast<float>(inverseW1) * static_cast<float>(1 - u - v) +
                    camera2 * static_cast<float>(inverseW2) * static_cast<float>(u) +
                    camera3 * static_cast<float>(inverseW3) * static_cast<float>(v);

            interpolatedPointCamera *= interpolatedDepth;

            (*interpolatedCameraSpacePosPtr)[interpPointIdx] = interpolatedPointCamera;
        }
        //===========================================================================================
        if (textureCoordInterpolationNeeded) {
            if  ( (textureCoordsPtr->size() != 3) ||
                  (interpolatedTextureCoordsPtr->size() != interpolationPoints.size()) )
            {
                throw std::runtime_error("Texture Coordinates Interpolation Invariant violated!");
            }

            glm::vec3 textureCoord1 = (*textureCoordsPtr)[0]; //cerr << textureCoord1.x << " " << textureCoord1.y << " " << textureCoord1.z << endl;
            glm::vec3 textureCoord2 = (*textureCoordsPtr)[1]; //cerr << textureCoord2.x << " " << textureCoord2.y << " " << textureCoord2.z << endl;
            glm::vec3 textureCoord3 = (*textureCoordsPtr)[2]; //cerr << textureCoord3.x << " " << textureCoord3.y << " " << textureCoord3.z << endl;

            glm::vec3 interpolatedTextureCoord =
                    textureCoord1 * static_cast<float>(inverseW1) * static_cast<float>(1 - u - v) +
                    textureCoord2 * static_cast<float>(inverseW2) * static_cast<float>(u) +
                    textureCoord3 * static_cast<float>(inverseW3) * static_cast<float>(v);

            interpolatedTextureCoord *= interpolatedDepth;

            (*interpolatedTextureCoordsPtr)[interpPointIdx] = interpolatedTextureCoord;
        }
        //===========================================================================================
        interpPointIdx++;
    }

    return toReturn;
}

} // namespace pv
