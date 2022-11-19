#ifndef PV_LIGHTSOURCE_H
#define PV_LIGHTSOURCE_H


#include "glm/ext/vector_float3.hpp"
#include <array>

namespace pv {

class LightSource
{
public:
    using uchar = unsigned char;
    LightSource();

    void UpdateLightSourcePosition(float azimuthDegrees);

    glm::vec3 GetLightSourcePositionWorld() const;
    float GetLightSourcePositionDegrees() const;

    void SetLightColor(std::array<uchar, 4>);
    std::array<uchar, 4> GetLightColor() const;

    float GetReflectancePower() const;
    void SetReflectancePower(float reflectancePower);

private:

    float GetRadianAngle(float degreeAngle) const;

    glm::vec3 GetNewLightSourcePosition() const;

    const float world_origin_distance_r_;
    float xy_azimuth_phi_degrees_;
    const float z_inclination_theta_degrees_;

    std::array<uchar, 4> argb_light_color_;
    glm::vec3 light_source_position_;
    float reflectance_power_;
};

} // namespace pv

#endif // PV_LIGHTSOURCE_H
