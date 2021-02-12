#pragma once

const double PI = 3.141592653589793;
const float PIf = 3.141592653589793f;

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat2x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

struct Angles
{
public:
    float roll = 0;
    float pitch = 0;
    float yaw = 0;

    Angles()
    {
    };

    Angles(
        float roll,
        float pitch,
        float yaw) :
        roll(roll),
        pitch(pitch),
        yaw(yaw)
    {
    };

    Angles(glm::quat quaternion)
    {
        float sqx = quaternion[0] * quaternion[0];
        float sqy = quaternion[1] * quaternion[1];
        float sqz = quaternion[2] * quaternion[2];
        float sqw = quaternion[3] * quaternion[3];

        float unit = sqx + sqy + sqz + sqw;

        float test =
            quaternion.x * quaternion.y +
            quaternion.z * quaternion.w;

        double heading = 0.0;
        double attitude = 0.0;
        double bank = 0.0;

        if (test > 0.4999 * unit)
        {
            heading = 2.0 * std::atan2(
                quaternion.x,
                quaternion.w);
            attitude = PI / 2.0;
            bank = 0.0;
        }
        else if (test < -0.4999 * unit)
        {
            heading = -2.0 * std::atan2(
                quaternion.x,
                quaternion.w);
            attitude = -PI / 2.0;
            bank = 0.0;
        }
        else
        {
            heading = std::atan2(
                2.0 * quaternion.y * quaternion.w -
                2.0 * quaternion.x * quaternion.z,
                sqx - sqy - sqz + sqw);

            attitude = std::asin(
                2.0 * test / unit);

            bank = std::atan2(
                2.0 * quaternion.x * quaternion.w -
                2.0 * quaternion.y * quaternion.z,
                -sqx + sqy - sqz + sqw);
        }

        roll = static_cast<float>(heading);
        pitch = static_cast<float>(bank);
        yaw = static_cast<float>(attitude);
    };
};
