#pragma once

#include "Math.hpp"

class Camera
{
private:
    void Reorient();

    const glm::vec3 roll_axis = glm::vec3(0, 0, 1);
    const glm::vec3 pitch_axis = glm::vec3(1, 0, 0);
    const glm::vec3 yaw_axis = glm::vec3(0, 1, 0);

    Angles angles;

    glm::quat roll;
    glm::quat pitch;
    glm::quat yaw;

    float near_plane = 0.1f;
    float far_plane = 2200.0f;

public:
    glm::mat4x4 view;
    glm::mat4x4 projection;
    glm::mat4x4 view_projection;
    glm::mat4x4 inverse_projection;
    glm::mat4x4 inverse_view_projection;
    glm::mat4x4 view_rotation;
    glm::mat4x4 inverse_view_rotation;
    glm::vec3 position;
    glm::vec3 translation;
    glm::vec3 direction;

    Camera();

    const glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);

    void LookAt(
        glm::vec3 target);

    void Perspective(
        float viewport_ratio,
        float fov = 80);

    void SetPosition(
        glm::vec3 position);

    void SetAngles(
        Angles value);

    void Strafe(float speed);

    void Validate();
};
