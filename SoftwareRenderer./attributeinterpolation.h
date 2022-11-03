#ifndef PV_ATTRIBUTEINTERPOLATION_H
#define PV_ATTRIBUTEINTERPOLATION_H


#include <glm/mat4x4.hpp>
#include <vector>

namespace pv {

class AttributeInterpolation{
public:
    using InterpolationPoint = glm::vec<3, double>;
    using ViewportPoint = glm::vec4;

    void InterpolateDepthOverLine(std::vector<InterpolationPoint>& interpolationPoints,
                                  const ViewportPoint& firstPoint,
                                  const ViewportPoint& secondPoint);

    void InterpolateDepthOverTriangle(std::vector<InterpolationPoint>& interpolationPoints,
                                  const ViewportPoint& firstPoint,
                                  const ViewportPoint& secondPoint,
                                  const ViewportPoint& thirdPoint);
};

} // namespace pv

#endif // PV_ATTRIBUTEINTERPOLATION_H
