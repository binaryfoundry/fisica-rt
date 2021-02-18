#pragma once

#include "math/Math.hpp"
#include "math/Angles.hpp"

class Camera
{
private:
    const glm::vec3 pitch_axis = glm::vec3(1, 0, 0);
    const glm::vec3 yaw_axis = glm::vec3(0, 1, 0);
    const glm::vec3 roll_axis = glm::vec3(0, 0, 1);

    glm::vec3 direction;
    glm::vec3 translation;

    glm::mat4 view;
    glm::mat4 projection;

public:
    Camera();
    Camera(const Camera&) = delete;

    const glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);

    Angles orientation;
    glm::vec3 position;
    glm::vec4 viewport;

    float fov = 80;
    float exposure = 1.0f;
    float near_plane = 0.1f;
    float far_plane = 2200.0f;

    void Strafe(float speed);
    void Validate();

    glm::mat4 View() const { return view; };
    glm::mat4 Projection() const { return projection; };
};
