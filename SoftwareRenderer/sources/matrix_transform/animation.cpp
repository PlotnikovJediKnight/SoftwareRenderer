#include "headers/matrix_transform/animation.h"

namespace pv {

glm::mat4 NoAnimation::GetModelMatrix() {
    return glm::mat4(1.0);
}

glm::mat4 XAnimation::GetModelMatrix() {
    static float rotationAngleInDegrees = 0.0;
    rotationAngleInDegrees += 1.0;

    if (rotationAngleInDegrees > 360) { rotationAngleInDegrees = 0.0; }
    float rotationAngleInRadians = GetRadianAngle(rotationAngleInDegrees);

    float cosineValue = cos(rotationAngleInRadians);
    float sineValue = sin(rotationAngleInRadians);

    return glm::mat4(
                    {1, 0, 0, 0},
                    {0, cosineValue, sineValue, 0},
                    {0,  -sineValue, cosineValue, 0},
                    {0, 0, 0, 1});
}

glm::mat4 YAnimation::GetModelMatrix() {
    static float rotationAngleInDegrees = 0.0;
    rotationAngleInDegrees += 1.0;

    if (rotationAngleInDegrees > 360) { rotationAngleInDegrees = 0.0; }
    float rotationAngleInRadians = GetRadianAngle(rotationAngleInDegrees);

    float cosineValue = cos(rotationAngleInRadians);
    float sineValue = sin(rotationAngleInRadians);

    return glm::mat4(
                    {cosineValue, 0, -sineValue, 0},
                    {0, 1, 0, 0},
                    {sineValue,  0, cosineValue, 0},
                    {0, 0, 0, 1});
}

glm::mat4 ZAnimation::GetModelMatrix() {
    static float rotationAngleInDegrees = 0.0;
    rotationAngleInDegrees += 1.0;

    if (rotationAngleInDegrees > 360) { rotationAngleInDegrees = 0.0; }
    float rotationAngleInRadians = GetRadianAngle(rotationAngleInDegrees);

    float cosineValue = cos(rotationAngleInRadians);
    float sineValue = sin(rotationAngleInRadians);

    return glm::mat4(
                    {cosineValue, sineValue, 0, 0},
                    {-sineValue, cosineValue, 0, 0},
                    {0, 0, 1, 0},
                    {0, 0, 0, 1});
}

glm::mat4 CarouselAnimation::GetModelMatrix() {
    static float zRotationAngleInDegrees = 0.0;
    static float translationVectorAngle = 0.0;

    zRotationAngleInDegrees += 0.5;
    translationVectorAngle += 0.15;

    if (zRotationAngleInDegrees > 360) { zRotationAngleInDegrees = 0.0; }
    if (translationVectorAngle > 360)  {  translationVectorAngle = 0.0; }

    float zRotationAngleInRadians = GetRadianAngle(zRotationAngleInDegrees);
    float translationVectorAngleInRadians = GetRadianAngle(translationVectorAngle);

    float ZCos = cos(zRotationAngleInRadians);
    float ZSin = sin(zRotationAngleInRadians);

    float TrCos = cos(translationVectorAngleInRadians);
    float TrSin = sin(translationVectorAngleInRadians);

    return glm::mat4(
                    {ZCos, ZSin, 0, 0},
                    {-ZSin, ZCos, 0, 0},
                    {0, 0, 1, 0},
                    {TrCos, TrSin, 0, 1});
}

float Animation::GetRadianAngle(float degreeAngle) const {
    return degreeAngle / 180.0 * M_PI;
}


} // namespace pv
