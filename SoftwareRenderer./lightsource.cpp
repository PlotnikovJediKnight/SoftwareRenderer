#include "lightsource.h"
#include <cmath>

namespace pv {

LightSource::LightSource() :
    world_origin_distance_r_(50.0),
    xy_azimuth_phi_degrees_(0.0),
    z_inclination_theta_degrees_(30.0),
    argb_light_color_({255, 253, 251, 211})
{
    light_source_position_ = GetNewLightSourcePosition();
}

void LightSource::UpdateLightSourcePosition(float azimuthDegrees) {
    xy_azimuth_phi_degrees_ = azimuthDegrees;
    light_source_position_ = GetNewLightSourcePosition();
}

glm::vec3 LightSource::GetLightSourcePositionWorld() const {
    return light_source_position_;
}

std::array<LightSource::uchar, 4> LightSource::GetLightColor() const {
    return argb_light_color_;
}

float LightSource::GetRadianAngle(float degreeAngle) const {
    return degreeAngle / 180.0 * M_PI;
}

glm::vec3 LightSource::GetNewLightSourcePosition() const {
    float phiInRadians = GetRadianAngle(xy_azimuth_phi_degrees_);
    float thetaInRadians = GetRadianAngle(z_inclination_theta_degrees_);

    return { world_origin_distance_r_ * cos(phiInRadians) * sin(thetaInRadians),
             world_origin_distance_r_ * sin(phiInRadians) * sin(thetaInRadians),
             world_origin_distance_r_ * cos(thetaInRadians)
    };
}

} // namespace pv
