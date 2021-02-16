#include "Camera.hpp"

Camera::Camera()
{
    Perspective(1.0f, 80.0f);
    Validate();
}

void Camera::SetPosition(glm::vec3 position_)
{
    position = position_;
    translation = -position_;

    Reorient();
}

void Camera::SetAngles(
    Angles value) {

    angles.roll = value.roll;
    angles.pitch = value.pitch;
    angles.yaw = value.yaw;

    roll = quat_from_axis_angle(
        yaw_axis,
        angles.roll);
    pitch = quat_from_axis_angle(
        pitch_axis,
        angles.pitch);
    yaw = quat_from_axis_angle(
        yaw_axis,
        angles.yaw);

    Reorient();
}

void Camera::Perspective(
    float viewport_ratio,
    float fov)
{
    const float f = 1.0f / std::tanf(fov * (PIf / 360.0f));

    const float matrix_values[16] =
    {
        f / viewport_ratio, 0, 0, 0,
        0, f, 0, 0,
        0, 0, (far_plane + near_plane) / (near_plane - far_plane), -1,
        0, 0, 2 * far_plane*near_plane / (near_plane - far_plane), 0
    };

    projection = glm::make_mat4(
        matrix_values);
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

void Camera::LookAt(glm::vec3 target)
{
    view = glm::lookAt(
        position,
        target,
        up);

    direction = normalize(
        position - target);
}

void Camera::Reorient()
{
    const glm::quat temp_1 = pitch * yaw;
    const glm::quat temp_2 = temp_1 * roll;

    view = mat4_cast(
        temp_2);

    view = translate(
        view,
        translation);

    const glm::mat4x4 pitch_matrix = mat4_cast(
        pitch);

    const glm::quat temp_3 = yaw * pitch;

    const glm::mat4x4 temp_matrix = mat4_cast(
        temp_3);

    direction = glm::vec3(
        temp_matrix[2][0],
        pitch_matrix[2][1],
        -temp_matrix[2][2]);
}

void Camera::Validate()
{
    view_rotation = view;
    view_rotation[3][0] = 0.0;
    view_rotation[3][1] = 0.0;
    view_rotation[3][2] = 0.0;
    inverse_view_rotation = glm::inverse(
        view_rotation);

    view_projection = projection * view;
    inverse_view_projection = glm::inverse(
        view_projection);
    inverse_projection = glm::inverse(
        projection);
}

