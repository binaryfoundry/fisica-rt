#pragma once

#include "Math.hpp"

static glm::quat quat_from_axis_angle(
    glm::vec3 axis,
    float angle)
{
    glm::quat dest;
    dest.x = axis.x;
    dest.y = axis.y;
    dest.z = axis.z;

    float n = std::sqrt(
        axis.x * axis.x +
        axis.y * axis.y +
        axis.z * axis.z);

    // zero-div may occur.
    float s = (std::sinf(0.5f * angle) / n);

    dest.x *= s;
    dest.y *= s;
    dest.z *= s;
    dest.w = std::cosf(0.5f * angle);

    return dest;
}

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
