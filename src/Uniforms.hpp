#pragma once

#include "Math.hpp"

struct Transform
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 inverse_projection;
    glm::mat4 inverse_view_rotation;
    glm::vec4 viewport;
    glm::vec4 camera_position;
    glm::vec4 exposure;
};
