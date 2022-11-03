#include "attributeinterpolation.h"

namespace pv {

void AttributeInterpolation::InterpolateDepthOverLine(
        std::vector<InterpolationPoint> &interpolationPoints,
        const ViewportPoint &firstPoint,
        const ViewportPoint &secondPoint)
{
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

void AttributeInterpolation::InterpolateDepthOverTriangle(
        std::vector<InterpolationPoint> &interpolationPoints,
        const ViewportPoint &firstPoint,
        const ViewportPoint &secondPoint,
        const ViewportPoint &thirdPoint)
{
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

    for (auto& interpolationPoint : interpolationPoints){
        const double qx = interpolationPoint.x;
        const double qy = interpolationPoint.y;

        const double uArea = 0.5 * abs(static_cast<double>(( (x2 - x0) * (qy - y0) - (y2 - y0) * (qx - x0) )));
        const double vArea = 0.5 * abs(static_cast<double>(( (qx - x0) * (y1 - y0) - (qy - y0) * (x1 - x0) )));

        double u = uArea / triangleArea;
        double v = vArea / triangleArea;

        double inverseDepthValue = inverseW1 * (1 - u - v) + inverseW2 * u + inverseW3 * v;
        interpolationPoint.z = 1.0 / inverseDepthValue;
    }
}

} // namespace pv
