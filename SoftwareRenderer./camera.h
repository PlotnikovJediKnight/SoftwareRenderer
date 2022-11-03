#ifndef PV_CAMERA_H
#define PV_CAMERA_H

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace pv {

class Camera {
public:
    Camera(float sphereRadius = 200);

    void SetWorldOriginDistanceR(float r);
    void UpdateCameraPosition(float azimuthDegrees, float inclinationDegrees);

    void SetViewFromX();
    void SetViewFromY();
    void SetViewFromZ();

    glm::mat4 GetCameraMatrix();

    glm::vec3 GetWorldViewDirection();

private:

    float GetRadianAngle(float degreeAngle) const;
    glm::vec3 GetNewCameraOriginPosition(float azimuthDegrees, float inclinationDegrees) const;

    glm::vec3 GetRotationAxis() const;

    void UpdateAzimuthAngle(float degreeDelta);
    void UpdateInclinationAngle(float degreeDelta);

    float world_origin_distance_r_;
    float xy_azimuth_phi_degrees_;
    float z_inclination_theta_degrees_;

    glm::vec3 camera_origin_point_;
    glm::vec3 x_camera_vector_;
    glm::vec3 y_camera_vector_;
    glm::vec3 z_camera_vector_;
};

} // namespace pv

#endif // PV_CAMERA_H
