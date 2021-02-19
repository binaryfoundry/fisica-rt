#include "Camera.hpp"

Camera::Camera()
{
}

void Camera::Strafe(float speed)
{
    glm::vec3 strafe_direction = glm::cross(
        up,
        direction);

    if (glm::length(strafe_direction) > 0.001f)
    {
        strafe_direction = glm::normalize(
            strafe_direction);
    }

    position += strafe_direction * speed;
}

void Camera::Forward(float speed)
{
    // TODO Fix OpenGL Y axis.
    const glm::vec3 d = glm::vec3(
        direction.x,
        -direction.y,
        direction.z);
    position += d * speed;
}

void Camera::Validate()
{
    const glm::quat roll = quat_from_axis_angle(
        yaw_axis,
        orientation.roll);

    const glm::quat pitch = quat_from_axis_angle(
        pitch_axis,
        orientation.pitch);

    const glm::quat yaw = quat_from_axis_angle(
        yaw_axis,
        orientation.yaw);

    const glm::quat temp_1 = pitch * yaw;
    const glm::quat temp_2 = temp_1 * roll;

    const glm::mat4x4 pitch_matrix = mat4_cast(
        pitch);

    const glm::quat temp_3 = yaw * pitch;

    const glm::mat4x4 temp_matrix = mat4_cast(
        temp_3);

    const float f = 1.0f / std::tanf(fov * (PIf / 360.0f));
    float viewport_ratio = 1.0f; // TODO

    const float proj_matrix_values[16] =
    {
        f / viewport_ratio, 0, 0, 0,
        0, f, 0, 0,
        0, 0, (far_plane + near_plane) / (near_plane - far_plane), -1,
        0, 0, 2 * far_plane*near_plane / (near_plane - far_plane), 0
    };

    direction = glm::vec3(
        temp_matrix[2][0],
        pitch_matrix[2][1],
        -temp_matrix[2][2]);

    translation = -position;

    projection = glm::make_mat4(
        proj_matrix_values);

    view = translate(
        mat4_cast(temp_2),
        translation);
}

