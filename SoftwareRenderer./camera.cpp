#include "camera.h"
#include <glm/gtx/rotate_vector.hpp>

namespace pv {

Camera::Camera(float sphereRadius) :
    world_origin_distance_r_(sphereRadius),
    xy_azimuth_phi_degrees_(0.0),
    z_inclination_theta_degrees_(0.0) {

    camera_origin_point_ = { 0, 0,  world_origin_distance_r_};
    x_camera_vector_   = { 0, 1, 0 };
    y_camera_vector_   = { 1, 0, 0 };
    z_camera_vector_   = { 0, 0, -1};

}

void Camera::SetWorldOriginDistanceR(float r) {
    world_origin_distance_r_ = r;
    camera_origin_point_ *= world_origin_distance_r_ / glm::length(camera_origin_point_);
}

void Camera::UpdateCameraPosition(float azimuthDegrees, float inclinationDegrees) {
    UpdateAzimuthAngle(azimuthDegrees);
    glm::tvec3<double> rotationAxis = GetRotationAxis();

    UpdateInclinationAngle(inclinationDegrees);
    glm::tvec3<double> oldYCameraVector{0, 0, -1};
    glm::tvec3<double>
    newYCameraVector = glm::rotate(oldYCameraVector,
                                   static_cast<double>(GetRadianAngle(90.0 - z_inclination_theta_degrees_)),
                                   -rotationAxis);

    camera_origin_point_ = GetNewCameraOriginPosition(xy_azimuth_phi_degrees_, z_inclination_theta_degrees_);
    glm::vec3 newZCameraVector = -glm::normalize(camera_origin_point_);

    glm::vec3 newYCameraVectorFloat = newYCameraVector;
    glm::vec3 newXCameraVector = glm::cross(newYCameraVectorFloat, newZCameraVector);

    x_camera_vector_ = newXCameraVector;
    y_camera_vector_ = newYCameraVectorFloat;
    z_camera_vector_ = newZCameraVector;
}

void Camera::SetViewFromX() {
    xy_azimuth_phi_degrees_ = 0.0;
    z_inclination_theta_degrees_ = 90.0;

    camera_origin_point_ = { world_origin_distance_r_, 0,  0};
    x_camera_vector_     = { 0, 1, 0};
    y_camera_vector_     = { 0, 0, -1};
    z_camera_vector_     = { -1, 0, 0};

}

void Camera::SetViewFromY() {
    xy_azimuth_phi_degrees_ = 90.0;
    z_inclination_theta_degrees_ = 90.0;

    camera_origin_point_ = { 0, world_origin_distance_r_, 0};
    x_camera_vector_   = { -1, 0, 0};
    y_camera_vector_   = { 0, 0, -1};
    z_camera_vector_   = { 0, -1, 0};

}

void Camera::SetViewFromZ() {
    xy_azimuth_phi_degrees_ = 0.0;
    z_inclination_theta_degrees_ = 0.0;

    camera_origin_point_ = { 0, 0,  world_origin_distance_r_};
    x_camera_vector_   = { 0, 1, 0 };
    y_camera_vector_   = { 1, 0, 0 };
    z_camera_vector_   = { 0, 0, -1};

}

glm::mat4 Camera::GetCameraMatrix() {

    glm::vec4 xAxisColumnVector = glm::vec4(x_camera_vector_, 0);
    glm::vec4 yAxisColumnVector = glm::vec4(y_camera_vector_, 0);
    glm::vec4 zAxisColumnVector = glm::vec4(z_camera_vector_, 0);
    glm::vec4 cameraTranslationVector = glm::vec4(camera_origin_point_, 1);

    return {xAxisColumnVector,
            yAxisColumnVector,
            zAxisColumnVector,
                cameraTranslationVector};
}

glm::vec3 Camera::GetWorldViewDirection() {
    return z_camera_vector_;
}

float Camera::GetRadianAngle(float degreeAngle) const {
    return degreeAngle / 180.0 * M_PI;
}

glm::vec3 Camera::GetNewCameraOriginPosition(float azimuthDegrees, float inclinationDegrees) const {
    float phiInRadians = GetRadianAngle(azimuthDegrees);
    float thetaInRadians = GetRadianAngle(inclinationDegrees);

    return { world_origin_distance_r_ * cos(phiInRadians) * sin(thetaInRadians),
             world_origin_distance_r_ * sin(phiInRadians) * sin(thetaInRadians),
             world_origin_distance_r_ * cos(thetaInRadians)
    };
}

glm::vec3 Camera::GetRotationAxis() const {
    glm::vec3 newCameraPosition = GetNewCameraOriginPosition(xy_azimuth_phi_degrees_, 90.0);
    glm::vec3 newZCameraVector  = -glm::normalize(newCameraPosition);

    glm::vec3 yCameraVector = { 0, 0, -1};
    glm::vec3 newXCameraVector = glm::cross(yCameraVector, newZCameraVector);

    return newXCameraVector;
}

void Camera::UpdateAzimuthAngle(float degreeDelta) {
    xy_azimuth_phi_degrees_ += degreeDelta;
    if (xy_azimuth_phi_degrees_ >  360.0 ||
        xy_azimuth_phi_degrees_ < -360.0){
        xy_azimuth_phi_degrees_ = 0.0;
    }
}

void Camera::UpdateInclinationAngle(float degreeDelta) {
    z_inclination_theta_degrees_ += degreeDelta;

    if (z_inclination_theta_degrees_ >  180.0){
        z_inclination_theta_degrees_ = 180.0;
    }

    if (z_inclination_theta_degrees_ <  0.0){
        z_inclination_theta_degrees_ = 0.0;
    }
}

} // namespace pv
