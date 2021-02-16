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

glm::quat quat_from_axis_angle(
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
    float f = 1.0f / std::tanf(fov * (PIf / 360.0f));

    float matrix_values[16] =
    {
        f / viewport_ratio, 0, 0, 0,
        0, f, 0, 0,
        0, 0, (far_plane + near_plane) / (near_plane - far_plane), -1,
        0, 0, 2 * far_plane*near_plane / (near_plane - far_plane), 0
    };

    projection = glm::make_mat4(matrix_values);
}

void Camera::Strafe(float speed)
{
    glm::vec3 strafe_direction = glm::cross(up, direction);

    if (glm::length(strafe_direction) > 0.001f)
    {
        strafe_direction = glm::normalize(strafe_direction);
    }

    position += strafe_direction * speed;
}

void Camera::LookAt(glm::vec3 target)
{
    view = glm::lookAt(
        position,
        target,
        up);

    direction = normalize(position - target);
}

void Camera::Reorient()
{
    glm::quat temp_1 = pitch * yaw;
    glm::quat temp_2 = temp_1 * roll;
    view = mat4_cast(temp_2);
    view = translate(view, translation);
    glm::mat4x4 pitch_matrix = mat4_cast(pitch);
    glm::quat temp_3 = yaw * pitch;
    glm::mat4x4 temp_matrix = mat4_cast(temp_3);

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
    inverse_view_rotation = glm::inverse(view_rotation);

    view_projection = projection * view;
    inverse_view_projection = glm::inverse(view_projection);
    inverse_projection = glm::inverse(projection);
}

